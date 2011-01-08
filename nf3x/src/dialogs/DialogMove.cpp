/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#include "StdAfx.h"
#include "dialogmove.h"
#include "Kernel.h"
#include "strings_utils.h"

#include "stlcatalogs.h"

//!TODO: find appropriate place for this function
bool IsNamedFoldersCatalogExists(tstring const& catalogName) {
	return nf::sc::CCatalog::IsCatalogExist(catalogName);
}


using namespace nf;
CDialogMove::CDialogMove(wchar_t const* ItemName
						 , tstring const& DestinationCatalog
						 , bool bMoveCatalog
						 , bool bSeveralItems
						 , bool bCopyDialog)
: dialogT(DIALOG_WIDTH, DIALOG_HEIGHT, g_PluginInfo, 10, L"DialogMove")
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
	
	GetDialogItemsRef().SetFarDialogItemData(ID_EDIT, m_DestinationCatalog.c_str());
	GetDialogItemsRef()[ID_EDIT].Focus = 1;

	if (Execute(nChoosedItem))
		if (nChoosedItem != ID_CANCEL)
		{	//редактируем имя каталога
			m_ItemName = this->GetDialogItemValue(ID_EDIT);
			return 1;
		}

	return 0;
}

