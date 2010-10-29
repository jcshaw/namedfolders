/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

#include "dialogsT.h"
#include "lang.h"

namespace nf 
{

extern const char* GetMsg(int MsgId);


class CDialogEditCatalog : public dialogT
{
	tstring m_ParentCatalog;
	tstring m_CatalogName;
	bool m_bCreateNew;
public:
	CDialogEditCatalog(TCHAR const* ParentCatalog, 
		TCHAR const* CatalogName, 
		bool bCreateNew);
	~CDialogEditCatalog(void);

	UINT ShowModal();
	tstring GetCatalogName(bool bIncludeParent) const;

	enum {DIALOG_WIDTH = 72, DIALOG_HEIGHT = 8};

	enum {//все элементы диалога		
		ID_DOUBLEBOX
		, ID_TEXT_CATALOG
		, ID_EDIT_CATALOG
		, ID_LINE1
		, ID_OK
		, ID_CANCEL
	};

	virtual void SetDialogItems()
	{
		//перечисляем все требуемые элементы диалога
		const int left = 3;
		m_DialogItems
			<< &far_di_doublebox(ID_DOUBLEBOX,
				m_bCreateNew
					? lg::DLG_MAKE_CATALOG
					: lg::DLG_MOVE_CATALOG_TITLE
					, 1, 1, DIALOG_WIDTH-2, DIALOG_HEIGHT-2) 
			<< &far_di_text(ID_TEXT_CATALOG, 
				m_bCreateNew
					? lg::DLG_CREATE_CATALOG_TITLE
					: lg::DLG_MOVE_CATALOG_MOVE_MESSAGE_S
				, left, 2, left+14)
			<< &far_di_edit(ID_EDIT_CATALOG, left, 3, DIALOG_WIDTH-4, "")
			<< &far_di_singlebox(ID_LINE1, 0, 2, 4, DIALOG_WIDTH-3, 4) 
			<< &far_di_button(ID_OK, lg::MSG_OK, 28, 5, 0, FALSE, FALSE, TRUE)	//default button
			<< &far_di_button(ID_CANCEL, lg::MSG_CANCEL, 37, 5, 0)
			;

		tstring msg = m_DialogItems[ID_TEXT_CATALOG].Data;
		wsprintf(m_DialogItems[ID_TEXT_CATALOG].Data, msg.c_str(), m_CatalogName.c_str());
	};

};

};