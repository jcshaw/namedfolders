/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#include "StdAfx.h"
#include "DialogApplyCommand.h"
#include "settings.h"

#include "Kernel.h"

using namespace nf;

CDialogApplyCommand::CDialogApplyCommand(void)
: dialogT(nf::NF_DIALOG_APPLY_COMMAND, DIALOG_WIDTH, DIALOG_HEIGHT, g_PluginInfo, 10, L"DialogExecuteCommand")
{
	SetDialogItems();
}

CDialogApplyCommand::~CDialogApplyCommand(void)
{
}

UINT CDialogApplyCommand::ShowModal() {
	intptr_t nChoosedItem;

	if (Execute(nChoosedItem)) {
		if (nChoosedItem != ID_CANCEL) {	
			m_Command =  this->GetDialogItemValue(ID_EDIT);
			return 1;
		}
	}
	return 0;
}


bool CDialogApplyCommand::IsUseHistory() const
{
	return (CSettings::GetInstance().GetValue(nf::ST_HISTORY_IN_DIALOG_APPLY_COMMAND)) != 0;
}