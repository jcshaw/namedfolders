/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#include "StdAfx.h"
#include "menus_impl.h"
#include <Shlwapi.h>
#include <cassert>
#include "stlcatalogs.h"

using namespace nf;
using namespace Menu;

namespace {
	void switch_view_mode(int BreakCode) {
		CSettings::GetInstance().SetValue(nf::ST_SELECT_SH_MENU_MODE, BreakCode-1);	
		CSettings::GetInstance().SaveSettings();
	}
}

// actions
int CMenuShortcuts::MakeAction(int BreakCode) {
	switch (BreakCode) {
	case 0: return -MS_COMMAND_DELETE; //Del - remove selected shortcut
	case 3: return -MS_COMMAND_EDIT; //F4 - edit selected shortcut
	case 6: switch_view_mode(3); //F7 is used instead of F4 since build 243.
	default: switch_view_mode(BreakCode);
	}	
	return 0;
}

int CMenuApplications::MakeAction(int BreakCode) {
	switch(BreakCode) {
	case 0: return -OPEN_PATH_IN_FAR; 
	case 1: return -OPEN_PATH_IN_EXPLORER;
	case 2: //switch catalogs view mode 
		CSettings::GetInstance().SetValue(nf::ST_SELECT_SH_MENU_SHOWCATALOGS_MODE
				, get_ignore_mode() == MODE_IGNORE_EXCEPTIONS_ON 
					? MODE_IGNORE_EXCEPTIONS_OFF 
					: MODE_IGNORE_EXCEPTIONS_ON);	
		return -SWITCH_IGNORE_MODE_ONOFF;
	default:
		assert(BreakCode == 3 || BreakCode == 4);
		CSettings::GetInstance().SetValue(nf::ST_SELECT_SOFT_MENU_SHOWCATALOGS_MODE, BreakCode-3);
		CSettings::GetInstance().SaveSettings();
	}
	return 0;	
}

int CMenuEnvironmentVariables::MakeAction(int BreakCode) {	
	CSettings::GetInstance().SetValue(nf::ST_SELECT_SH_MENU_MODE_EV, BreakCode+1);	
	CSettings::GetInstance().SaveSettings();
	return 0;
}