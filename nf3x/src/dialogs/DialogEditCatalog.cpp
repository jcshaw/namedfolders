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
#include "catalog_utils.h"

using namespace nf;
CDialogEditCatalog::CDialogEditCatalog(wchar_t const* parentCatalog
									   , wchar_t const* catalogName
									   , bool bCreateNew)
: dialogT(DIALOG_WIDTH, DIALOG_HEIGHT, g_PluginInfo, 10, L"DialogEditCatalog")
, m_bCreateNew(bCreateNew)
, m_ParentCatalog(parentCatalog)
, m_CatalogName(catalogName)
{
	SetDialogItems();
	if (m_bCreateNew) {
		nf::sc::GetDefaultCatalogProperties(m_Properties);
	} else {
		nf::sc::LoadCatalogProperties(GetCatalogName(true), m_Properties);
	}
}

CDialogEditCatalog::~CDialogEditCatalog(void)
{
}


UINT CDialogEditCatalog::ShowModal() {	
	GetDialogItemsRef().SetFarDialogItemData(ID_EDIT_CATALOG, m_CatalogName.c_str());
	GetDialogItemsRef()[ID_SHOW_IN_DISKMENU].Selected = m_Properties.show_in_disk_menu ? 1 : 0;
	GetDialogItemsRef()[ID_EDIT_CATALOG].Focus = 1;

	int nChoosedItem;
	if (Execute(nChoosedItem))
		if (nChoosedItem != ID_CANCEL) {
			m_Properties.show_in_disk_menu = this->IsDialogItemSelected(ID_SHOW_IN_DISKMENU);
			nf::sc::SaveCatalogProperties(GetCatalogName(true), m_Properties);

			m_CatalogName = this->GetDialogItemValue(ID_EDIT_CATALOG);
			return 1;
		}

	return 0;
}
