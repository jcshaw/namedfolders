/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/


#pragma once 
#include "stdafx.h"
#include "far_impl.h"

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

extern struct PluginStartupInfo g_PluginInfo; 

int nf::FarCmpName(const wchar_t *Pattern, const wchar_t *String, int SkipPath) {
	return g_PluginInfo.CmpName(Pattern, String, SkipPath);
}

void nf::CloseAndStartAnotherPlugin(HANDLE hPlugin, tstring const& Command, bool bActivePanel, bool bOpenBoth)
{	//"press" all keys - Command + Enter
	using namespace nf;
	//ignore all prefixes except first one 
	tstring prefix = CSettings::GetInstance().GetPrimaryPluginPrefix();

	ULONG add_size = 1 + 4 + static_cast<int>(prefix.size()); //CTRL+Y + TAB + ENTER + TAB + "cd:" + ENTER  
	nf::autobuffer_wrapper<DWORD> ks_buffer;
	ks_buffer.resize(static_cast<unsigned int>(Command.size() + add_size));

	//always add Ctrl + Y at the beginning to clear command line
	ks_buffer.push_back(int('y') | KEY_CTRL);

	if (! bActivePanel) {
		ks_buffer.push_back(VK_TAB);
		for (unsigned int i = 0; i < Command.size(); ++i) {
			ks_buffer.push_back( static_cast<DWORD>(Command[i]));
		}
		ks_buffer.push_back(VK_RETURN);

		if (! bOpenBoth) {
			ks_buffer.push_back(VK_TAB);
				for (unsigned int i = 0; i < prefix.size(); ++i) {
					ks_buffer.push_back( static_cast<DWORD>(prefix[i]));
				}
				ks_buffer.push_back(VK_RETURN);
		}
	} else {
		for (unsigned int i = 0; i < Command.size(); ++i) {
			if (static_cast<DWORD>(Command[i]) == L'\n') {
				ks_buffer.push_back(VK_RETURN);
			} else {
				ks_buffer.push_back( static_cast<DWORD>(Command[i]));
			}
		}
		ks_buffer.push_back(VK_RETURN);
	}

	static KeySequence ks;
	ks.Flags = 0;//KSFLAGS_DISABLEOUTPUT;
	ks.Count = static_cast<int>(ks_buffer.size());
	ks.Sequence = &ks_buffer[0];

	BOOL bSuccess = static_cast<BOOL>(g_PluginInfo.AdvControl(g_PluginInfo.ModuleNumber
		, ACTL_POSTKEYSEQUENCE
		, reinterpret_cast<void*>(&ks)));
	assert(bSuccess);
}

namespace {
	void open_path_and_close_plugin(CPanelInfoWrap &Plugin
									, bool bClosePlugin
									, bool bActivePanel
									, tstring srcDir
									, tstring const& fileName) {
		srcDir = Utils::ReplaceStringAll(srcDir, SLASH_CATS, SLASH_DIRS); // "/" are replaced by "\" - protection from UNIX-users

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

	bool find_path_and_filename(HANDLE hPlugin
		, tstring const& srcPath
		, nf::twhat_to_search_t whatToSearch
		, tstring const& localPath
		, tstring &destPath
		, tstring &fnDest) 
	{
		tstring dest_path;
		if (! nf::Selectors::GetPath(hPlugin, srcPath, localPath, dest_path, whatToSearch)) return false;
		if (! ::PathIsDirectory(dest_path.c_str())) {
			//open directory where file is located; currently, we lost filename.
			//!TODO: it world be perfect to position on this file
			Utils::DividePathFilename(dest_path, destPath, fnDest, SLASH_DIRS_CHAR, false);
			Utils::RemoveLeadingCharsOnPlace(fnDest, SLASH_DIRS_CHAR);
		} else {
			fnDest = L"";
		}
		dest_path.swap(destPath);
		return true;
	}
} //namespace 

bool nf::OpenShortcutOnPanel(HANDLE hPlugin, nf::tshortcut_value_parsed &panel, tstring path
							 , bool bActivePanel, bool bOpenBoth
							 , bool bClosePlugin
							 , nf::twhat_to_search_t WhatToSearch) {
	using namespace nf;
	assert(panel.bValueEnabled);
	CPanelInfoWrap plugin(hPlugin);
	tstring filename;	//имя файла на котором нужно позиционироваться

	if (panel.ValueType == VAL_TYPE_PLUGIN_DIRECTORY) {	//открыть виртуальную директорию
		nf::CloseAndStartAnotherPlugin(hPlugin, panel.value, bActivePanel, bOpenBoth);
		g_PluginInfo.Control(hPlugin
			, FCTL_CLOSEPLUGIN
			, 0
			, 0
		);	//плагин необходимо закрывать, иначе при вызове из панели //!TODO: проверить на Far 2.0
			//происходит зацикливание (начиная с FAR 2060)
	} else {	//другие типы директорий
		//issue #4: SLASH_CATS shouldn't be changed on SLASH_DIRS if shortcuts links to external plugin directory
		panel.value =  Utils::ReplaceStringAll(panel.value, SLASH_CATS, SLASH_DIRS);
		path =  Utils::ReplaceStringAll(path, SLASH_CATS, SLASH_DIRS);

		tstring dir;
		switch (panel.ValueType) {
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
					if (! nf::Selectors::GetPath(hPlugin, dir, path, dir, nf::WTS_DIRECTORIES)) return false;
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
			if (! ::find_path_and_filename(hPlugin, panel.value, WhatToSearch, path, panel.value, filename)) {
				if (! nf::Selectors::FindBestDirectory(hPlugin, panel, dir)) return false;
			}
		}; //switch
		
		if (! PathFileExists(dir.c_str())) {
			if (! nf::Selectors::FindBestDirectory(hPlugin, panel, dir)) return false;
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
	if (! find_path_and_filename(hPlugin, dest_path, whatToSearch, L"", dest_path, dest_filename)) return false;
	::open_path_and_close_plugin(CPanelInfoWrap(hPlugin), false, bActivePanel, dest_path, dest_filename);
	return true;
}