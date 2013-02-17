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

namespace nf {
	namespace nffar3 {
		/// GUID плагина Named Folders (требуется в FAR3)
		/// {C6A35DB8-6846-449E-A1DC-2FC152ED4B0B}
		static const GUID NF_PLUGIN_GUID = { 0xc6a35db8, 0x6846, 0x449e, { 0xa1, 0xdc, 0x2f, 0xc1, 0x52, 0xed, 0x4b, 0xb } };
		/// {B90A8402-14F7-4382-9275-C79824DD8A24}
		static const GUID NF_DISKMENU_GUID = { 0xb90a8402, 0x14f7, 0x4382, { 0x92, 0x75, 0xc7, 0x98, 0x24, 0xdd, 0x8a, 0x24 } };

		// {FE4E3C90-3590-4F76-9D05-D30FA3338F9E}
		static const GUID NF_PLUGINSMENU_GUID = { 0xfe4e3c90, 0x3590, 0x4f76, { 0x9d, 0x5, 0xd3, 0xf, 0xa3, 0x33, 0x8f, 0x9e } };

		// {F30CBBA9-397A-4BF9-81FB-3154B8FBCD90}
		static const GUID NF_CONFIGMENU_GUID = { 0xf30cbba9, 0x397a, 0x4bf9, { 0x81, 0xfb, 0x31, 0x54, 0xb8, 0xfb, 0xcd, 0x90 } };

	}
}

using namespace nf;
void WINAPI GetGlobalInfoW(struct GlobalInfo *dest) {
	dest->StructSize = sizeof(GlobalInfo);
	dest->MinFarVersion = FARMANAGERVERSION;
	dest->Guid = nf::nffar3::NF_PLUGIN_GUID;
	dest->Version = MAKEFARVERSION(3, 0, 3052, 0, VS_RELEASE);
	dest->Title = L"Named Folders";
	dest->Description = L"Fast access to directories";
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

	pInfo->DiskMenuStringsNumber = static_cast<int>(menu_helper.GetCountStrings());
	pInfo->DiskMenuStrings = menu_helper.GetStringsArray();

	static const wchar_t *PluginMenuStrings[1];
	static const GUID *PluginMenuGuids[1];
	PluginMenuStrings[0]= nf::GetMsg(lg::PLUGINMENUSTRING);

	pInfo->PluginMenuStringsNumber = CSettings::GetInstance().GetValue(nf::ST_SHOW_IN_PLUGINS_MENU) ? 1 : 0;
	pInfo->PluginMenuStrings = PluginMenuStrings;

	static const wchar_t *PluginConfigStrings[1];
	static const GUID *PluginConfigGuids[1];
	PluginConfigStrings[0]= nf::GetMsg(lg::PLUGINSCONFIGSTRING);

	pInfo->PluginConfigStringsNumber = 1;
	pInfo->PluginConfigStrings = PluginConfigStrings;

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
		case OPEN_DISKMENU:
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

void WINAPI  GetOpenPluginInfoW(HANDLE hPlugin, struct OpenPluginInfo *Info) {
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		p->GetOpenPluginInfo(Info);
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

int WINAPI  ProcessKeyW(HANDLE hPlugin,int Key,unsigned int ControlState) {
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		return p->ProcessKey(Key, ControlState);
	} catch (...) {
		return FALSE;
	}
}

int WINAPI  GetMinFarVersionW(void) {
	return MAKEFARVERSION(2, 0
		//, 1692
		, 1500
	);
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

int WINAPI  ProcessEventW(HANDLE hPlugin, int Event, void *Param) {
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		return p->ProcessEvent(Event, Param);
	} catch (...) {
		return FALSE;
	}
}

int WINAPI  ProcessDialogEventW(int Event, void *Param) {
	try {
		if (Event == DE_DEFDLGPROCINIT) {
			FarDialogEvent *pevent = reinterpret_cast<FarDialogEvent*>(Param);
			if (pevent->Msg == DN_KEY) {
				if (KEY_F12 == pevent->Param2) {
					int a = 0;
				}
			}
		}
		return 0;
	} catch (...) {
		return FALSE;
	}
}
