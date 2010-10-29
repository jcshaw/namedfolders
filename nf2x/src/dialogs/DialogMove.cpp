/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/

#include "StdAfx.h"
#include "dialogmove.h"
#include "Kernel.h"
#include "strings_utils.h"


using namespace nf;
CDialogMove::CDialogMove(TCHAR const* ItemName
						 , tstring const& DestinationCatalog
						 , bool bMoveCatalog
						 , bool bSeveralItems
						 , bool bCopyDialog)
: dialogT(DIALOG_WIDTH, DIALOG_HEIGHT, g_PluginInfo, 10, _T("DialogMove"))
, m_ItemName(ItemName)
, m_bMoveCatalog(bMoveCatalog)
, m_bSeveralItems(bSeveralItems)
, m_bCopyDialog(bCopyDialog)
, m_DestinationCatalog(DestinationCatalog)
{
	SetDialogItems();
}

CDialogMove::~CDialogMove(void)
{
}


UINT CDialogMove::ShowModal()
{
	//вызываем диалог редактированик имени каталога
	int nChoosedItem;
	
	::lstrcpy(m_DialogItems[ID_EDIT].Data, Utils::GetInternalToOem(m_DestinationCatalog + SLASH_CATS).c_str());
	m_DialogItems[ID_EDIT].Focus = 1;

	if (Execute(nChoosedItem))
		if (nChoosedItem != ID_CANCEL)
		{	//редактируем имя каталога
			m_ItemName = Utils::GetOemToInternal(m_DialogItems[ID_EDIT].Data);
			return 1;
		}

	return 0;
}

