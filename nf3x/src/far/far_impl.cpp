/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/


#pragma once 
#include "stdafx.h"
#include "far_impl.h"
#include "plugin.hpp"

#include <cassert>
#include <Shlwapi.h>
#include <boost/foreach.hpp>

#include "strings_utils.h"
#include "executor_addons.h"
#include "executor_select.h"
#include "PanelInfoWrap.h"
#include "Menu2.h"
#include "PanelUpdater.h"
#include "Parser.h"
#include "autobuffer_wrapper.h"
#include "dialogs_completion.h"

extern struct PluginStartupInfo g_PluginInfo; 

size_t nf::FarCmpName(const wchar_t *pattern, const wchar_t *str, int skipPath) {
	return g_FSF.ProcessName(pattern, const_cast<wchar_t*>(str), 0
		, skipPath 
			? PN_CMPNAME | PN_SKIPPATH 
			: PN_CMPNAME
	);
	//return g_PluginInfo.CmpName(Pattern, String, SkipPath);
}

namespace {
	class buffer_wrapper {
		tstring _Buffer;
	public:
		buffer_wrapper() {
			_Buffer.reserve(256);
		}

		void put(int key, bool ctrlPressed = false) {
			INPUT_RECORD ir;
			ir.EventType = KEY_EVENT;
			ir.Event.KeyEvent.bKeyDown = TRUE;
			ir.Event.KeyEvent.dwControlKeyState = ctrlPressed ? LEFT_CTRL_PRESSED : 0;
			ir.Event.KeyEvent.wRepeatCount = 1;
			if (key != 0) {
				ir.Event.KeyEvent.wVirtualKeyCode = 0;
				ir.Event.KeyEvent.wVirtualScanCode = 0;
				ir.Event.KeyEvent.uChar.UnicodeChar = key; //(WORD)(key & 0x0003FFFF);
			} else {
				ir.Event.KeyEvent.wVirtualKeyCode = key;
				ir.Event.KeyEvent.wVirtualScanCode = MapVirtualKey(key, MAPVK_VK_TO_VSC);
				ir.Event.KeyEvent.uChar.UnicodeChar = 0;
			}

			wchar_t buffer[256];
			size_t size = g_PluginInfo.FSF->FarInputRecordToName(&ir, &buffer[0], 256);
			if (! _Buffer.empty()) {
				_Buffer.append(L" ");
			}
			_Buffer.append(&buffer[0], 0, size - 1);
		}

		tstring getResult() const {
			return _Buffer;
		}
	};
}

void nf::CloseAndStartAnotherPlugin(HANDLE hPlugin, tstring const& Command, bool bActivePanel, bool bOpenBoth)
{	//"press" all keys - Command + Enter
	//¬ Far3 команда ACTL_POSTKEYSEQUENCE выброшена, эмулируем ее через MacroControl 
	buffer_wrapper buf;

	using namespace nf;
	//ignore all prefixes except first one 
	tstring prefix = CSettings::GetInstance().GetPrimaryPluginPrefix();

	//We must execute: CTRL+Y + TAB + ENTER + TAB + "cd:" + ENTER  
	//always add Ctrl + Y at the beginning to clear command line
	buf.put(int(L'y'), true);

	if (! bActivePanel) {
		buf.put(VK_TAB);
		for (unsigned int i = 0; i < Command.size(); ++i) {
			buf.put(static_cast<DWORD>(Command[i]));
		}
		buf.put(VK_RETURN);

		if (! bOpenBoth) {
			buf.put(VK_TAB);
				for (unsigned int i = 0; i < prefix.size(); ++i) {
					buf.put(static_cast<DWORD>(prefix[i]));
				}
				buf.put(VK_RETURN);
		}
	} else {
		for (unsigned int i = 0; i < Command.size(); ++i) {
			if (static_cast<DWORD>(Command[i]) == L'\n') {
				buf.put(VK_RETURN);
			} else {
				buf.put(static_cast<DWORD>(Command[i]));
			}
		}
		buf.put(VK_RETURN);
	}
	tstring str = tstring(L"Keys('") + buf.getResult() + L"')";

	MacroSendMacroText mcr = {sizeof(MacroSendMacroText)};
	mcr.SequenceText = str.c_str();
	mcr.Flags = KMFLAGS_DISABLEOUTPUT;	
	g_PluginInfo.MacroControl(&nf::NF_PLUGIN_GUID, MCTL_SENDSTRING, MSSC_POST, &mcr);
}

namespace {
	void open_path_and_close_plugin(CPanelInfoWrap &Plugin
									, bool bClosePlugin
									, bool bActivePanel
									, tstring srcDir
									, tstring const& fileName) {
		srcDir = Utils::ReplaceStringAll(srcDir, SLASH_CATS, SLASH_DIRS); // "/" are replaced by "\" - protection from UNIX-users
		if (Utils::IsAliasIsDisk(srcDir)) {
			srcDir += SLASH_DIRS; //C: -> C:\, otherwise FAR won't open C: correctly
		}

		if (nf::DialogsCompletion::IsDialogsCompletionOn()) {
			nf::DialogsCompletion::SetDialogsCompletionResult(srcDir);
		} else {
			Plugin.SetPanelDir(bActivePanel, srcDir);
			Plugin.RedrawPanel(bActivePanel);

			if (! fileName.empty()) {
				int current_item = nf::Panel::CPanelItemFinder(fileName.c_str())(); 
				int top_item = (static_cast<LONG>(current_item) > 
					Plugin.GetPanelInfo(true).PanelRect.bottom)
					? current_item 
					: 0;
				Plugin.UpdateAndRedraw(true, current_item, top_item);
			};

			if (bClosePlugin) Plugin.ClosePlugin(srcDir);
		}
	}

	nf::tpath_selection_result find_path_and_filename(HANDLE hPlugin
		, tstring const& srcPath
		, nf::twhat_to_search_t whatToSearch
		, tstring const& localPath
		, tstring &destPath
		, tstring &fnDest) 
	{
		tstring dest_path;
		bool bexist_file = PathFileExists(srcPath.c_str()) && ! ::PathIsDirectory(srcPath.c_str()); //work around #49
		if (! bexist_file) {
			nf::tpath_selection_result ret = nf::Selectors::GetPath(hPlugin, srcPath, localPath, dest_path, whatToSearch);
			if (ret != nf::ID_PATH_SELECTED) return ret;
		} else {
			dest_path = srcPath;
		}

		if (bexist_file || (whatToSearch != nf::WTS_DIRECTORIES && ! ::PathIsDirectory(dest_path.c_str()))) {
			//open directory where file is located; currently, we lost filename.
			tpair_strings kvp = Utils::DividePathFilename(dest_path
				, SLASH_DIRS_CHAR, false);
			dest_path.swap(kvp.first);
			fnDest.swap(kvp.second);
			Utils::RemoveLeadingCharsOnPlace(fnDest, SLASH_DIRS_CHAR);
		} else {
			fnDest = L"";
		}
		dest_path.swap(destPath);
		return nf::ID_PATH_SELECTED;
	}
} //namespace 

bool nf::OpenShortcutOnPanel(HANDLE hPlugin, nf::tshortcut_value_parsed &panel, tstring path
							 , bool bActivePanel, bool bOpenBoth
							 , bool bClosePlugin
							 , nf::twhat_to_search_t whatToSearch) {
	using namespace nf;
	assert(panel.bValueEnabled);
	CPanelInfoWrap plugin(hPlugin);
	tstring filename;	//им€ файла на котором нужно позиционироватьс€

	if (panel.ValueType == VAL_TYPE_PLUGIN_DIRECTORY) {	//открыть виртуальную директорию
		nf::CloseAndStartAnotherPlugin(hPlugin, panel.value, bActivePanel, bOpenBoth);
		g_PluginInfo.PanelControl(hPlugin
			, FCTL_CLOSEPANEL
			, 0
			, 0
		);	//плагин необходимо закрывать, иначе при вызове из панели //!TODO: проверить на Far 2.0
			//происходит зацикливание (начина€ с FAR 2060)
	} else {	//другие типы директорий
		//issue #4: SLASH_CATS shouldn't be changed on SLASH_DIRS if shortcuts links to external plugin directory
		panel.value = Utils::ReplaceStringAll(panel.value, SLASH_CATS, SLASH_DIRS);
		Utils::RemoveSingleTrailingCharOnPlace(panel.value, SLASH_DIRS_CHAR); //last slash is always ignored
		path =  Utils::ReplaceStringAll(path, SLASH_CATS, SLASH_DIRS);

		tstring dir;
		switch (panel.ValueType) {
		case nf::VAL_KNOWN_FOLDER:
			if (! nf::Selectors::GetKnownFolderPath(hPlugin, panel.value, path, dir)) return false;
			break;
		case nf::VAL_ENVIRONMENT_VARIABLE:
			if (! nf::Selectors::GetPathByEnvvarPattern(hPlugin, panel.value, path, dir)) return false;
			break;
		case nf::VAL_REGISTRY_KEY:
			if (! nf::Selectors::GetPathByRegKey(hPlugin, panel.value, path, dir)) return false;
			break;
		case nf::VAL_DIRECT_PATH: {			
				dir = panel.value;
				tstring full_path = Utils::CombinePath(dir, path, SLASH_DIRS);
				if (! PathFileExists(full_path.c_str())) {
					tstring disk;
					Utils::DivideDiskPath(full_path, disk, path);
					if (ID_PATH_SELECTED != nf::Selectors::GetPath(hPlugin, disk, path, dir, whatToSearch)) {
						if (! nf::Selectors::FindBestDirectory(hPlugin, full_path, full_path, dir)) return false;
					}
				} else {
					dir.swap(full_path);
				}
			} break;
		case nf::VAL_TYPE_NET_DIRECTORY:
			if (CSettings::GetInstance().GetValue(nf::ST_FLAG_NETWORK_COMMANDS_THROUGH_COMMAND_LINE)) {
				panel.value = L"net:\ncd " + panel.value;	//\n заменим на ENTER
				nf::CloseAndStartAnotherPlugin(hPlugin, panel.value, bActivePanel, bOpenBoth);			
				break;
			}
		default:
			switch( ::find_path_and_filename(hPlugin, panel.value, whatToSearch, path, panel.value, filename)) {
			case ID_PATH_NOT_FOUND: if (! nf::Selectors::FindBestDirectory(hPlugin, panel, path, dir)) return false;
				break;
			case ID_MENU_CANCELED: return false;				
			};
		}; //switch
		
		if (! PathFileExists(dir.c_str())) {
			if (! nf::Selectors::FindBestDirectory(hPlugin, panel, L"", dir)) {
				if (dir.empty() || ! nf::Selectors::FindBestDirectory(hPlugin, dir, dir, dir)) {				
					return false;
				}
			}
		}
		::open_path_and_close_plugin(plugin, bClosePlugin, bActivePanel, dir, filename);
	}; 

	return true;
}	

bool nf::SelectAndOpenPathOnPanel(HANDLE hPlugin, tlist_pairs_strings const& listAliasPaths
								  , nf::twhat_to_search_t whatToSearch, bool bActivePanel)
{	//suggest to select required variant from menu, then open selected path
	//show list directories, without shortcut names
	nf::tlist_strings paths;	//all possible paths
	BOOST_FOREACH(tpair_strings const& kvp, listAliasPaths) {
		paths.push_back(kvp.second);
	}
	tstring dest_path;
	if (! nf::Menu::SelectPath(paths, dest_path)) return false;

	tstring dest_filename;
	if (find_path_and_filename(hPlugin, dest_path, whatToSearch, L"", dest_path, dest_filename) != nf::ID_PATH_SELECTED) {
		return false;
	}
	::open_path_and_close_plugin(CPanelInfoWrap(hPlugin), false, bActivePanel, dest_path, dest_filename);
	return true;
}