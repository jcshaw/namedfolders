/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "confirmations.h"
#include "header.h"
#include "settings.h"

using namespace nf;
using namespace nf::Confirmations;

//ask confrimation to create shortcut with implicitly specified name
UINT nf::Confirmations::AskForImplicitInsert(HANDLE hPlugin, nf::tshortcut_info const&cmd, tstring const& srcValue) {
	if (CSettings::GetInstance().GetValue(nf::ST_CONFIRM_IMPLICIT_CREATION) == 0) return 1;	//confirmation is not required

	const wchar_t * Msg[4];
	Msg[0] = GetMsg(lg::MSG_INSERT_SHORTCUT);      
	Msg[1] = GetMsg(lg::CONFIRM_INSERT_SHORTCUT);   
	Msg[2] = cmd.shortcut.c_str();
	Msg[3] = srcValue.c_str();

	if (g_PluginInfo.Message(g_PluginInfo.ModuleNumber
		, FMSG_MB_OKCANCEL
		, 0
		, Msg
		, sizeof(Msg)/sizeof(Msg[0])
		, 3) == 0) return 2;
	else return 0;
}

//ask confirmation for shortcut overriding
UINT nf::Confirmations::AskForOverride(HANDLE hPlugin, nf::tshortcut_info const&cmd, tstring const& srcValue) {
	if (CSettings::GetInstance().GetValue(nf::ST_CONFIRM_OVERRIDE) == 0) return 1;	//confirmation is not required
	tstring s = tstring(GetMsg(lg::MSG_OVERRIDE_SHORTCUT)) + L": " + cmd.shortcut;

	const wchar_t * Msg[5];
	Msg[0] = s.c_str();      
	Msg[1] = GetMsg(lg::CONFIRM_OVERRIDE_SHORTCUT);   
	Msg[2] = srcValue.c_str();
	Msg[3] = GetMsg(lg::MSG_OVERRIDE);      
	Msg[4] = GetMsg(lg::MSG_CANCEL);

	if (g_PluginInfo.Message(g_PluginInfo.ModuleNumber
		, FMSG_WARNING 
		, 0
		, Msg
		, sizeof(Msg)/sizeof(Msg[0])
		, 2) == 0) return 2;
	else return 0;
}

UINT nf::Confirmations::AskToGoToNearest(HANDLE hPlugin, tstring const& srcDir, tstring const& nearestDir) {
	if (CSettings::GetInstance().GetValue(nf::ST_CONFIRM_GO_TO_NEAREST) == 0) return 1;	//confirmation is not required

	const wchar_t* Msg[5];
	Msg[0] = GetMsg(lg::MSG_ERROR);   
	Msg[1] = GetMsg(lg::MSG_PATH_NOT_FOUND);
	Msg[2] = srcDir.c_str();
	Msg[3] = GetMsg(lg::MSG_JUMP_TO_THE_NEAREST_EXISTING_FOLDER);
	Msg[4] = nearestDir.c_str();

	if (g_PluginInfo.Message(g_PluginInfo.ModuleNumber
		, FMSG_WARNING | FMSG_MB_YESNO
		, 0
		, Msg
		, sizeof(Msg)/sizeof(Msg[0])
		, 2) == 0) return 2;
	else return 0;
}

nf::Confirmations::tconfirm_delete_result 
nf::Confirmations::Private::ask_for_delete_general(tstring const& srcTitle, int Msg0, int Msg1, bool bSeveral) {	
	const unsigned int number_buttons = bSeveral ? 4 : 2;
	const wchar_t* Msg[7];	//std::vector<const char*> Msg(bSeveral ? 7 : 5);
	Msg[0] = GetMsg(Msg0);
	Msg[1] = GetMsg(Msg1);
	Msg[2] = srcTitle.c_str();
	Msg[3] = GetMsg(lg::MSG_DELETE);  
	if (bSeveral) {
		Msg[4] = GetMsg(lg::MSG_DELETE_ALL);
		Msg[5] = GetMsg(lg::MSG_SKIP);
		Msg[6] = GetMsg(lg::MSG_CANCEL);	
	} else {
		Msg[4] = GetMsg(lg::MSG_CANCEL);	
	}

	int code = g_PluginInfo.Message(g_PluginInfo.ModuleNumber
		, FMSG_WARNING 
		, 0
		, &Msg[0]
		, bSeveral ? 7 : 5
		, number_buttons);

	if (bSeveral) {
		switch (code) {
			case 0: return R_DELETE;
			case 1: return R_DELETEALL;
			case 2: return R_SKIP;	
			default: return R_CANCEL;
		};
	} else {
		switch (code) {
			case 0: return R_DELETE;
			default: return R_CANCEL;
		};

	}
}

UINT nf::Confirmations::AskForCreateCatalog(HANDLE hPlugin, nf::tcatalog_info const&cat) {
	if (CSettings::GetInstance().GetValue(nf::ST_CONFIRM_IMPLICIT_CREATION) == 0) return 1;	//confirmation is not required

	const wchar_t * Msg[3];
	Msg[0] = GetMsg(lg::DLG_CREATE_CATALOG_TITLE);
	Msg[1] = GetMsg(lg::CONFIRM_INSERT_CATALOG);   
	Msg[2] = cat.c_str();

	if (g_PluginInfo.Message(g_PluginInfo.ModuleNumber
		, FMSG_MB_OKCANCEL
		, 0
		, Msg
		, sizeof(Msg)/sizeof(Msg[0])
		, 3) == 0) return 2;
	else return 0;
	return 2;
}

