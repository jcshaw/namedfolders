/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#include "StdAfx.h"
#include "configuredialog.h"
#include "Kernel.h"
#include <cmath>
#include "strings_utils.h"

using namespace nf;
using namespace nf::Private;

namespace
{
	struct {
		int control_id;
		tsetting_flags settings_id;
	} main_dw_settings[] = {
		{ID_SHOW_INDISK_MENU, nf::ST_SHOW_IN_DISK_MENU}
		,{ID_SHOW_INPLUGINS_MENU, nf::ST_SHOW_IN_PLUGINS_MENU}
		,{ID_CONFIRM_OVERRIDE, nf::ST_CONFIRM_OVERRIDE}
		,{ID_CONFIRM_DELETE, nf::ST_CONFIRM_DELETE}
		,{ID_CONFIRM_DELETE_CATALOGS, nf::ST_CONFIRM_DELETE_NOT_EMPTY_CATALOGS}
		,{ID_CONFIRM_IMPLICIT_CREATING, nf::ST_CONFIRM_IMPLICIT_CREATION}
		,{ID_CONFIRM_IMPLICIT_DELETION, nf::ST_CONFIRM_IMPLICIT_DELETION}
		,{ID_CONFIRM_GO_TO_NEAREST, nf::ST_CONFIRM_GO_TO_NEAREST}
		,{ID_ALWAYS_EXPAND_SHORTCUTS, nf::ST_ALWAYS_EXPAND_SHORTCUTS}
		,{ID_FLAG_NETWORK_COMMANDS_THROUGH_COMMAND_LINE, nf::ST_FLAG_NETWORK_COMMANDS_THROUGH_COMMAND_LINE}
		,{ID_HISTORY_IN_APPLY_COMMAND_DIALOG, nf::ST_HISTORY_IN_DIALOG_APPLY_COMMAND}
		,{ID_TEMPORARY_AS_HIDDEN, nf::ST_SHOW_TEMPORARY_AS_HIDDEN}
		//depricated: ,{ID_EDIT_MENU_DISK_FAST_KEY, nf::ST_EDIT_MENU_DISK_FAST_KEY}
		,{ID_USE_SINGLE_MENU_MODE, nf::ST_USE_SINGLE_MENU_MODE}
		,{ID_SUBDIRECTORIES_AS_ALIASES, nf::ST_SUBDIRECTORIES_AS_ALIASES}
		,{ID_TEXT_SHOW_CATALOGS_IN_DISK_MENU, nf::ST_SHOW_CATALOGS_IN_DISK_MENU}
		,{ID_ALLOW_ABBREVIATED_SYNTAX_FOR_DEEP_SEARCH, nf::ST_ALLOW_ABBREVIATED_SYNTAX_FOR_DEEP_SEARCH}
	};
	struct {
		int control_id;
		tsetting_strings settings_id;
	} main_str_settings[] = 
	{
		{ID_EDIT_PREFIX, nf::STS_PREFIXES}
		,{ID_EDIT_PANELWIDTH, nf::STS_PANELWIDTH}
		,{ID_EDIT_MASKS_SOFT_EXCEPTIONS, nf::ST_SOFT_MASKS_TO_IGNORE_COMMA_SEPARETED}
		,{ID_EDIT_ASTERIXMODE, nf::ST_ASTERIX_MODE} //this is combobox now
	};
	const unsigned int NUM_MAIN_DW_SETTINGS = sizeof(main_dw_settings) / sizeof(main_dw_settings[0]);
	const unsigned int NUM_MAIN_STR_SETTINGS = sizeof(main_str_settings) / sizeof(main_str_settings[0]);
}

CConfigureDialog::CConfigureDialog() 
	: dialogT(DIALOG_WIDTH, DIALOG_HEIGHT, g_PluginInfo, 10, L"Settings")
	, m_dwParams(max(nf::NUMBER_FLAG_SETTINGS, NUM_MAIN_DW_SETTINGS))
	, m_strParams(max(nf::NUMBER_STRING_SETTINGS, NUM_MAIN_STR_SETTINGS))
{
	static wchar_t const* ITEM_TEXTS[] = {
		L"0: a->*a*"
		, L"1: a->a*"
		, L"2: a->a"
	};

	static FarListItem g_Items[sizeof(ITEM_TEXTS) / sizeof(wchar_t const*)];
	unsigned int count_items = sizeof(ITEM_TEXTS) / sizeof(wchar_t const*);

	m_FarList.ItemsNumber = count_items;
	m_FarList.Items = &g_Items[0]; //по какой то не€сной причине динамические массивы тут не проход€т... выравнивание???

	for (unsigned int i = 0; i < count_items; ++ i) {
		memset(&m_FarList.Items[i].Reserved, 0, sizeof(m_FarList.Items[i].Reserved));
		m_FarList.Items[i].Text = ITEM_TEXTS[i];
		m_FarList.Items[i].Flags = 0;
	}

	//загружаем текущие настройки - они нужны, чтобы определить текущий Asterix mode и инициализировать комбобокс
	CSettings::GetInstance().ReloadSettings();
	dde_registry(false);

	for (unsigned int i = 0; i < NUM_MAIN_STR_SETTINGS; ++i) {
		if (ID_EDIT_ASTERIXMODE == main_str_settings[i].control_id) {
			unsigned int cur_asterix_mode = Utils::atoi(m_strParams[main_str_settings[i].settings_id]);
			if (cur_asterix_mode < count_items) {
				m_FarList.Items[cur_asterix_mode].Flags = LIF_SELECTED;
			}
		}
	}
	SetDialogItems();
}

CConfigureDialog::~CConfigureDialog()
{
	//delete[] m_FarList.Items;
}

UINT CConfigureDialog::ShowModal()
{
	dde_main_dialog(true);

//вызываем диалог настроек
	int nChoosedItem;
	while (true) {
		if (! Execute(nChoosedItem)) break;
		switch (nChoosedItem) {
		case ID_CANCEL: 
			return 0;
		case ID_OK: 
		{	//записываем настройки
			dde_main_dialog(false);
			dde_registry(true);
			CSettings::GetInstance().SaveSettings();
			return 0;
		} break;
		default: assert(false);
		}
	} //while

	return 0;
}

void CConfigureDialog::dde_registry(bool bSaveToRegistry)
{
	if (bSaveToRegistry) {
		for (int i = 0; i < NUM_MAIN_DW_SETTINGS; ++i) {
			CSettings::GetInstance().SetValue(main_dw_settings[i].settings_id, m_dwParams[main_dw_settings[i].settings_id]);
		}
		for (int i = 0; i < NUM_MAIN_STR_SETTINGS; ++i) {
			CSettings::GetInstance().SetValue(main_str_settings[i].settings_id, m_strParams[main_str_settings[i].settings_id]); //!TODO: OEM ?
		}
	} else {
		for (int i = 0; i < NUM_MAIN_DW_SETTINGS; ++i) {
			m_dwParams[main_dw_settings[i].settings_id] = static_cast<BYTE>(CSettings::GetInstance().GetValue(main_dw_settings[i].settings_id));
		}
		for (int i = 0; i < NUM_MAIN_STR_SETTINGS; ++i) {
			m_strParams[main_str_settings[i].settings_id] = CSettings::GetInstance().GetValue(main_str_settings[i].settings_id);
		}
	}
}

void CConfigureDialog::dde_main_dialog(bool bSaveToControls)
{
	if (bSaveToControls) {
		//считываем настройки из векторов в элементы диалога
		for (int i = 0; i < NUM_MAIN_DW_SETTINGS; ++i) {
			GetDialogItemsRef()[main_dw_settings[i].control_id].Selected = m_dwParams[main_dw_settings[i].settings_id];
		}
		for (int i = 0; i < NUM_MAIN_STR_SETTINGS; ++i) {
			if (ID_EDIT_ASTERIXMODE != main_str_settings[i].control_id) {
				GetDialogItemsRef().SetFarDialogItemData(main_str_settings[i].control_id, m_strParams[main_str_settings[i].settings_id].c_str());
			} else {
				//nothing to do
			}
		}

	} else 	{	//сохран€ем настройки из контролов в вектора
		for (int i = 0; i < NUM_MAIN_DW_SETTINGS; ++i) {
			m_dwParams[main_dw_settings[i].settings_id] = this->IsDialogItemSelected(main_dw_settings[i].control_id);
		}
		for (int i = 0; i < NUM_MAIN_STR_SETTINGS; ++i) {
			if (ID_EDIT_ASTERIXMODE != main_str_settings[i].control_id) {
				m_strParams[main_str_settings[i].settings_id] = this->GetDialogItemValue(main_str_settings[i].control_id);
			} else {
				int cur_asterix_mode = static_cast<int>(g_PluginInfo.SendDlgMessage(GetDialogHandle(), DM_LISTGETCURPOS, ID_EDIT_ASTERIXMODE, 0));
				m_strParams[main_str_settings[i].settings_id] = Utils::itoa(cur_asterix_mode);
			}
		}
	}
}
