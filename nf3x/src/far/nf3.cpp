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
	/// GUID плагина Named Folders (требуется в FAR3)
	/// {C6A35DB8-6846-449E-A1DC-2FC152ED4B0B}
	const GUID NF_PLUGIN_GUID = { 0xc6a35db8, 0x6846, 0x449e, { 0xa1, 0xdc, 0x2f, 0xc1, 0x52, 0xed, 0x4b, 0xb } };
	/// {B90A8402-14F7-4382-9275-C79824DD8A24}
	const GUID NF_DISKMENU_GUID = { 0xb90a8402, 0x14f7, 0x4382, { 0x92, 0x75, 0xc7, 0x98, 0x24, 0xdd, 0x8a, 0x24 } };

	// {FE4E3C90-3590-4F76-9D05-D30FA3338F9E}
	const GUID NF_PLUGINSMENU_GUID = { 0xfe4e3c90, 0x3590, 0x4f76, { 0x9d, 0x5, 0xd3, 0xf, 0xa3, 0x33, 0x8f, 0x9e } };

	// {F30CBBA9-397A-4BF9-81FB-3154B8FBCD90}
	const GUID NF_CONFIGMENU_GUID = { 0xf30cbba9, 0x397a, 0x4bf9, { 0x81, 0xfb, 0x31, 0x54, 0xb8, 0xfb, 0xcd, 0x90 } };

	// {F5EA3E3D-A578-4C1B-B1EA-D6CCB33175CF}
	const GUID NF_DIALOG_CONFIGURE = { 0xf5ea3e3d, 0xa578, 0x4c1b, { 0xb1, 0xea, 0xd6, 0xcc, 0xb3, 0x31, 0x75, 0xcf } };

	// {38730031-17D8-4985-BFA6-DCD3613C2364}
	const GUID NF_DIALOG_APPLY_COMMAND = { 0x38730031, 0x17d8, 0x4985, { 0xbf, 0xa6, 0xdc, 0xd3, 0x61, 0x3c, 0x23, 0x64 } };

	// {A5B778D5-A0A0-41C2-9F7B-350EB9BE6969}
	const GUID NF_DIALOG_EDIT_CATALOG = { 0xa5b778d5, 0xa0a0, 0x41c2, { 0x9f, 0x7b, 0x35, 0xe, 0xb9, 0xbe, 0x69, 0x69 } };

	// {47D727D5-9699-4A4B-89D0-5809E426806E}
	const GUID NF_DIALOG_EDIT_PATTERN = { 0x47d727d5, 0x9699, 0x4a4b, { 0x89, 0xd0, 0x58, 0x9, 0xe4, 0x26, 0x80, 0x6e } };

	// {4E3B3296-9B39-406A-864C-1723CF309C4D}
	const GUID NF_DIALOG_EDIT_SHORTCUT = { 0x4e3b3296, 0x9b39, 0x406a, { 0x86, 0x4c, 0x17, 0x23, 0xcf, 0x30, 0x9c, 0x4d } };

	// {813CFDA1-711D-4760-99B0-1ED7D6C7D0C0}
	const GUID NF_DIALOG_MOVE = { 0x813cfda1, 0x711d, 0x4760, { 0x99, 0xb0, 0x1e, 0xd7, 0xd6, 0xc7, 0xd0, 0xc0 } };

	// {A23F828E-FC4D-46B9-BEBD-6C9CC8F9435D}
	static const GUID NF_MESSAGE_SAFE_PANEL_STATE = { 0xa23f828e, 0xfc4d, 0x46b9, { 0xbe, 0xbd, 0x6c, 0x9c, 0xc8, 0xf9, 0x43, 0x5d } };
}

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

void WINAPI  GetOpenPluginInfoW(HANDLE hPlugin, struct InfoPanelLine *info) {
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
//  !TODO: ProcessPanelInputW  
// int WINAPI  ProcessKeyW(HANDLE hPlugin,int Key,unsigned int ControlState) {
// 	try {
// 		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
// 		return p->ProcessKey(Key, ControlState);
// 	} catch (...) {
// 		return FALSE;
// 	}
// }

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
