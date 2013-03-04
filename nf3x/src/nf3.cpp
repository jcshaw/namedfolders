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

	pInfo->DiskMenu.Count = static_cast<int>(menu_helper.GetCountStrings());
	pInfo->DiskMenu.Strings = menu_helper.GetStringsArray();
	pInfo->DiskMenu.Guids = menu_helper.GetGuidsPtr();

	static const wchar_t *PluginMenuStrings[1];
	static const GUID *PluginMenuGuids[1];
	PluginMenuStrings[0]= nf::GetMsg(lg::PLUGINMENUSTRING);
	PluginMenuGuids[0] = &nf::NF_PLUGINSMENU_GUID;

	pInfo->PluginMenu.Count = CSettings::GetInstance().GetValue(nf::ST_SHOW_IN_PLUGINS_MENU) ? 1 : 0;
	pInfo->PluginMenu.Strings = PluginMenuStrings;
	pInfo->PluginMenu.Guids = PluginMenuGuids[0];

	static const wchar_t *PluginConfigStrings[1];
	static const GUID *PluginConfigGuids[1];
	PluginConfigStrings[0]= nf::GetMsg(lg::PLUGINSCONFIGSTRING);
	PluginConfigGuids[0]= &nf::NF_CONFIGMENU_GUID;

	pInfo->PluginConfig.Count = 1;
	pInfo->PluginConfig.Strings = PluginConfigStrings;

	pInfo->StructSize = sizeof(*pInfo); 
	pInfo->Flags = PF_FULLCMDLINE | PF_DIALOG;
	
	static std::wstring list_prefixes = CSettings::GetInstance().GetListPrefixes().c_str();
	pInfo->CommandPrefix = list_prefixes.c_str();
	return;
}

HANDLE WINAPI  OpenPluginW(int OpenFrom, INT_PTR Item) {
	try {
		switch (OpenFrom) {
		case OPEN_COMMANDLINE: 
			{
#pragma warning(disable: 4312)
				wchar_t const* pCmd = reinterpret_cast<wchar_t const*>(Item);
#pragma warning(default: 4312)
				return nf::OpenFromCommandLine(pCmd).first;
			}
		break;
		case OPEN_PLUGINSMENU:
			return nf::OpenFromPluginsMenu();
			break;
		case OPEN_RIGHTDISKMENU:
		case OPEN_LEFTDISKMENU:
			if (Item == 0) {
				return new nf::Panel::CPanel();
			} else {
				//we get "nf:catalogname"
				//so, open "cd:catalogname/*"
				nf::DiskMenuHelper menu_helper;
				return nf::OpenFromCommandLine(menu_helper.GetCommand(static_cast<unsigned int>(Item)).c_str()).first;
			}
		break;
		case OPEN_DIALOG:
			nf::DialogsCompletion::OpenFromDialog(reinterpret_cast<OpenDlgPluginData*>(Item)->hDlg);
			break;
		}
	} catch (...) {
	}

	return INVALID_HANDLE_VALUE;
}

void WINAPI  GetOpenPluginInfoW(HANDLE hPlugin, OpenPanelInfo *info) {
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		p->GetOpenPluginInfo(info);
	} catch (...) {
	}
}

int WINAPI  GetFindDataW(HANDLE hPlugin, struct PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode) {
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		return p->GetFindData(pPanelItem, pItemsNumber, OpMode);
	} catch (...) {
		return FALSE;
	}
}

void WINAPI  FreeFindDataW(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber) {
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		p->FreeFindData(PanelItem, ItemsNumber);
	} catch (...) {
	}
}

int WINAPI  SetDirectoryW(HANDLE hPlugin, const wchar_t *Dir, int OpMode) {
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		return p->SetDirectory(Dir, OpMode);
	} catch (...) {
		return FALSE;
	}
}

void WINAPI  ClosePluginW(HANDLE hPlugin) {
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		delete p;
	} catch (...) {
	}
}

intptr_t WINAPI ProcessPanelInputW(const struct ProcessPanelInputInfo *pInfo) {
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(pInfo->hPanel);
		return p->ProcessPanelInputW(pInfo->Rec);
	} catch (...) {
		return FALSE;
	}
}

int WINAPI  ConfigureW(int ItemNumber) {
	try {
		nf::CConfigureDialog dlg;
		return dlg.ShowModal();
	} catch (...) {
		return FALSE;
	}
}


int WINAPI  MakeDirectoryW(HANDLE hPlugin, wchar_t *Name, int OpMode) {
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		return p->MakeDirectory(Name, OpMode);
	} catch (...) {
		return FALSE;
	}
}

int WINAPI  PutFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t *SrcPath, int OpMode) {
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		return p->PutFiles(PanelItem, ItemsNumber, Move, OpMode);
	} catch (...) {
		return FALSE;
	}
}

//!TODO: этой функции в API больше нет! ProcessPanelEventW  ???
int WINAPI  ProcessEventW(HANDLE hPlugin, int Event, void *Param) {
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		return p->ProcessEvent(Event, Param);
	} catch (...) {
		return FALSE;
	}
}

int WINAPI  ProcessDialogEventW(int Event, void *param) {
	try {
		if (Event == DE_DEFDLGPROCINIT) {
			FarDialogEvent *pevent = reinterpret_cast<FarDialogEvent*>(param);
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
