/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/

#include "StdAfx.h"
#include "dialogeditcatalog.h"
#include "Kernel.h"
#include "strings_utils.h"

using namespace nf;
CDialogEditCatalog::CDialogEditCatalog(TCHAR const* ParentCatalog
									   , TCHAR const* CatalogName
									   , bool bCreateNew)
	: dialogT(DIALOG_WIDTH, DIALOG_HEIGHT, g_PluginInfo, 10, _T("DialogEditCatalog")),
	m_bCreateNew(bCreateNew), m_ParentCatalog(ParentCatalog), m_CatalogName(CatalogName)
{
	SetDialogItems();
}

CDialogEditCatalog::~CDialogEditCatalog(void)
{
}


UINT CDialogEditCatalog::ShowModal()
{
	//вызываем диалог редактированик имени каталога
	int nChoosedItem;
	
	::lstrcpy(m_DialogItems[ID_EDIT_CATALOG].Data, Utils::GetInternalToOem(m_CatalogName).c_str());
	m_DialogItems[ID_EDIT_CATALOG].Focus = 1;

	if (Execute(nChoosedItem))
		if (nChoosedItem != ID_CANCEL)
		{	//редактируем имя каталога
			m_CatalogName = Utils::GetOemToInternal(m_DialogItems[ID_EDIT_CATALOG].Data);
			return 1;
		}

	return 0;
}

tstring CDialogEditCatalog::GetCatalogName(bool bIncludeParent) const
{
	if (! bIncludeParent) return m_CatalogName;
	else return m_ParentCatalog + tstring(SLASH_CATS) + m_CatalogName;
}