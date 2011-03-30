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

namespace {
	int call_menu(HANDLE hPlugin,  DWORD menuFlags, wchar_t const* const* menuStrings, unsigned int itemsNumber, unsigned int buttonsNumber, tsetting_flags confirmationFlag) {
		if (confirmationFlag != 0 && CSettings::GetInstance().GetValue(confirmationFlag) == 0) return 1;	//confirmation is not required

		if (g_PluginInfo.Message(g_PluginInfo.ModuleNumber
			, menuFlags
			, 0
			, menuStrings
			, itemsNumber
			, buttonsNumber) == 0) return 2;
		else return 0;
	}
}

//ask confirmation to create shortcut with implicitly specified name
UINT nf::Confirmations::AskForImplicitInsert(HANDLE hPlugin, nf::tshortcut_info const&cmd, tstring const& srcValue) {
	wchar_t const * msg[4] = {
		GetMsg(lg::MSG_INSERT_SHORTCUT)
		, GetMsg(lg::CONFIRM_INSERT_SHORTCUT)
		, cmd.shortcut.c_str()
		, srcValue.c_str()
	};

	return call_menu(hPlugin, FMSG_MB_OKCANCEL, &msg[0], sizeof(msg)/sizeof(msg[0]), 3, nf::ST_CONFIRM_IMPLICIT_CREATION);
}

//ask confirmation for shortcut overriding
UINT nf::Confirmations::AskForOverride(HANDLE hPlugin, nf::tshortcut_info const&cmd, tstring const& srcValue) {
	tstring s = tstring(GetMsg(lg::MSG_OVERRIDE_SHORTCUT)) + L": " + cmd.shortcut;

	wchar_t const * msg[5] = {
		s.c_str()
		, GetMsg(lg::CONFIRM_OVERRIDE_SHORTCUT)
		, srcValue.c_str()
		, GetMsg(lg::MSG_OVERRIDE)
		, GetMsg(lg::MSG_CANCEL) 
	};

	return call_menu(hPlugin, FMSG_WARNING, &msg[0], sizeof(msg)/sizeof(msg[0]), 2, nf::ST_CONFIRM_OVERRIDE);
}

UINT nf::Confirmations::AskToGoToNearest(HANDLE hPlugin, tstring const& srcDir, tstring const& nearestDir) {
	wchar_t const * msg[5] = {
		GetMsg(lg::MSG_ERROR)
		, GetMsg(lg::MSG_PATH_NOT_FOUND)
		, srcDir.c_str()
		, GetMsg(lg::MSG_JUMP_TO_THE_NEAREST_EXISTING_FOLDER)
		, nearestDir.c_str() 
	};
	return call_menu(hPlugin,  FMSG_WARNING | FMSG_MB_YESNO, msg, sizeof(msg)/sizeof(msg[0]), 2, nf::ST_CONFIRM_GO_TO_NEAREST);
}

nf::Confirmations::tconfirm_delete_result 
nf::Confirmations::Private::ask_for_delete_general(tstring const& srcTitle, int Msg0, int Msg1, bool bSeveral) {	
	const unsigned int number_buttons = bSeveral ? 4 : 2;
	const wchar_t* Msg[7];	
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
	const wchar_t * msg[3] = {
		GetMsg(lg::DLG_CREATE_CATALOG_TITLE)
		, GetMsg(lg::CONFIRM_INSERT_CATALOG)
		, cat.c_str()
	};

	return call_menu(hPlugin,  FMSG_MB_OKCANCEL, &msg[0], sizeof(msg)/sizeof(msg[0]), 3, nf::ST_CONFIRM_IMPLICIT_CREATION);
}


UINT nf::Confirmations::AskForAddRemoveDirectoryToWin7Library(HANDLE hPlugin, tstring const& targetLibrary, tstring const& targetDirectory, bool bRemove) {
	wchar_t const * msg[4] = {
		GetMsg(lg::MSG_WIN7_LIBRARIES)
		, GetMsg(bRemove ? lg::CONFIRM_REMOVE_DIR_FROM_WIN7_LIBRARY : lg::CONFIRM_ADD_DIR_TO_WIN7_LIBRARY)
		, targetDirectory.c_str()
		, targetLibrary.c_str()
	};

	return call_menu(hPlugin, FMSG_MB_OKCANCEL, &msg[0], sizeof(msg)/sizeof(msg[0]), 3, nf::ST_CONFIRM_IMPLICIT_CREATION);

}