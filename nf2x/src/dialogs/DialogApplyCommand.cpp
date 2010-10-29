/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/

#include "StdAfx.h"
#include "DialogApplyCommand.h"
#include "settings.h"

#include "Kernel.h"

using namespace nf;

CDialogApplyCommand::CDialogApplyCommand(void)
: dialogT(DIALOG_WIDTH, DIALOG_HEIGHT, g_PluginInfo, 10, _T("DialogExecuteCommand"))
{
	SetDialogItems();
}

CDialogApplyCommand::~CDialogApplyCommand(void)
{
}

UINT CDialogApplyCommand::ShowModal()
{
	//вызываем диалог редактированик имени каталога
	int nChoosedItem;

	if (Execute(nChoosedItem))
		if (nChoosedItem != ID_CANCEL)
		{	//редактируем имя каталога
			m_Command = m_DialogItems[ID_EDIT].Data;
			return 1;
		}

	return 0;
}


bool CDialogApplyCommand::IsUseHistory() const
{
	return (CSettings::GetInstance().GetValue(nf::ST_HISTORY_IN_DIALOG_APPLY_COMMAND)) != 0;
}