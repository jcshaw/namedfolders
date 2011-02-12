/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

#include "Kernel.h"
#include "settings.h"

namespace nf {
namespace Menu {
	class CMenu {
	public:
		enum {
			FG_SHOW_SINGLE_VARIANT = 0x1	//показывать меню даже если вариант единственный 
		};
		enum { //keysequences for sorting menu by 
			F_NO_SORT_BY_COLUMNS = VK_F12 | PKF_CONTROL << 16
			, F_SORT_COLUMN_1 = VK_F5 | PKF_CONTROL << 16
			, F_SORT_COLUMN_2 = VK_F6 | PKF_CONTROL << 16
			, F_SORT_COLUMN_3 = VK_F7 | PKF_CONTROL << 16
		};

		tstring const m_KeysInMenu;
		tstring const m_HelpTopic;
		int const m_Flags;
		nf::tsetting_flags m_MenuMode;
		nf::tsetting_flags m_SortMenuMode;

		CMenu(wchar_t const* KeysInMenu = L"", wchar_t const* HelpTopic = L"", int Flags = 0
			, nf::tsetting_flags MenuMode = static_cast<nf::tsetting_flags>(0)
			, nf::tsetting_flags sortMenuMode = static_cast<nf::tsetting_flags>(0))
			: m_HelpTopic(HelpTopic)
			, m_KeysInMenu(KeysInMenu)
			, m_Flags(Flags)
			, m_MenuMode(MenuMode)
			, m_SortMenuMode(sortMenuMode)
		{}
	public:
		virtual tstring GetMenuTitle() {return L"";}
		virtual int* GetBreakKeys() {return 0;}
		int GetCurrentViewMode() {
			return m_MenuMode != 0 ? nf::CSettings::GetInstance().GetValue(m_MenuMode) : 0;
		}
		int GetCurrentSortMode() {
			return m_SortMenuMode != 0 ? nf::CSettings::GetInstance().GetValue(m_SortMenuMode) : 0;
		}
		virtual int MakeAction(int BreakCode) {return 0;}
	};

	class CMenuShortcuts : public CMenu {
	public:
		CMenuShortcuts() : CMenu(L"Del,F4; F5-F9; Ctrl+F5-F7,F12", L"MenuSelect", 0, nf::ST_SELECT_SH_MENU_MODE, nf::ST_SORT_SH_MENU_COLUMN) {}
		enum {
			MM_SHORTCUTS_ONLY 
			, MM_VALUES_ONLY 
			, MM_SHORTCUTS_AND_VALUES	
			, MM_CATALOGS_AND_SHORTCUTS 
			, MM_CATALOGS_SHORTCUTS_VALUES	

			, NUMBER_MENU_MODES  
		};	 
		enum {
			MS_COMMAND_DELETE = 1
			, MS_COMMAND_EDIT = 2
		};
	public:
		virtual int* GetBreakKeys() {
			static int shortcuts_break_keys [] = {VK_DELETE, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9
				, CMenu::F_SORT_COLUMN_1, CMenu::F_SORT_COLUMN_2, CMenu::F_SORT_COLUMN_3, CMenu::F_NO_SORT_BY_COLUMNS, 0};
			return &shortcuts_break_keys[0];
		}
		virtual int MakeAction(int BreakCode);
	};

	class CMenuApplications : public CMenu {
	public:
		CMenuApplications() : CMenu(L"F11, Ctrl+Enter, Shift+Enter; F5-F7; Ctrl+F5,F6,F12", L"StartApplications"
			, CMenu::FG_SHOW_SINGLE_VARIANT
			, nf::ST_SELECT_SOFT_MENU_SHOWCATALOGS_MODE
			, nf::ST_SORT_SOFT_MENU_COLUMN) {}
		enum {
			OPEN_PATH_IN_EXPLORER = VK_RETURN | PKF_CONTROL << 16
			, OPEN_PATH_IN_FAR = VK_RETURN | PKF_SHIFT << 16
			, SWITCH_IGNORE_MODE_ONOFF = VK_F11
		};
		enum {
			MODE_IGNORE_EXCEPTIONS_ON = 0		//не искать среди ярлыков, удовлетворяющих игнорируемым в соответствии с маской в настройках 
			, MODE_IGNORE_EXCEPTIONS_OFF = 1	//искать среди всех ярлыков, ничего не игнорировать
		};
		enum { MM_PATH, MM_CAT_PATH, MM_PATH_CAT };
		virtual int MakeAction(int BreakCode);
	public:
		virtual int* GetBreakKeys() {
			static int soft_break_keys[] = {OPEN_PATH_IN_EXPLORER, OPEN_PATH_IN_FAR, SWITCH_IGNORE_MODE_ONOFF, VK_F5, VK_F6, VK_F7
				, CMenu::F_SORT_COLUMN_1, CMenu::F_SORT_COLUMN_2, CMenu::F_NO_SORT_BY_COLUMNS, 0};				
			return &soft_break_keys[0];
		}
		virtual tstring GetMenuTitle() {
			return MODE_IGNORE_EXCEPTIONS_OFF == get_ignore_mode() ? L"" : L"*";
		}
	private:
		inline int get_ignore_mode() { return CSettings::GetInstance().GetValue(nf::ST_SELECT_SH_MENU_SHOWCATALOGS_MODE);}
	};

	class CMenuEnvironmentVariables : public CMenu {
	public:
		CMenuEnvironmentVariables() : CMenu(L"F5,F6; Ctrl+F5,F6,F12", L"MenuSelect", 0, nf::ST_SELECT_SH_MENU_MODE_EV, nf::ST_SORT_SH_MENU_EV_COLUMN) {}
		enum {
			MM_VARIABLE_AND_PATH	
			, MM_PATH_ONLY 

			, NUMBER_MENU_MODES
		};	
	public:
		virtual int MakeAction(int BreakCode);
		virtual int* GetBreakKeys() {
			static int env_var_break_keys[] = {VK_F5, VK_F6
				, CMenu::F_SORT_COLUMN_1, CMenu::F_SORT_COLUMN_2, CMenu::F_NO_SORT_BY_COLUMNS,  0};	
			return &env_var_break_keys[0];
		}
	};

	typedef CMenu CMenuPaths;
	typedef CMenu CMenuCatalogs;
}
}