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

// actions
int CMenuShortcuts::MakeAction(int BreakCode) {
	if (BreakCode == 0) { //Del - remove selected shortcut
		return -1;
	} else  {	//change menu view mode; save new mode
		CSettings::GetInstance().SetValue(nf::ST_SELECT_SH_MENU_MODE, BreakCode-1);	
		CSettings::GetInstance().SaveSettings();
	}
	return 0;
}

int CMenuApplications::MakeAction(int BreakCode) {
	switch(BreakCode) {
	case 0:  return -OPEN_PATH_IN_FAR; 
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