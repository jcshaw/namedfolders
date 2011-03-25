/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

#include "dialogsT.h"
#include "lang.h"
#include "strings_utils.h"

namespace nf {
extern const wchar_t* GetMsg(int MsgId);

//this dialog is used to create/edit catalog
class CDialogEditCatalog : public dialogT {
	tstring m_ParentCatalog;
	tstring m_CatalogName;
	bool const m_bCreateNew; //false - edit
	tcatalog_properties m_Properties;
public:
	CDialogEditCatalog(wchar_t const* ParentCatalog, wchar_t const* CatalogName, bool bCreateNew);
	~CDialogEditCatalog(void);

	UINT ShowModal();
	tstring GetCatalogName(bool bIncludeParent) const {
		return ! bIncludeParent 
			? m_CatalogName
			: Utils::CombinePath(m_ParentCatalog, m_CatalogName, SLASH_CATS);
	}
	tcatalog_properties const& GetCatalogProperties() const {
		return m_Properties;
	}

	enum {DIALOG_WIDTH = 72, DIALOG_HEIGHT = 9};

	enum {//all dialog items
		ID_DOUBLEBOX
		, ID_TEXT_CATALOG
		, ID_EDIT_CATALOG
		, ID_SHOW_IN_DISKMENU
		, ID_LINE1
		, ID_OK
		, ID_CANCEL
	};

	virtual void SetDialogItems() {
		const int left = 3;
		this->GetDialogItemsRef()
			<< &far_di_box(DI_SINGLEBOX, ID_DOUBLEBOX,
				m_bCreateNew
					? lg::DLG_MAKE_CATALOG
					: lg::DLG_EDIT_CATALOG
					, 1, 1, DIALOG_WIDTH-2, DIALOG_HEIGHT-2) 
			<< &far_di_text(ID_TEXT_CATALOG, lg::DLG_CREATE_CATALOG_TITLE, left, 2, DIALOG_WIDTH-3)
			<< &far_di_edit(ID_EDIT_CATALOG, left, 3, DIALOG_WIDTH-4, L"")
			<< &far_di_checkbox(ID_SHOW_IN_DISKMENU, lg::MSG_SHOW_IN_DISKMENU, left, 4)
			<< &far_di_box(DI_SINGLEBOX, ID_LINE1, 0, 2, 5, DIALOG_WIDTH-3, 5) 
			<< &far_di_button(ID_OK, lg::MSG_OK, 28, 6, 0, FALSE, FALSE, TRUE)	//default button
			<< &far_di_button(ID_CANCEL, lg::MSG_CANCEL, 37, 6, 0)
		;
	};
};

}