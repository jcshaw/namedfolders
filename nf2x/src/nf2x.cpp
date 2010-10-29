/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
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
extern "C"
{
	void WINAPI _export SetStartupInfo(struct PluginStartupInfo const* Info);
	HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item);
	void WINAPI _export GetPluginInfo(struct PluginInfo* Info);
	void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin, struct OpenPluginInfo *Info);
	int WINAPI _export GetFindData(HANDLE hPlugin, struct PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode);
	int WINAPI _export SetDirectory(HANDLE hPlugin,const char *Dir,int OpMode);
	void WINAPI _export ClosePlugin(HANDLE hPlugin);
	int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState);
	int WINAPI _export GetMinFarVersion(void);
	int WINAPI _export Configure(int ItemNumber);
	int WINAPI _export MakeDirectory (HANDLE hPlugin, char *Name, int OpMode);
	void WINAPI _export FreeFindData(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber);
	int WINAPI _export PutFiles(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode);
	int WINAPI _export ProcessEvent(HANDLE hPlugin, int Event, void *Param);
};

struct PluginStartupInfo g_PluginInfo; 
struct FarStandardFunctions g_FSF;

using namespace nf;
///////////////////////////////////////////////////////////////////////////////////////
void WINAPI _export SetStartupInfo(struct PluginStartupInfo *pInfo)
{
	g_PluginInfo = *pInfo;
	g_FSF = *pInfo->FSF;
}

void WINAPI _export GetPluginInfo(struct PluginInfo *pInfo)
{
	pInfo->DiskMenuStringsNumber = CSettings::GetInstance().GetValue(nf::ST_SHOW_IN_DISK_MENU) ? 1 : 0;
	pInfo->PluginMenuStringsNumber = CSettings::GetInstance().GetValue(nf::ST_SHOW_IN_PLUGINS_MENU) ? 1 : 0;
	pInfo->PluginConfigStringsNumber = 1;

	static const char *DiskMenuStrings[1];
	static const char *PluginMenuStrings[1];
	static const char *PluginConfigStrings[1];
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

HANDLE WINAPI _export OpenPlugin(int OpenFrom, int Item)
{
	try {
		switch (OpenFrom)
		{
		case OPEN_COMMANDLINE: 
			{
#pragma warning(disable: 4312)
				char const* pCmd = reinterpret_cast<char const*>(Item);
#pragma warning(default: 4312)
				return nf::OpenFromCommandLine(pCmd).first;
			}
		break;
		case OPEN_PLUGINSMENU:
			return nf::OpenFromPluginsMenu();
			break;
		case OPEN_DISKMENU:
			return new nf::CPanel();
		break;
		}
	} catch (...)
	{
	}

	return INVALID_HANDLE_VALUE;
}

void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin, struct OpenPluginInfo *Info)
{
	try {
		nf::CPanel *p = reinterpret_cast<nf::CPanel*>(hPlugin);
		p->GetOpenPluginInfo(Info);
	} catch (...)
	{
	}
}

int WINAPI _export GetFindData(HANDLE hPlugin, struct PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode)
{
	try {
		nf::CPanel *p = reinterpret_cast<nf::CPanel*>(hPlugin);
		return p->GetFindData(pPanelItem, pItemsNumber, OpMode);
	} catch (...)
	{
		return FALSE;
	}
}

void WINAPI _export FreeFindData(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber)
{
	try {
		nf::CPanel *p = reinterpret_cast<nf::CPanel*>(hPlugin);
		p->FreeFindData(PanelItem, ItemsNumber);
	} catch (...)
	{
	}
}

int WINAPI _export SetDirectory(HANDLE hPlugin, const char *Dir, int OpMode)
{
	try {
		nf::CPanel *p = reinterpret_cast<nf::CPanel*>(hPlugin);
		return p->SetDirectory(Dir, OpMode);
	} catch (...)
	{
		return FALSE;
	}
}

void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
	try {
		nf::CPanel *p = reinterpret_cast<nf::CPanel*>(hPlugin);
		delete p;
	} catch (...)
	{
		
	}
}

int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
	try {
		nf::CPanel *p = reinterpret_cast<nf::CPanel*>(hPlugin);
		return p->ProcessKey(Key, ControlState);
	} catch (...)
	{
		return FALSE;
	}
}

int WINAPI _export GetMinFarVersion(void)
{
	return MAKEFARVERSION(1,70,1282);
}

int WINAPI _export Configure(int ItemNumber)
{
	try {
		nf::CConfigureDialog dlg;
		return dlg.ShowModal();
	} catch (...)
	{
		return FALSE;
	}
}


int WINAPI _export MakeDirectory (HANDLE hPlugin, char *Name, int OpMode)
{
	try {
		nf::CPanel *p = reinterpret_cast<nf::CPanel*>(hPlugin);
		return p->MakeDirectory(Name, OpMode);
	} catch (...)
	{
		return FALSE;
	}
}

int WINAPI _export PutFiles(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode)
{
	try {
		nf::CPanel *p = reinterpret_cast<nf::CPanel*>(hPlugin);
		return p->PutFiles(PanelItem, ItemsNumber, Move, OpMode);
	} catch (...)
	{
		return FALSE;
	}
}

int WINAPI _export ProcessEvent(HANDLE hPlugin, int Event, void *Param)
{
	try {
		nf::CPanel *p = reinterpret_cast<nf::CPanel*>(hPlugin);
		return p->ProcessEvent(Event, Param);
	} catch (...)
	{
		return FALSE;
	}
}
