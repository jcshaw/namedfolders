/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#include "StdAfx.h"
#include "dialogeditcatalog.h"

#include <boost/foreach.hpp>

#include "Kernel.h"
#include "strings_utils.h"
#include "catalog_utils.h"
#include "win7_libraries.h"

using namespace nf;
CDialogEditCatalog::CDialogEditCatalog(wchar_t const* parentCatalog
									   , wchar_t const* catalogName
									   , bool bCreateNew)
: dialogT(nf::NF_DIALOG_EDIT_CATALOG, DIALOG_WIDTH, DIALOG_HEIGHT, g_PluginInfo, 10, L"DialogEditCatalog")
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

	nf::Win7LibrariesManager m;
	m_Buffers.reserve(m_ListLibraries.size());
	m.GetListLibraries(m_ListLibraries);
	BOOST_FOREACH(tpair_strings const& kvp, m_ListLibraries) {
		nf::tstring_buffer pbuffer(new nf::tautobuffer_char(kvp.second.size() + 1));
		lstrcpy(&(*pbuffer)[0], kvp.second.c_str());
		m_Buffers.push_back(pbuffer);
	}
	
	m_FarListItems = new FarListItem[m_Buffers.size()];

	m_FarList.ItemsNumber = m_Buffers.size();
	m_FarList.Items = &m_FarListItems[0]; 	
	
	unsigned int i = 0;
	BOOST_FOREACH(nf::tstring_buffer const& lib, m_Buffers) {
		memset(&m_FarList.Items[i], 0, sizeof(FarListItem));
		m_FarList.Items[i].Text = &(*lib)[0];
		m_FarList.Items[i].Flags = 0;
		++i;
	}

}

CDialogEditCatalog::~CDialogEditCatalog(void)
{
	delete[] m_FarListItems;
}


UINT CDialogEditCatalog::ShowModal() {	
	GetDialogItemsRef().SetFarDialogItemData(ID_EDIT_CATALOG, m_CatalogName.c_str());
	GetDialogItemsRef()[ID_SHOW_IN_DISKMENU].Selected = m_Properties.show_in_disk_menu ? 1 : 0;
	GetDialogItemsRef()[ID_ADD_TO_WIN7_LIBRARY_BY_DEFAULT].Selected = m_Properties.flag_add_to_win7_lib_by_default ? 1 : 0;
	//GetDialogItemsRef()[ID_EDIT_CATALOG].Focus = 1; //!TODO
	GetDialogItemsRef().SetFarDialogItemData(ID_EDIT_WIN7_LIBRARY_NAME, m_Properties.default_win7_lib.c_str());
	int nChoosedItem;
	if (Execute(nChoosedItem))
		if (nChoosedItem != ID_CANCEL) {
			m_Properties.show_in_disk_menu = this->IsDialogItemSelected(ID_SHOW_IN_DISKMENU);
			m_Properties.flag_add_to_win7_lib_by_default = this->IsDialogItemSelected(ID_ADD_TO_WIN7_LIBRARY_BY_DEFAULT);
			m_Properties.default_win7_lib = this->GetDialogItemValue(ID_EDIT_WIN7_LIBRARY_NAME);

			nf::sc::SaveCatalogProperties(GetCatalogName(true), m_Properties);

			m_CatalogName = this->GetDialogItemValue(ID_EDIT_CATALOG);
			return 1;
		}

	return 0;
}
