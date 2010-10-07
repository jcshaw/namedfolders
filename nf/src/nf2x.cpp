/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
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
// extern "C"
// {
// 	void WINAPI _export SetStartupInfoW(struct PluginStartupInfo const* Info);
// 	HANDLE WINAPI _export OpenPluginW(int OpenFrom,int Item);
// 	void WINAPI _export GetPluginInfoW(struct PluginInfo* Info);
// 	void WINAPI _export GetOpenPluginInfoW(HANDLE hPlugin, struct OpenPluginInfo *Info);
// 	int WINAPI _export GetFindDataW(HANDLE hPlugin, struct PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode);
// 	int WINAPI _export SetDirectoryW(HANDLE hPlugin,const wchar_t *Dir,int OpMode);
// 	void WINAPI _export ClosePluginW(HANDLE hPlugin);
// 	int WINAPI _export ProcessKeyW(HANDLE hPlugin,int Key,unsigned int ControlState);
// 	int WINAPI _export GetMinFarVersionW(void);
// 	int WINAPI _export ConfigureW(int ItemNumber);
// 	int WINAPI _export MakeDirectoryW(HANDLE hPlugin, wchar_t *Name, int OpMode);
// 	void WINAPI _export FreeFindDataW(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber);
// 	int WINAPI _export PutFilesW(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode);
// 	int WINAPI _export ProcessEventW(HANDLE hPlugin, int Event, void *Param);
// };

struct PluginStartupInfo g_PluginInfo; 
struct FarStandardFunctions g_FSF;

using namespace nf;
///////////////////////////////////////////////////////////////////////////////////////
void WINAPI _export SetStartupInfoW(struct PluginStartupInfo *pInfo)
{
	g_PluginInfo = *pInfo;
	g_FSF = *pInfo->FSF;
}

void WINAPI _export GetPluginInfoW(struct PluginInfo *pInfo)
{
	pInfo->DiskMenuStringsNumber = CSettings::GetInstance().GetValue(nf::ST_SHOW_IN_DISK_MENU) ? 1 : 0;
	pInfo->PluginMenuStringsNumber = CSettings::GetInstance().GetValue(nf::ST_SHOW_IN_PLUGINS_MENU) ? 1 : 0;
	pInfo->PluginConfigStringsNumber = 1;

	static const wchar_t *DiskMenuStrings[1];
	static const wchar_t *PluginMenuStrings[1];
	static const wchar_t *PluginConfigStrings[1];
	DiskMenuStrings[0] = nf::GetMsg(lg::DISKMENUSTRING);
	PluginMenuStrings[0]= nf::GetMsg(lg::PLUGINMENUSTRING);
	PluginConfigStrings[0]= nf::GetMsg(lg::PLUGINSCONFIGSTRING);

	static int hotkey = 0;
	hotkey = CSettings::GetInstance().GetValue(nf::ST_EDIT_MENU_DISK_FAST_KEY);
	if (hotkey >= 0 && hotkey <= 10) 
	{
		pInfo->DiskMenuNumbers = &hotkey;
	} else {
		pInfo->DiskMenuNumbers = 0;
	}

	pInfo->DiskMenuStrings= DiskMenuStrings;
	pInfo->PluginMenuStrings= PluginMenuStrings;
	pInfo->PluginConfigStrings = PluginConfigStrings;

	pInfo->StructSize = sizeof(*pInfo); 
	pInfo->Flags = PF_FULLCMDLINE;
	
	pInfo->CommandPrefix = CSettings::GetInstance().GetListPrefixes().c_str();
	return;
}

HANDLE WINAPI _export OpenPluginW(int OpenFrom, int Item)
{
	try {
		switch (OpenFrom)
		{
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
			return new nf::Panel::CPanel();
		break;
		}
	} catch (...) {
	}

	return INVALID_HANDLE_VALUE;
}

void WINAPI _export GetOpenPluginInfoW(HANDLE hPlugin, struct OpenPluginInfo *Info)
{
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		p->GetOpenPluginInfo(Info);
	} catch (...) {
	}
}

int WINAPI _export GetFindDataW(HANDLE hPlugin, struct PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode)
{
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		return p->GetFindData(pPanelItem, pItemsNumber, OpMode);
	} catch (...) {
		return FALSE;
	}
}

void WINAPI _export FreeFindDataW(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber)
{
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		p->FreeFindData(PanelItem, ItemsNumber);
	} catch (...) {
	}
}

int WINAPI _export SetDirectoryW(HANDLE hPlugin, const wchar_t *Dir, int OpMode)
{
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		return p->SetDirectory(Dir, OpMode);
	} catch (...) {
		return FALSE;
	}
}

void WINAPI _export ClosePluginW(HANDLE hPlugin)
{
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		delete p;
	} catch (...) {
	}
}

int WINAPI _export ProcessKeyW(HANDLE hPlugin,int Key,unsigned int ControlState)
{
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		return p->ProcessKey(Key, ControlState);
	} catch (...) {
		return FALSE;
	}
}

int WINAPI _export GetMinFarVersionW(void)
{
	return MAKEFARVERSION(2,0,1000);
}

int WINAPI _export ConfigureW(int ItemNumber)
{
	try {
		nf::CConfigureDialog dlg;
		return dlg.ShowModal();
	} catch (...) {
		return FALSE;
	}
}


int WINAPI _export MakeDirectoryW(HANDLE hPlugin, wchar_t *Name, int OpMode)
{
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		return p->MakeDirectory(Name, OpMode);
	} catch (...) {
		return FALSE;
	}
}

int WINAPI _export PutFilesW(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode)
{
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		return p->PutFiles(PanelItem, ItemsNumber, Move, OpMode);
	} catch (...) {
		return FALSE;
	}
}

int WINAPI _export ProcessEventW(HANDLE hPlugin, int Event, void *Param)
{
	try {
		nf::Panel::CPanel *p = reinterpret_cast<nf::Panel::CPanel*>(hPlugin);
		return p->ProcessEvent(Event, Param);
	} catch (...) {
		return FALSE;
	}
}
