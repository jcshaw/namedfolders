/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

#include "dialogsT.h"
#include "lang.h"
#include "codec_for_values.h"

namespace nf {

class CDialogEditShortcut : public dialogT

{
	nf::tshortcut_info m_sh;
	tstring m_ValueActivePanel;
	tstring m_ValueInActivePanel;
	bool m_bInsertMode;
public:
	CDialogEditShortcut(nf::tshortcut_info const &sh
		, tstring const &ValueActivePanel
		, tstring const &ValueInActivePanel
		, bool bInsertMode
	); 
	~CDialogEditShortcut(void);

	UINT ShowModal();
	nf::tshortcut_info const & GetShortcutInfo() const { return m_sh; }
	tstring const & GetValueForActivePanel() const {return m_ValueActivePanel; }
	tstring const & GetValueForInActivePanel() const {return m_ValueInActivePanel; }

	enum {DIALOG_WIDTH = 52, DIALOG_HEIGHT = 15};

	enum {//все элементы диалога		
		ID_DOUBLEBOX
		, ID_TEXT_CATALOG
		, ID_EDIT_CATALOG
		, ID_TEXT_SHORTCUT
		, ID_EDIT_SHORTCUT
		, ID_TEXT_VALUE_ACTIVE
		, ID_EDIT_VALUE_ACTIVE
		, ID_TEXT_VALUE_INACTIVE
		, ID_EDIT_VALUE_INACTIVE
		, ID_SHOW_ISTEMPORARY
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
				m_bInsertMode 
					? lg::DGL_INSERT_SHORTCUT_TITLE
					: lg::DGL_EDIT_SHORTCUT_TITLE
					, 1, 1, DIALOG_WIDTH-2, DIALOG_HEIGHT-2) 
			<< &far_di_text(ID_TEXT_CATALOG, lg::MSG_CATALOG, left, 2, left+15)
			<< &far_di_edit(ID_EDIT_CATALOG, left, 3, DIALOG_WIDTH-4, "")
			<< &far_di_text(ID_TEXT_SHORTCUT, lg::DLG_SHORTCUT, left, 4, left+15)
			<< &far_di_edit(ID_EDIT_SHORTCUT, left, 5, DIALOG_WIDTH-4, "")
			<< &far_di_text(ID_TEXT_VALUE_ACTIVE, lg::DLG_ACTIVE_VALUE, left, 6, left+15)
			<< &far_di_edit(ID_EDIT_VALUE_ACTIVE, left, 7, DIALOG_WIDTH-4, "")
			<< &far_di_text(ID_TEXT_VALUE_INACTIVE, lg::DLG_INACTIVE_VALUE, left, 8, left+15)
			<< &far_di_edit(ID_EDIT_VALUE_INACTIVE, left, 9, DIALOG_WIDTH-4, "")
			<< &far_di_checkbox(ID_SHOW_ISTEMPORARY, lg::MSG_ISTEMPORARY, left, 10)
			<< &far_di_singlebox(ID_LINE1, 0, 2, 11, DIALOG_WIDTH-3, 11) 
			<< &far_di_button(ID_OK, lg::MSG_OK, 12, 12, 0, FALSE, FALSE, TRUE)	//default button
			<< &far_di_button(ID_CANCEL, lg::MSG_CANCEL, 26, 12, 0)
			;
	};

};

};