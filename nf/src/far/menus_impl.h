/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

#include "Kernel.h"
#include "settings.h"
namespace nf
{

namespace Menu
{
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// интерфейс класса меню
	class tmenu {
	public:
		enum {
			FG_SHOW_SINGLE_VARIANT = 0x1	//показывать меню даже если вариант единственный 
		};
		tstring const m_KeysInMenu;
		tstring const m_HelpTopic;
		int const m_Flags;
		nf::tsetting_flags m_MenuMode;

		tmenu(wchar_t const* KeysInMenu = L"", wchar_t const* HelpTopic = L"", int Flags = 0
			, nf::tsetting_flags MenuMode = static_cast<nf::tsetting_flags>(0))
			: m_HelpTopic(HelpTopic)
			, m_KeysInMenu(KeysInMenu)
			, m_Flags(Flags)
			, m_MenuMode(MenuMode)
		{}
	public:
		virtual tstring GetMenuTitle() {return L"";}
		virtual int* GetBreakKeys() {return 0;}
		int GetCurrentMenuMode() {
			return m_MenuMode != 0 ? nf::CSettings::GetInstance().GetValue(m_MenuMode) : 0;
		}
		virtual int MakeAction(int BreakCode) {return 0;}
	};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//€рлыки
	class tshortcuts_menu : public tmenu
	{
	public:
		tshortcuts_menu() : tmenu(L"Del, F2-F6", L"MenuSelect", 0, nf::ST_SELECT_SH_MENU_MODE) {}
		enum {
			MM_SH //только псевдоним
			, MM_VAL //только значение
			, MM_SH_VAL	//псевдоним и значение
			, MM_CAT_SH //каталог и псевдоним
			, MM_CAT_SH_VAL	//каталог, псевдоним и значение

			, NUMBER_MENU_MODES  //кол-во режимов отображени€ меню
		};	 
	public:
		virtual int* GetBreakKeys() 
		{
			static int shortcuts_break_keys [] = {VK_DELETE, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, 0};
			return &shortcuts_break_keys[0];
		}
		virtual int MakeAction(int BreakCode);
	};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//программы
	class tsoft_menu : public tmenu
	{
	public:
		tsoft_menu() : tmenu(L"F11, Ctrl+Enter, Shift+Enter, F2, F3", L""
			, tmenu::FG_SHOW_SINGLE_VARIANT
			, nf::ST_SELECT_SOFT_MENU_SHOWCATALOGS_MODE) {}
		enum {
			OPEN_PATH_IN_EXPLORER = VK_RETURN | PKF_CONTROL << 16
			, OPEN_PATH_IN_FAR = VK_RETURN | PKF_SHIFT << 16
			, SWITCH_IGNORE_MODE_ONOFF = VK_F11
		};

		enum {
			MODE_IGNORE_ON = 0		//не искать среди €рлыков, удовлетвор€ющих игнорируемым в соответствии с маской в настройках 
			, MODE_IGNORE_OFF = 1	//искать среди всех €рлыков, ничего не игнорировать
		};

		enum {
			MM_PATH
			, MM_CAT_PATH
		};
		virtual int MakeAction(int BreakCode);
	public:
		virtual int* GetBreakKeys() 
		{
			static int soft_break_keys[] = {OPEN_PATH_IN_EXPLORER, OPEN_PATH_IN_FAR, SWITCH_IGNORE_MODE_ONOFF, VK_F2, VK_F3, 0};				
			return &soft_break_keys[0];
		}
		virtual tstring GetMenuTitle() 
		{
			return MODE_IGNORE_OFF == get_ignore_mode() ? L"" : L"*";
		}
	private:
		inline int get_ignore_mode() { return CSettings::GetInstance().GetValue(nf::ST_SELECT_SH_MENU_SHOWCATALOGS_MODE);}
	};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//переменные среды
	class tenv_menu : public tmenu
	{
	public:
		tenv_menu() : tmenu(L"F2, F3", L"", 0, nf::ST_SELECT_SH_MENU_MODE_EV) {}
		enum {
			MM_VAR_PATH	//с названиеи переменной
			, MM_PATH //без названи€ переменной
			, NUMBER_MENU_MODES
		};	
	public:
		virtual int MakeAction(int BreakCode);
		virtual int* GetBreakKeys()
		{
			static int env_var_break_keys[] = {VK_F2, VK_F3, 0};	
			return &env_var_break_keys[0];
		}
	};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//пути
	typedef tmenu tpath_menu;
//каталоги
	typedef tmenu tcatalogs_menu;

};//menu

} //nf