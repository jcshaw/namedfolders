/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "settings.h"
#include "Registry.h"
#include <cassert>
#include "Kernel.h"
#include "CommandPatterns.h"

using namespace nf;

namespace 
{
	typedef struct tdefault_string_value
	{
		tsetting_strings flag;
		TCHAR const* default_value;
		TCHAR const *regkey;
	} _tdefault_string_value;

	tdefault_string_value const default_strings_values [] = 
	{
		{STS_PREFIXES, _T("cd:cc:"), _T("prefix")}
		,{ST_SOFT_MASKS_TO_IGNORE_COMMA_SEPARETED, _T("*readme*,*uninstall*,*read me*"), _T("ST_SOFT_MASKS_TO_IGNORE_COMMA_SEPARETED")}
		,{STS_PANELWIDTH, _T("N,Z,C0;10,0,5"), _T("STS_PANELWIDTH")}
		,{ST_ASTERIX_MODE, _T("0"), _T("ST_ASTERIX_MODE")}
	};

	typedef struct tdefault_flag_value
	{
		tsetting_flags flag;
		BYTE default_value;	//ограничиваемся byte'ом, чтобы исключить лишний vector<DWORD>, и приминить уже используемый vector<BYTE>
		TCHAR const *regkey;
	} _default_flag_value;

	tdefault_flag_value const default_flags_values [] = 
	{
		{ST_SHOW_ERRORS, 0, _T("ShowErrors")}
		,{ST_SHOW_IN_PLUGINS_MENU, 1, _T("ShowInPluginsMenu")}
		,{ST_SHOW_IN_DISK_MENU, 1, _T("ShowInDiskMenu")}
		,{ST_SHOW_TEMPORARY_AS_HIDDEN, 1, _T("ShowTemporaryNamesAsHideFiles")}
		,{ST_CONFIRM_DELETE, 1, _T("CONFIRM_DELETE")}
		,{ST_CONFIRM_DELETE_CATALOGS, 1, _T("CONFIRM_DELETE_CATALOGS")}
		,{ST_CONFIRM_DELETE_NOT_EMPTY_CATALOGS, 1, _T("CONFIRM_DELETE_NOT_EMPTY_CATALOGS")}
		,{ST_CONFIRM_OVERRIDE, 1, _T("CONFIRM_OVERRIDE")}
		,{ST_CONFIRM_IMPLICIT_CREATION, 1, _T("CONFIRM_IMPLICIT_CREATION")}
		,{ST_CONFIRM_IMPLICIT_DELETION, 1, _T("CONFIRM_IMPLICIT_DELETION")}
		,{ST_CONFIRM_GO_TO_NEAREST, 1, _T("CONFIRM_GO_TO_NEAREST")}
		,{ST_SHOW_KEYS_IN_MENU, 1, 	_T("CONFIRM_KEYS_IN_MENU")}
		,{ST_SELECT_SH_MENU_MODE, 1, _T("SH_MENU_MODE")}
		,{ST_PANEL_MODE, 6, _T("PANEL_MODE")}
		,{ST_SELECT_CATS_MENU_MODE, 1, _T("CATS_MENU_MODE")}
		,{ST_ALWAYS_EXPAND_SHORTCUTS, 1, _T("ALWAYS_EXPAND_SHORTCUTS")}
		,{ST_HISTORY_IN_DIALOG_APPLY_COMMAND, 1, _T("USE_HISTORY_IN_DIALOG_APPLY_COMMAND")}
		,{ST_SELECT_SH_MENU_MODE_EV, 1, _T("SH_MENU_MODE_EV")}
		,{ST_SELECT_SH_MENU_SHOWCATALOGS_MODE, 1, _T("ST_SELECT_SH_MENU_SHOWCATALOGS_MODE")}
		,{ST_EDIT_MENU_DISK_FAST_KEY, 0, _T("ST_EDIT_MENU_DISK_FAST_KEY")}
		,{ST_FLAG_NETWORK_COMMANDS_THROUGH_COMMAND_LINE, 0, _T("ST_FLAG_NETWORK_COMMANDS_THROUGH_COMMAND_LINE")}
		,{ST_SELECT_SOFT_MENU_SHOWCATALOGS_MODE, 0, _T("ST_SELECT_SOFT_MENU_SHOWCATALOGS_MODE")}
		,{ST_USE_SINGLE_MENU_MODE, 0, _T("ST_USE_SINGLE_MENU_MODE")}
		,{ST_SUBDIRECTORIES_AS_ALIASES, 0, _T("ST_SUBDIRECTORIES_AS_ALIASES")}
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

	assert(g_PluginInfo.RootKey);
	m_nf_reg_key = tstring(g_PluginInfo.RootKey);
	m_nf_reg_key += _T("\\NamedFolders");
	ReloadSettings();
}

CSettings::~CSettings(void)
{
	
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
tstring CSettings::GetPrimaryPluginPrefix()
{
	tstring prefix = CSettings::GetInstance().GetValue(nf::STS_PREFIXES);
	tstring::size_type npos = prefix.find_first_of(_T(":"));
	if (npos != tstring::npos) prefix.erase(npos + 1, prefix.size() - npos + 1);
	return prefix;
}


tstring const& CSettings::GetListPrefixes() 
{
	m_FullListPrefixes = CSettings::GetInstance().GetValue(nf::STS_PREFIXES).c_str();
	nf::Patterns::CommandsManager cm(nf::GetRegistryKeyForCommandPatterns());
	m_FullListPrefixes += cm.GetListCommandPrefixes();
	return m_FullListPrefixes;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void CSettings::ReloadSettings()
{	//загрузить настройки из реестра
	CRegistry reg(HKEY_CURRENT_USER, m_nf_reg_key.c_str());
	for (int i = 0; i < NUMBER_FLAGS; ++i)
	{
		tdefault_flag_value const &d = default_flags_values[i];
	//получаем значение из реестра
		DWORD value;
		if (reg.GetValue(d.regkey, value)) m_FV[d.flag] = static_cast<BYTE>(value);
		else m_FV[d.flag] = d.default_value;	//присваиваем значение по-умолчанию
	}
	for (int i = 0; i < NUMBER_STRINGS; ++i)
	{
		tdefault_string_value const &d = default_strings_values[i];
		tstring value;
		if (reg.GetValue(d.regkey, value)) 
			m_SV[d.flag] = value;
		else 
			m_SV[d.flag] = d.default_value;	
	}

}

void CSettings::SaveSettings()
{
	CRegistry reg(HKEY_CURRENT_USER, m_nf_reg_key.c_str());
	for (int i = 0; i < NUMBER_FLAGS; ++i)
	{
		tdefault_flag_value const &d = default_flags_values[i];
		reg.SetValue(d.regkey, m_FV[d.flag]);
	}
	for (int i = 0; i < NUMBER_STRINGS; ++i)
	{
		tdefault_string_value const &d = default_strings_values[i];
		reg.SetValue(d.regkey, m_SV[d.flag].c_str());
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

