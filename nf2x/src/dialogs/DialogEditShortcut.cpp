/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
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
	: dialogT(DIALOG_WIDTH, DIALOG_HEIGHT, g_PluginInfo, 12, _T("DialogEditShortcut"))
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

	::lstrcpy(m_DialogItems[ID_EDIT_CATALOG].Data, Utils::GetInternalToOem(m_sh.catalog).c_str());
	::lstrcpy(m_DialogItems[ID_EDIT_SHORTCUT].Data, Utils::GetInternalToOem(m_sh.shortcut).c_str());
	::lstrcpy(m_DialogItems[ID_EDIT_VALUE_ACTIVE].Data, Utils::GetInternalToOem(m_ValueActivePanel).c_str());
	::lstrcpy(m_DialogItems[ID_EDIT_VALUE_INACTIVE].Data, Utils::GetInternalToOem(m_ValueInActivePanel).c_str());
	m_DialogItems[ID_SHOW_ISTEMPORARY].Selected = m_sh.bIsTemporary ? 1 : 0;

	m_DialogItems[ID_EDIT_SHORTCUT].Focus = 1;

	if (Execute(nChoosedItem))
		if (nChoosedItem != ID_CANCEL)
		{	//редактируем имя псевдонима
			m_sh.catalog = Utils::GetOemToInternal(m_DialogItems[ID_EDIT_CATALOG].Data);
			m_sh.shortcut = Utils::GetOemToInternal(m_DialogItems[ID_EDIT_SHORTCUT].Data);
			m_ValueActivePanel = Utils::GetOemToInternal(m_DialogItems[ID_EDIT_VALUE_ACTIVE].Data);
			m_ValueInActivePanel = Utils::GetOemToInternal(m_DialogItems[ID_EDIT_VALUE_INACTIVE].Data);
			m_sh.bIsTemporary = m_DialogItems[ID_SHOW_ISTEMPORARY].Selected != 0;

			return 1;
		}

		return 0;
}
