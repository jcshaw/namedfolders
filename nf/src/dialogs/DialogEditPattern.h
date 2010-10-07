#pragma once

#include "dialogsT.h"
#include "lang.h"

namespace nf
{

extern const wchar_t* GetMsg(int MsgId);

namespace Patterns
{

class DialogEditPattern	: public dialogT
{
	bool m_bCreateNewPattern;
	tstring m_Prefix;
	tstring m_Pattern;
public:
	DialogEditPattern();
	DialogEditPattern(tstring const& Prefix, tstring const& Pattern);
	~DialogEditPattern(void);

	UINT ShowModal();
	tstring const& GetResultPrefix() const {return m_Prefix;}
	tstring const& GetResultPattern() const {return m_Pattern;}

	enum {DIALOG_WIDTH = 72, DIALOG_HEIGHT = 10};

	enum {	//all dialog items
		ID_DOUBLEBOX
		, ID_TEXT_PREFIX
		, ID_EDIT_PREFIX
		, ID_TEXT_PATTERN
		, ID_EDIT_PATTERN
		, ID_LINE2
		, ID_OK
		, ID_CANCEL
	};

	virtual void SetDialogItems()
	{
		const int left = 3;
		GetDialogItemsRef()
			<< &far_di_doublebox(ID_DOUBLEBOX,
				m_bCreateNewPattern
					? lg::DLG_CREATE_COMMAND_PATTERN
					: lg::DLG_EDIT_COMMAND_PATTERN
				, 1, 1, DIALOG_WIDTH-2, DIALOG_HEIGHT-2) 
			<< &far_di_text(ID_TEXT_PREFIX, lg::DLG_TEXT_PREFIX, left, 2, left+14)
			<< &far_di_edit(ID_EDIT_PREFIX, left, 3, DIALOG_WIDTH-4, L"")
			<< &far_di_text(ID_TEXT_PATTERN, lg::DLG_TEXT_PATTERN, left, 4, left+14)
			<< &far_di_edit(ID_EDIT_PATTERN, left, 5, DIALOG_WIDTH-4, L""
				, DIF_HISTORY
				, 1
				, HISTORY_COMMAND_PATTERNS
			)
			<< &far_di_singlebox(ID_LINE2, 0, 2, 6, DIALOG_WIDTH-left, 6) 
			<< &far_di_button(ID_OK, lg::MSG_OK, 28, 7, 0, FALSE, FALSE, TRUE)	//default button
			<< &far_di_button(ID_CANCEL, lg::MSG_CANCEL, 37, 7, 0)
			;
	};
};


} //Patterns

} //nf