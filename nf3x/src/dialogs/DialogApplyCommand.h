/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

#include "dialogsT.h"
#include "lang.h"

namespace nf {

extern const wchar_t* GetMsg(int MsgId);

class CDialogApplyCommand : public dialogT {
	tstring m_Command;
public:
	CDialogApplyCommand(void);
	~CDialogApplyCommand(void);

	enum {DIALOG_WIDTH = 72, DIALOG_HEIGHT = 8};

	enum {//все элементы диалога		
		ID_DOUBLEBOX
		, ID_TEXT
		, ID_EDIT
		, ID_LINE1
		, ID_OK
		, ID_CANCEL
	};

	UINT ShowModal();
	tstring const& GetCommandText() const {return m_Command;}
	bool IsUseHistory() const;

	virtual void SetDialogItems()
	{
		//перечисляем все требуемые элементы диалога
		const int left = 3;
		GetDialogItemsRef()
			<< &far_di_box(DI_SINGLEBOX, ID_DOUBLEBOX
				, lg::DLG_APPLY_COMMAND_TITLE
				, 1, 1, DIALOG_WIDTH-2, DIALOG_HEIGHT-2) 
			<< &far_di_text(ID_TEXT
				, lg::DLG_APPLY_COMMAND_TEXT
				, left, 2, DIALOG_WIDTH-2)
			<< &far_di_edit(ID_EDIT, left, 3, DIALOG_WIDTH-4, L""
				, DIF_HISTORY
				, 1
				, IsUseHistory()
					? HISTORY_APPLY_COMMANDS
					: 0
			)
			<< &far_di_box(DI_SINGLEBOX, ID_LINE1, 0, 2, 4, DIALOG_WIDTH-3, 4) 
			<< &far_di_button(ID_OK, lg::MSG_OK, 28, 5, 0, FALSE, FALSE, TRUE)	//default button
			<< &far_di_button(ID_CANCEL, lg::MSG_CANCEL, 37, 5, 0)
			;
	}; //SetDialogItems
}; //CDialogApplyCommand

} //nf