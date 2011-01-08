/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#include "StdAfx.h"
#include "dialogeditcatalog.h"
#include "Kernel.h"
#include "strings_utils.h"

using namespace nf;
CDialogEditCatalog::CDialogEditCatalog(wchar_t const* ParentCatalog
									   , wchar_t const* CatalogName
									   , bool bCreateNew)
	: dialogT(DIALOG_WIDTH, DIALOG_HEIGHT, g_PluginInfo, 10, L"DialogEditCatalog"),
	m_bCreateNew(bCreateNew), m_ParentCatalog(ParentCatalog), m_CatalogName(CatalogName)
{
	SetDialogItems();
}

CDialogEditCatalog::~CDialogEditCatalog(void)
{
}


UINT CDialogEditCatalog::ShowModal() {
	//call edit dialog of catalog name
	int nChoosedItem;
	
	GetDialogItemsRef().SetFarDialogItemData(ID_EDIT_CATALOG, m_CatalogName.c_str());
	GetDialogItemsRef()[ID_EDIT_CATALOG].Focus = 1;

	if (Execute(nChoosedItem))
		if (nChoosedItem != ID_CANCEL) {
			m_CatalogName = this->GetDialogItemValue(ID_EDIT_CATALOG);

			return 1;
		}

	return 0;
}

tstring CDialogEditCatalog::GetCatalogName(bool bIncludeParent) const
{
	if (! bIncludeParent) return m_CatalogName;
	else return m_ParentCatalog + tstring(SLASH_CATS) + m_CatalogName;
}