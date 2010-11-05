/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include "dialogsT.h"
#include "lang.h"
#include "strings_utils.h"

namespace nf {
extern const wchar_t* GetMsg(int MsgId);

class CDialogMove : public dialogT {
public:
	CDialogMove(wchar_t const* ItemName
		, tstring const& DestinationCatalog
		, bool bMoveCatalog
		, bool bSeveralItems
		, bool bCopyDialog);
	~CDialogMove(void);

	UINT ShowModal();
	tstring GetItemName() const {return m_ItemName;	}

	enum {DIALOG_WIDTH = 72
		, DIALOG_HEIGHT = 8};

	enum {//all dialog items
		ID_DOUBLEBOX
		, ID_TEXT
		, ID_EDIT
		, ID_LINE1
		, ID_OK
		, ID_CANCEL
	};

	virtual void SetDialogItems() {
		const int left = 3;
		GetDialogItemsRef()
			<< &far_di_box(DI_SINGLEBOX, ID_DOUBLEBOX,
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
				, left, 2, DIALOG_WIDTH-4)
			<< &far_di_edit(ID_EDIT, left, 3, DIALOG_WIDTH-4, L"")
			<< &far_di_box(DI_SINGLEBOX, ID_LINE1, 0, 2, 4, DIALOG_WIDTH-3, 4) 
			<< &far_di_button(ID_OK, lg::MSG_OK, 28, 5, 0, FALSE, FALSE, TRUE)	//default button
			<< &far_di_button(ID_CANCEL, lg::MSG_CANCEL, 37, 5, 0)
			;

		if (! m_bSeveralItems) {
			GetDialogItemsRef().SetFarDialogItemData(ID_TEXT
				, Utils::SubstituteSubstringLikePrintf(GetDialogItemsRef()[ID_TEXT].PtrData, m_ItemName).c_str()
				);
		};
	} //SetDialogItems
private:
	tstring m_ItemName;
	bool m_bMoveCatalog;	//move catalog or shortcut
	bool m_bSeveralItems;
	bool m_bCopyDialog;		//This is "copy" dialog, not "move" dialog
	tstring m_DestinationCatalog;
};

}