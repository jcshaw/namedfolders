/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/


#pragma once 
#include "stdafx.h"
#include "far_impl.h"
#include "kernel.h"
#include "executor.h"
#include "strings_utils.h"
#include "executor_addons.h"
#include "executor_select.h"
#include "PanelInfoWrap.h"
#include "settings.h"
#include "Menu2.h"
#include "Panel.h"
#include "PanelUpdater.h"

#include <cassert>
#include <Shlwapi.h>

extern struct PluginStartupInfo g_PluginInfo; 

int FarCmpName(const wchar_t *Pattern, const wchar_t *String, int SkipPath) {
	return g_PluginInfo.CmpName(Pattern, String, SkipPath);
}


void CloseAndStartAnotherPlugin(HANDLE hPlugin
								, tstring const& Command
								, bool bActivePanel
								, bool bOpenBoth)
{	//"��������" ��� ������ Command + Enter
	using namespace nf;
	//���������� ��� �������� ����� �������
	tstring prefix = CSettings::GetInstance().GetPrimaryPluginPrefix();

	ULONG add_size = 1 + 4 + static_cast<int>(prefix.size()); //CTRL+Y + TAB + ENTER + TAB + "cd:" + ENTER  

	std::vector<DWORD> ks_buffer;
	//������� ������ ��������� Ctrl + Y
	ks_buffer.push_back(int('y') | KEY_CTRL);

	if (! bActivePanel) {
		ks_buffer.push_back(VK_TAB);
		for (unsigned int i = 0; i < Command.size(); ++i) {
			ks_buffer.push_back(static_cast<DWORD>(Command[i]));
		}
		ks_buffer.push_back(VK_RETURN);

		if (! bOpenBoth) {
			ks_buffer.push_back(VK_TAB);
			for (int i = 0; i < prefix.size(); ++i) {
				ks_buffer.push_back(static_cast<DWORD>(prefix[i]));
			}
			ks_buffer.push_back(VK_RETURN);
		}
	} else {
		for (unsigned int i = 0; i < Command.size(); ++i) {
			if (static_cast<DWORD>(Command[i]) == L'\n') ks_buffer.push_back(VK_RETURN);
			else ks_buffer.push_back(static_cast<DWORD>(Command[i]));
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

namespace
{

	void open_path_and_close_plugin(CPanelInfoWrap &Plugin
									, bool bClosePlugin
									, bool bActivePanel
									, tstring DirANSI
									, tstring const& FileNameANSI)
	{
		//������� "/" ���� �������� ��������� "\"
		//������ �� UNIX-������������� :)
		DirANSI =  Utils::ReplaceStringAll(DirANSI, SLASH_CATS, SLASH_DIRS);

		tstring dir_oem = DirANSI;
		Plugin.SetPanelDir(bActivePanel, dir_oem);
		Plugin.RedrawPanel(bActivePanel);

		if (! FileNameANSI.empty())
		{
			int current_item = nf::Panel::CPanelItemFinder(FileNameANSI.c_str())(); 
			int top_item = (static_cast<LONG>(current_item) > 
				Plugin.GetPanelInfo(true).PanelRect.bottom)
				? current_item 
				: 0;
			Plugin.UpdateAndRedraw(true, current_item, top_item);
		};

		if (bClosePlugin) Plugin.ClosePlugin(dir_oem);
	}

	bool find_path_and_filename(HANDLE hPlugin
		, tstring &SrcPath
		, nf::twhat_to_search_t WhatToSearch
		, tstring const& Path
		, tstring &DestFilename)
	{
		if (! nf::Selectors::GetPath(hPlugin, SrcPath, Path, SrcPath, WhatToSearch)) 
		{
			return false;
		} else if (! ::PathIsDirectory(SrcPath.c_str())) {
			//������ ���������� �� ����� �����
			//������ ��������� � ���������� � ������� ����� ���� ����
			//!TODO: ���� �� ������� ����������������� �� ���� �����.
			Utils::DividePathFilename(SrcPath, SrcPath, DestFilename, SLASH_DIRS_CHAR, false);
			Utils::RemoveLeadingChars(DestFilename, SLASH_DIRS_CHAR);
		}

		return true;
	}
} //namespace 

bool OpenShortcutOnPanel(HANDLE hPlugin
						 , nf::tshortcut_value_parsed &panel
						 , tstring path
						 , bool bActivePanel
						 , bool bOpenBoth	
						 , bool bClosePlugin
						 , nf::twhat_to_search_t WhatToSearch)
{
	using namespace nf;
	assert(panel.bValueEnabled);
	CPanelInfoWrap plugin(hPlugin);
	tstring filename;	//��� ����� �� ������� ����� �����������������

	panel.value =  Utils::ReplaceStringAll(panel.value, SLASH_CATS, SLASH_DIRS);
	path =  Utils::ReplaceStringAll(path, SLASH_CATS, SLASH_DIRS);

	if (panel.ValueType == VAL_TYPE_PLUGIN_DIRECTORY) {	//������� ����������� ����������
		::CloseAndStartAnotherPlugin(hPlugin, panel.value, bActivePanel, bOpenBoth);
		g_PluginInfo.Control(hPlugin
			, FCTL_CLOSEPLUGIN
			, 0
			, 0
		);	//������ ���������� ���������, ����� ��� ������ �� ������ //!TODO: ��������� �� Far 2.0
			//���������� ������������ (������� � FAR 2060)
	} else {	//������ ���� ����������
		tstring dir;
		switch (panel.ValueType)
		{
		case nf::VAL_ENVIRONMENT_VARIABLE:
			if (! nf::Selectors::GetPathByEnvvarPattern(hPlugin, panel.value, path, dir)) return false;
			break;
		case nf::VAL_REGISTRY_KEY:
			if (! nf::Selectors::GetPathByRegKey(hPlugin, panel.value, path, dir)) return false;
			break;
		case nf::VAL_DIRECT_PATH:
			dir = panel.value;
			break;
		case nf::VAL_TYPE_NET_DIRECTORY:
			if (CSettings::GetInstance().GetValue(nf::ST_FLAG_NETWORK_COMMANDS_THROUGH_COMMAND_LINE))
			{
				panel.value = L"net:\ncd " + panel.value;	//\n ������� �� ENTER
				::CloseAndStartAnotherPlugin(hPlugin, panel.value, bActivePanel, bOpenBoth);			
				break;
			}
		default:
			if (! ::find_path_and_filename(hPlugin, panel.value, WhatToSearch, path, filename)) return false;
			if (! nf::Selectors::FindBestDirectory(hPlugin, panel, dir)) return false;
		}; //switch
		::open_path_and_close_plugin(plugin, bClosePlugin, bActivePanel, dir, filename);
	}; 

	return true;
}	//open_shortcut_on_panel




bool SelectAndOpenPathOnPanel(HANDLE hPlugin
							  , std::list<std::pair<tstring, tstring> > const& SrcListAliasPath
							  , nf::twhat_to_search_t WhatToSearch)
{	//���������� ���� ��� ������ ���������� ��������, ����� ������� ��������� ����

	//��������� ������ ������ ����������, ��� ����� ����������
	std::list<tstring> paths;	//������ ���� ��������� �����
	for (std::list<std::pair<tstring, tstring> >::const_iterator p = SrcListAliasPath.begin(); p != SrcListAliasPath.end(); ++p)
	{
		paths.push_back(p->second);
	}
	tstring dest_path;
	if (! nf::Menu::SelectPath(paths, dest_path)) return false;

	tstring dest_filename;
	if (! find_path_and_filename(hPlugin, dest_path, WhatToSearch, L"", dest_filename)) return false;
	::open_path_and_close_plugin(CPanelInfoWrap(hPlugin), false, true, dest_path, dest_filename);
	return true;
}