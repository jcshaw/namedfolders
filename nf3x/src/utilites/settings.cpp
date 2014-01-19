/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "settings.h"
#include <cassert>
#include "Kernel.h"
#include "CommandPatterns.h"
#include "FarSettingsKeyWrapper.h"

using namespace nf;

namespace 
{
	typedef struct tdefault_string_value {
		tsetting_strings flag;
		wchar_t const* default_value;
		wchar_t const *regkey;
	} _tdefault_string_value;

	tdefault_string_value const default_strings_values [] = {
		{STS_PREFIXES, L"cd:st:", L"prefix"}
		, {ST_SOFT_MASKS_TO_IGNORE_COMMA_SEPARETED, L"*readme*,*uninstall*,*read me*", L"ST_SOFT_MASKS_TO_IGNORE_COMMA_SEPARETED"}
		, {STS_PANELWIDTH, L"N,Z,C0;10,0,5", L"STS_PANELWIDTH"}
		, {ST_ASTERIX_MODE, L"0", L"ST_ASTERIX_MODE"}
	};

	typedef struct tdefault_flag_value {
		tsetting_flags flag;
		BYTE default_value;	//ограничиваемся byte'ом, чтобы исключить лишний vector<DWORD>, и приминить уже используемый vector<BYTE>
		wchar_t const *keyName;
	} _default_flag_value;

	tdefault_flag_value const default_flags_values [] = {
		{ST_SHOW_ERRORS, 0, L"ShowErrors"}
		, {ST_SHOW_IN_PLUGINS_MENU, 1, L"ShowInPluginsMenu"}
		, {ST_SHOW_IN_DISK_MENU, 1, L"ShowInDiskMenu"}
		, {ST_SHOW_TEMPORARY_AS_HIDDEN, 1, L"ShowTemporaryNamesAsHideFiles"}
		, {ST_CONFIRM_DELETE, 1, L"CONFIRM_DELETE"}
		, {ST_CONFIRM_DELETE_CATALOGS, 1, L"CONFIRM_DELETE_CATALOGS"}
		, {ST_CONFIRM_DELETE_NOT_EMPTY_CATALOGS, 1, L"CONFIRM_DELETE_NOT_EMPTY_CATALOGS"}
		, {ST_CONFIRM_OVERRIDE, 1, L"CONFIRM_OVERRIDE"}
		, {ST_CONFIRM_IMPLICIT_CREATION, 1, L"CONFIRM_IMPLICIT_CREATION"}
		, {ST_CONFIRM_IMPLICIT_DELETION, 1, L"CONFIRM_IMPLICIT_DELETION"}
		, {ST_CONFIRM_GO_TO_NEAREST, 1, L"CONFIRM_GO_TO_NEAREST"}
		, {ST_SHOW_KEYS_IN_MENU, 1, 	L"CONFIRM_KEYS_IN_MENU"}
		, {ST_SELECT_SH_MENU_MODE, 1, L"SH_MENU_MODE"}
		, {ST_PANEL_MODE, 6, L"PANEL_MODE"}
		, {ST_SELECT_CATS_MENU_MODE, 1, L"CATS_MENU_MODE"}
		, {ST_ALWAYS_EXPAND_SHORTCUTS, 1, L"ALWAYS_EXPAND_SHORTCUTS"}
		, {ST_HISTORY_IN_DIALOG_APPLY_COMMAND, 1, L"USE_HISTORY_IN_DIALOG_APPLY_COMMAND"}
		, {ST_SELECT_SH_MENU_MODE_EV, 1, L"SH_MENU_MODE_EV"}
		, {ST_SELECT_SH_MENU_SHOWCATALOGS_MODE, 1, L"ST_SELECT_SH_MENU_SHOWCATALOGS_MODE"}
		//depricated: , {ST_EDIT_MENU_DISK_FAST_KEY, 0, L"ST_EDIT_MENU_DISK_FAST_KEY"}
		, {ST_FLAG_NETWORK_COMMANDS_THROUGH_COMMAND_LINE, 0, L"ST_FLAG_NETWORK_COMMANDS_THROUGH_COMMAND_LINE"}
		, {ST_SELECT_SOFT_MENU_SHOWCATALOGS_MODE, 0, L"ST_SELECT_SOFT_MENU_SHOWCATALOGS_MODE"}
		, {ST_USE_SINGLE_MENU_MODE, 0, L"ST_USE_SINGLE_MENU_MODE"}
		, {ST_SUBDIRECTORIES_AS_ALIASES, 0, L"ST_SUBDIRECTORIES_AS_ALIASES"}
		, {ST_SHOW_CATALOGS_IN_DISK_MENU, 0, L"ST_SHOW_CATALOGS_IN_DISK_MENU"}
		, {ST_ALLOW_ABBREVIATED_SYNTAX_FOR_DEEP_SEARCH, 1, L"ST_DISABLE_ABBREVIATED_SYNTAX_FOR_DEEP_SEARCH"}

		, {ST_SORT_SH_MENU_COLUMN, 0, L"ST_SORT_SH_MENU_COLUMN"}	//колонка, по которой сортировать меню вариантов с шоткатами
		, {ST_SORT_SOFT_MENU_COLUMN, 0, L"ST_SORT_SOFT_MENU_COLUMN"}   //колонка, по которой сортировать меню вариантов с программами
		, {ST_SORT_SH_MENU_EV_COLUMN, 0, L"ST_SORT_SH_MENU_EV_COLUMN"}  //колонка, по которой сортировать меню вариантов с переменными среды

	};

	const int NUMBER_FLAGS = 
		sizeof(default_flags_values) / sizeof(tdefault_flag_value);
	
	const int NUMBER_STRINGS = 
		sizeof(default_strings_values) / sizeof(tdefault_string_value);

}

CSettings::CSettings()
: m_FV(NUMBER_FLAGS)
, m_SV(NUMBER_STRINGS)
{
	assert(NUMBER_STRINGS == NUMBER_STRING_SETTINGS);
	assert(NUMBER_FLAGS == NUMBER_FLAG_SETTINGS);

	m_nf_reg_key = L"Software\\Far3\\Plugins"; //!TODO: заменить на Settings API
	m_nf_reg_key += L"\\NamedFolders";
	ReloadSettings();
}

CSettings::~CSettings(void)
{
	
}

tstring CSettings::GetPrimaryPluginPrefix() {
	tstring prefix = CSettings::GetInstance().GetValue(nf::STS_PREFIXES);
	tstring::size_type npos = prefix.find_first_of(L":");
	if (npos != tstring::npos) prefix.erase(npos + 1, prefix.size() - npos + 1);
	return prefix;
}


tstring const& CSettings::GetListPrefixes() {
	m_FullListPrefixes = CSettings::GetInstance().GetValue(nf::STS_PREFIXES).c_str();
	nf::Patterns::CommandsManager cm;
	m_FullListPrefixes += cm.GetListCommandPrefixes();
// 	if (tstring::npos == m_FullListPrefixes.find(L"st:")) { //!TODO: we need possibility to turn of a support of "st:" 
// 		m_FullListPrefixes += L"st:";
// 	}
	return m_FullListPrefixes;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

namespace {
	wchar_t const* ROOT_NF_SETTINGS_KEY = L"nf_settings";
	boost::shared_ptr<FarSettingsKeyWrapper> get_root_key() {
		boost::shared_ptr<FarSettingsKeyWrapper> dest(new FarSettingsKeyWrapper(ROOT_NF_SETTINGS_KEY, true));
		return dest;
// 		auto h = nf::PluginSettings::FarOpenKey(0, ROOT_NF_SETTINGS_KEY);
// 		if (nf::PluginSettings::isInvalidHandle(h)) {
// 			h = nf::PluginSettings::FarCreateKey(0, ROOT_NF_SETTINGS_KEY);
// 		}
// 		return h;
	}
}

///загрузить настройки из реестра
void CSettings::ReloadSettings() {	
	boost::shared_ptr<FarSettingsKeyWrapper> fc = get_root_key();
	for (int i = 0; i < NUMBER_FLAGS; ++i) {
		tdefault_flag_value const &d = default_flags_values[i];
	//получаем значение из реестра
		__int64 value;
		if (fc->getValue(d.keyName, value)) {
			m_FV[d.flag] = static_cast<BYTE>(value);
		} else {
			m_FV[d.flag] = d.default_value;	//присваиваем значение по-умолчанию
		}
	}
	for (int i = 0; i < NUMBER_STRINGS; ++i) {
		tdefault_string_value const &d = default_strings_values[i];
		tstring value;
		if (fc->getValue(d.regkey, value)) {
			m_SV[d.flag] = value;
		} else {
			m_SV[d.flag] = d.default_value;	
		}
	}

}

void CSettings::SaveSettings() {
	boost::shared_ptr<FarSettingsKeyWrapper> fc = get_root_key();

	for (int i = 0; i < NUMBER_FLAGS; ++i) {
		tdefault_flag_value const &d = default_flags_values[i];
		fc->setValue(d.keyName, m_FV[d.flag]);
	}
	for (int i = 0; i < NUMBER_STRINGS; ++i) {
		tdefault_string_value const &d = default_strings_values[i];
		fc->setValue(d.regkey, m_SV[d.flag].c_str());
	}
}

DWORD CSettings::GetValue(tsetting_flags fg) const
{	//вернуть значение требуемого флага настройки
	assert(static_cast<UINT>(fg) < m_FV.size());
	return m_FV[static_cast<DWORD>(fg)];
}

void CSettings::SetValue(tsetting_flags fg, DWORD Value) 
{	//задать значение требуемого флага настройки (в промежуточном массиве)
	assert(static_cast<UINT>(fg) < m_FV.size());
	m_FV[static_cast<UINT>(fg)] = static_cast<BYTE>(Value);
}

tstring const& CSettings::GetValue(tsetting_strings fg) const
{	//вернуть значение требуемого флага настройки
	assert(static_cast<UINT>(fg) < m_SV.size());
	return m_SV[static_cast<DWORD>(fg)];
}

void CSettings::SetValue(tsetting_strings fg, tstring Value) 
{	//задать значение требуемого флага настройки (в промежуточном массиве)
	assert(static_cast<UINT>(fg) < m_SV.size());
	m_SV[static_cast<UINT>(fg)] = Value;
}

