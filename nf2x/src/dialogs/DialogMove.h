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


class CDialogMove : public dialogT
{
	tstring m_ItemName;
	bool m_bMoveCatalog;	//перемещаем каталог или псевдоним
	bool m_bSeveralItems;
	bool m_bCopyDialog;	//диалог copy, а не move
	tstring m_DestinationCatalog;
public:
	CDialogMove(TCHAR const* ItemName
		, tstring const& DestinationCatalog
		, bool bMoveCatalog
		, bool bSeveralItems
		, bool bCopyDialog);
	~CDialogMove(void);

	UINT ShowModal();
	tstring GetItemName() const {return m_ItemName;	}

	enum {DIALOG_WIDTH = 72, DIALOG_HEIGHT = 8};

	enum {//все элементы диалога		
		ID_DOUBLEBOX
		, ID_TEXT
		, ID_EDIT
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
				m_bCopyDialog
					? m_bMoveCatalog
						? lg::DLG_COPY_CATALOG_TITLE
						: lg::DLG_COPY_SHORTCUT_TITLE						
					: m_bMoveCatalog
						? lg::DLG_MOVE_CATALOG_TITLE
						: lg::DLG_MOVE_SHORTCUT_TITLE						
				, 1, 1, DIALOG_WIDTH-2, DIALOG_HEIGHT-2) 
			<< &far_di_text(ID_TEXT, 
				m_bCopyDialog 
					? m_bSeveralItems	
						? lg::DLG_COPY_ITEMS
						: m_bMoveCatalog
							? lg::DLG_COPY_CATALOG_MOVE_MESSAGE_S
							: lg::DLG_COPY_SHORTCUT_MOVE_MESSAGE_S
					: m_bSeveralItems	
						? lg::DLG_MOVE_ITEMS
						: m_bMoveCatalog
							? lg::DLG_MOVE_CATALOG_MOVE_MESSAGE_S
							: lg::DLG_MOVE_SHORTCUT_MOVE_MESSAGE_S
				, left, 2, left+14)
			<< &far_di_edit(ID_EDIT, left, 3, DIALOG_WIDTH-4, "")
			<< &far_di_singlebox(ID_LINE1, 0, 2, 4, DIALOG_WIDTH-3, 4) 
			<< &far_di_button(ID_OK, lg::MSG_OK, 28, 5, 0, FALSE, FALSE, TRUE)	//default button
			<< &far_di_button(ID_CANCEL, lg::MSG_CANCEL, 37, 5, 0)
			;

		if (! m_bSeveralItems)
		{
			tstring msg = m_DialogItems[ID_TEXT].Data;
			wsprintf(m_DialogItems[ID_TEXT].Data, msg.c_str(), m_ItemName.c_str());
		};
	} //SetDialogItems

};

};