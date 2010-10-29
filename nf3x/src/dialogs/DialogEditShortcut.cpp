/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#include "StdAfx.h"
#include "dialogeditshortcut.h"
#include "Kernel.h"
#include "strings_utils.h"

using namespace nf;
CDialogEditShortcut::CDialogEditShortcut(nf::tshortcut_info const &sh
										 , tstring const &ValueActivePanel
										 , tstring const &ValueInActivePanel
										 , bool bInsertMode
										 )
	: dialogT(DIALOG_WIDTH, DIALOG_HEIGHT, g_PluginInfo, 12, L"DialogEditShortcut")
	, m_sh(sh)
	, m_ValueActivePanel(ValueActivePanel)
	, m_ValueInActivePanel(ValueInActivePanel)
	, m_bInsertMode(bInsertMode)
{
	SetDialogItems();
}

CDialogEditShortcut::~CDialogEditShortcut(void)
{
}

UINT CDialogEditShortcut::ShowModal()
{
	//вызываем диалог редактированик имени псевдонима
	int nChoosedItem;

	GetDialogItemsRef().SetFarDialogItemData(ID_EDIT_CATALOG, m_sh.catalog.c_str());
	GetDialogItemsRef().SetFarDialogItemData(ID_EDIT_SHORTCUT, m_sh.shortcut.c_str());
	GetDialogItemsRef().SetFarDialogItemData(ID_EDIT_VALUE_ACTIVE, m_ValueActivePanel.c_str());
	GetDialogItemsRef().SetFarDialogItemData(ID_EDIT_VALUE_INACTIVE, m_ValueInActivePanel.c_str());
	GetDialogItemsRef()[ID_SHOW_ISTEMPORARY].Selected = m_sh.bIsTemporary ? 1 : 0;

	GetDialogItemsRef()[ID_EDIT_SHORTCUT].Focus = 1;

	if (Execute(nChoosedItem))
		if (nChoosedItem != ID_CANCEL)
		{	//редактируем имя псевдонима
			m_sh.catalog = this->GetDialogItemValue(ID_EDIT_CATALOG); 
			m_sh.shortcut = this->GetDialogItemValue(ID_EDIT_SHORTCUT);
			m_ValueActivePanel = this->GetDialogItemValue(ID_EDIT_VALUE_ACTIVE);
			m_ValueInActivePanel = this->GetDialogItemValue(ID_EDIT_VALUE_INACTIVE);
			m_sh.bIsTemporary = this->IsDialogItemSelected(ID_SHOW_ISTEMPORARY);

			return 1;
		}

		return 0;
}
