/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
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
	void switch_mode(nf::tsetting_flags flag, int modeValue) {
		CSettings::GetInstance().SetValue(flag, modeValue);	
		CSettings::GetInstance().SaveSettings();
	}
}

// actions
int CMenuShortcuts::MakeAction(int breakCode) {
	switch (breakCode) {
	case 0: return -MS_COMMAND_DELETE; //Del - remove selected shortcut
	case 1: return -MS_COMMAND_EDIT; //F4 - edit selected shortcut
	case 2: case 3: case 4: case 5: case 6: //view mode
		switch_mode(nf::ST_SELECT_SH_MENU_MODE, breakCode - 2); 
		break;
	case 7: case 8: case 9://sort mode
		switch_mode(nf::ST_SORT_SH_MENU_COLUMN, breakCode - 6); 
		break;
	case 10: //no sort 
		switch_mode(nf::ST_SORT_SH_MENU_COLUMN, 0); 
		break;
	}	
	return 0;
}

int CMenuApplications::MakeAction(int breakCode) {
	switch(breakCode) {
	case 0: return -OPEN_PATH_IN_FAR; 
	case 1: return -OPEN_PATH_IN_EXPLORER;
	case 2: //switch catalogs view mode 
		CSettings::GetInstance().SetValue(nf::ST_SELECT_SH_MENU_SHOWCATALOGS_MODE
				, get_ignore_mode() == MODE_IGNORE_EXCEPTIONS_ON 
					? MODE_IGNORE_EXCEPTIONS_OFF 
					: MODE_IGNORE_EXCEPTIONS_ON);	
		return -SWITCH_IGNORE_MODE_ONOFF;
	case 3: case 4: case 5:
		switch_mode(nf::ST_SELECT_SOFT_MENU_SHOWCATALOGS_MODE, breakCode-3);
		break;
	case 6: case 7: //sort mode
		switch_mode(nf::ST_SORT_SOFT_MENU_COLUMN, breakCode - 5); 
		break;
	case 8: //no sort
		switch_mode(nf::ST_SORT_SOFT_MENU_COLUMN, 0);
		break;
	}
	return 0;	
}

int CMenuEnvironmentVariables::MakeAction(int breakCode) {	
	switch(breakCode) {
	case 0: case 1: //view mode
		switch_mode(nf::ST_SELECT_SH_MENU_MODE_EV, breakCode + 1);
		break;
	case 2: case 3: //sort mode
		switch_mode(nf::ST_SORT_SH_MENU_EV_COLUMN, breakCode - 1);
		break;
	case 4: //no sort
		switch_mode(nf::ST_SORT_SH_MENU_EV_COLUMN, 0);
		break;
	}
	return 0;
}