/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "stdafx.h"
#include <windows.h> 
#include <cassert>
#include "settings.h"
#include "lang.h"
#include "Panel.h"
#include "kernel.h"
#include "ConfigureDialog.h"
#include "open_plugin.h"
#include "DiskMenuHelper.h"
#include "dialogs_completion.h"

struct PluginStartupInfo g_PluginInfo; 
struct FarStandardFunctions g_FSF;

using namespace nf;
void WINAPI GetGlobalInfoW(struct GlobalInfo *dest) {
	dest->StructSize = sizeof(GlobalInfo);
	dest->MinFarVersion = FARMANAGERVERSION;
	dest->Guid = nf::NF_PLUGIN_GUID;
	dest->Version = MAKEFARVERSION(3, 0, 0, 3052, VS_RELEASE);
	dest->Title = L"Named Folders"; //!TODO
	dest->Description = L"Fast access to directories"; //!TODO
	dest->Author = L"Victor Derevyanko, dvpublic0@gmail.com";
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *pInfo) {
	g_PluginInfo = *pInfo;
	g_FSF = *pInfo->FSF;
	g_PluginInfo.FSF = &g_FSF;
	g_PluginInfo.FSF=&g_FSF; //see SetStartupInfoW in encyclopedia
}


void WINAPI GetPluginInfoW(struct PluginInfo *pInfo) {	
	static nf::DiskMenuHelper menu_helper;

	pInfo->StructSize = sizeof(*pInfo); 
	pInfo->Flags = PF_FULLCMDLINE | PF_DIALOG;

	pInfo->DiskMenu.Count = static_cast<int>(menu_helper.GetCountStrings());
	pInfo->DiskMenu.Strings = menu_helper.GetStringsArray();
	pInfo->DiskMenu.Guids = menu_helper.GetGuidsPtr();

	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0]= nf::GetMsg(lg::PLUGINMENUSTRING);

	pInfo->PluginMenu.Count = CSettings::GetInstance().GetValue(nf::ST_SHOW_IN_PLUGINS_MENU) ? 1 : 0;
	pInfo->PluginMenu.Strings = PluginMenuStrings;
	pInfo->PluginMenu.Guids = &nf::NF_PLUGINSMENU_GUID;

	static const wchar_t *PluginConfigStrings[1];
	PluginConfigStrings[0]= nf::GetMsg(lg::PLUGINSCONFIGSTRING);

	pInfo->PluginConfig.Count = 1;
	pInfo->PluginConfig.Strings = PluginConfigStrings;
	pInfo->PluginConfig.Guids = &nf::NF_CONFIGMENU_GUID;


	static std::wstring list_prefixes = CSettings::GetInstance().GetListPrefixes().c_str();
	pInfo->CommandPrefix = list_prefixes.c_str();
	return;
}

HANDLE WINAPI  OpenW(const struct OpenInfo *pInfo) {
	if (! (pInfo->StructSize >= sizeof(OpenInfo))) {
		return 0;
	}
	try {
		switch (pInfo->OpenFrom) {
		case OPEN_COMMANDLINE: 
			{
#pragma warning(disable: 4312)
				wchar_t const* pCmd = reinterpret_cast<OpenCommandLineInfo const*>(pInfo->Data)->CommandLine;
#pragma warning(default: 4312)
				return nf::OpenFromCommandLine(pCmd).first;
			}
		break;
		case OPEN_PLUGINSMENU:
			return nf::OpenFromPluginsMenu();
			break;
		case OPEN_RIGHTDISKMENU:
		case OPEN_LEFTDISKMENU:
			if (pInfo->Data == 0) {
				return new nf::Panel::CPanel();
			} else {
				//we get "nf:catalogname"
				//so, open "cd:catalogname/*"
				nf::DiskMenuHelper menu_helper;
				return nf::OpenFromCommandLine(menu_helper.GetCommand(static_cast<unsigned int>(pInfo->Data)).c_str()).first;
			}
		break;
		case OPEN_DIALOG:
			nf::DialogsCompletion::OpenFromDialog(reinterpret_cast<OpenDlgPluginData*>(pInfo->Data)->hDlg);
			break;
		}
	} catch (...) {
	}

	return 0;
}

void WINAPI GetOpenPanelInfoW(struct OpenPanelInfo *pInfo) {
	if (! (pInfo->StructSize >= sizeof(OpenPanelInfo))) {
		return;
	}
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(pInfo->hPanel);
		p->GetOpenPanelInfo(pInfo);
	} catch (...) {
	}
}

intptr_t WINAPI GetFindDataW(struct GetFindDataInfo *pInfo) {
	if (! (pInfo->StructSize >= sizeof(GetFindDataInfo))) {
		return FALSE;
	}
	if (pInfo->hPanel == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(pInfo->hPanel);
		return p->GetFindData(pInfo);
	} catch (...) {
		return FALSE;
	}
}

void WINAPI FreeFindDataW(const struct FreeFindDataInfo *pInfo) {
	if (! (pInfo->StructSize >= sizeof(FreeFindDataInfo))) {
		return;
	}
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(pInfo->hPanel);
		p->FreeFindData(pInfo);
	} catch (...) {
	}
}

intptr_t WINAPI SetDirectoryW(const struct SetDirectoryInfo *pInfo) {
	if (! (pInfo->StructSize >= sizeof(SetDirectoryInfo))) {
		return 0;
	}
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(pInfo->hPanel);
		return p->SetDirectory(pInfo);
	} catch (...) {
		return 0;
	}
}

void WINAPI ClosePanelW(const struct ClosePanelInfo *pInfo) {
	if (! (pInfo->StructSize >= sizeof(ClosePanelInfo))) {
		return;
	}
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(pInfo->hPanel);
		delete p;
	} catch (...) {
	}
}

intptr_t WINAPI ProcessPanelInputW(const struct ProcessPanelInputInfo *pInfo) {
	if (! (pInfo->StructSize >= sizeof(ProcessPanelInputInfo))) {
		return 0;
	}
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(pInfo->hPanel);
		return p->ProcessPanelInputW(pInfo->Rec);
	} catch (...) {
		return 0;
	}
}

intptr_t WINAPI ConfigureW(const struct ConfigureInfo *pInfo) {
	if (! (pInfo->StructSize >= sizeof(ConfigureInfo))) {
		return 0;
	}
	try {
		nf::CConfigureDialog dlg;
		return dlg.ShowModal();
	} catch (...) {
		return FALSE;
	}
}

intptr_t WINAPI MakeDirectoryW(struct MakeDirectoryInfo *pInfo) {
	if (! (pInfo->StructSize >= sizeof(MakeDirectoryInfo))) {
		return 0;
	}
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(pInfo->hPanel);
		return p->MakeDirectory(pInfo);
	} catch (...) {
		return FALSE;
	}
}

intptr_t WINAPI PutFilesW(const struct PutFilesInfo *pInfo) {
	if (! (pInfo->StructSize >= sizeof(PutFilesInfo))) {
		return 0;
	}
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(pInfo->hPanel);
		return p->PutFiles(pInfo);
	} catch (...) {
		return FALSE;
	}
}

intptr_t WINAPI ProcessPanelEventW(const struct ProcessPanelEventInfo *pInfo) {
	if (! (pInfo->StructSize >= sizeof(ProcessPanelEventInfo))) {
		return 0;
	}
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(pInfo->hPanel);
		return p->ProcessEvent(pInfo);
	} catch (...) {
		return FALSE;
	}
}

intptr_t WINAPI ProcessDialogEventW(const struct ProcessDialogEventInfo *pInfo) {
	if (! (pInfo->StructSize >= sizeof(ProcessDialogEventInfo))) {
		return 0;
	}
	try {
		if (pInfo->Event == DE_DEFDLGPROCINIT) {
			FarDialogEvent *pevent = reinterpret_cast<FarDialogEvent*>(pInfo->Param);
			if (pevent->Msg == DN_CONTROLINPUT) {
				const INPUT_RECORD* record = (const INPUT_RECORD *)pevent->Param2;
				if (record->EventType == KEY_EVENT && record->Event.KeyEvent.bKeyDown) {
					if (VK_F12 == record->Event.KeyEvent.wVirtualKeyCode) {
						int a = 0; //!TODO: что за тесты?
					}
				}
			}
		}
		return 0;
	} catch (...) {
		return FALSE;
	}
}
