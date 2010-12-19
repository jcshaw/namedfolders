/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include <vector>
#include "plugin.hpp"

//глобальные переменные и настройки
namespace nf {
	typedef enum tsetting_strings {
		STS_PREFIXES
		, ST_SOFT_MASKS_TO_IGNORE_COMMA_SEPARETED	//список масок игнорируемых "cd: soft"
		, STS_PANELWIDTH //настройки ширины панели в виде N,Z,CO;10,0,10
		, ST_ASTERIX_MODE	//режим автоматического расширения масок звездочками: 0 - *a*, 1 - a*, 2 - a (не расширять)

		, NUMBER_STRING_SETTINGS 
	} _tsetting_strings;

	typedef enum tsetting_flags {	//флаги настроек
		ST_SHOW_ERRORS					//показывать ошибки
		, ST_SHOW_IN_PLUGINS_MENU		//отображать название плагина в меню плагинов
		, ST_SHOW_IN_DISK_MENU			//показывать название плагина в меню дисков
		, ST_SHOW_TEMPORARY_AS_HIDDEN	//показывать временные плагины как скрытые файлы
		, ST_CONFIRM_DELETE				//подтверждать удаление псевдонимов 
		, ST_CONFIRM_DELETE_CATALOGS	//подтверждать удаление каталогов
		, ST_CONFIRM_DELETE_NOT_EMPTY_CATALOGS //подтверждать удаление непустых каталогов
		, ST_CONFIRM_OVERRIDE			//подтверждать перезапись значения псевдонима
		, ST_CONFIRM_IMPLICIT_CREATION	//подтверждать создание псевдонима по команде cd::, cd:+
		, ST_CONFIRM_IMPLICIT_DELETION	//подтверждать удаление псевдонима по команде cd:-
		, ST_CONFIRM_GO_TO_NEAREST		//подтверждать переход в ближайшую директорию
		, ST_SHOW_KEYS_IN_MENU		//показывать в меню сочетания клавиш
		, ST_SELECT_SH_MENU_MODE	//режим отображения меню со списком вариантов псевдонимов
		, ST_PANEL_MODE				//режим отображения панели
		, ST_SELECT_CATS_MENU_MODE	//режим отображения меню со списком вариантов каталогов
		, ST_ALWAYS_EXPAND_SHORTCUTS	//всегда расширять однобуквенные псевдонимы
		, ST_HISTORY_IN_DIALOG_APPLY_COMMAND	//использовать историю в диалоге выполнения команды
		, ST_SELECT_SH_MENU_MODE_EV		//режим отображения меню со списком переменных среды
		, ST_SELECT_SH_MENU_SHOWCATALOGS_MODE // режим отображнеия каталогов в меню вариантов
		//, ST_EDIT_MENU_DISK_FAST_KEY // клавиша быстрого доступа в меню дисков
		, ST_FLAG_NETWORK_COMMANDS_THROUGH_COMMAND_LINE //переходить в сетевые директории с помощью плагина Network
		, ST_SELECT_SOFT_MENU_SHOWCATALOGS_MODE // режим отображения каталогов в меню запуска программ
		, ST_USE_SINGLE_MENU_MODE
		, ST_SUBDIRECTORIES_AS_ALIASES

		//NF 3.0
		, ST_SHOW_CATALOGS_IN_DISK_MENU //allow to show list of catalogs in disk menu
		, ST_ALLOW_ABBREVIATED_SYNTAX_FOR_DEEP_SEARCH //disable possibility to use "\\\" instead of "\*\*\*", "...." instead of "\..\.." and use "cd:a.." instead of "cd:a\.."
		// при добавлении - добавить значение по умолчанию в default_flags_values

		, NUMBER_FLAG_SETTINGS 
	} _tflag;

	class CSettings {
	private:
		CSettings(void);
		~CSettings(void);
	public:
		static CSettings& GetInstance() {
			static CSettings st;
			return st;
		}
		void ReloadSettings();
		void SaveSettings();
	public: //префиксы плагина
		tstring GetPrimaryPluginPrefix();
		tstring const& GetListPrefixes();
	public: //вернуть значение требуемого флага настройки
		DWORD GetValue(tsetting_flags fg) const;	
		tstring const& GetValue(tsetting_strings fg) const;
		tstring const& get_NamedFolders_reg_key() const { return m_nf_reg_key;}
	public: //задать значение требуемого флага настройки (в промежуточном массиве)
		void SetValue(tsetting_flags fg, DWORD Value);	
		void SetValue(tsetting_strings fg, tstring Value);
	private:	//members
		nf::tautobuffer_byte m_FV;	//flag values
		nf::tvector_strings m_SV;	//string values
		tstring m_nf_reg_key;		//ключ в котором хранятся настройки NF
		tstring m_plugin_prefixes;	//префиксы плагина
		tstring m_FullListPrefixes;
	};

	inline const wchar_t *GetMsg(int MsgId) {
		return g_PluginInfo.GetMsg(g_PluginInfo.ModuleNumber, MsgId);
	}
	inline tstring GetRegistryKeyForCommandPatterns() {
		return CSettings::GetInstance().get_NamedFolders_reg_key() + tstring(L"\\CommandPatterns");
	}};
