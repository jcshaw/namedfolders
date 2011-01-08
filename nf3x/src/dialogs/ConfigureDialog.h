/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

#include "dialogsT.h"
#include "lang.h"
#include "settings.h"

//NamedFolders Configure Dialog
// list of plugin settings

namespace nf {
namespace Private {
	enum {//все элементы диалога		
		ID_DOUBLEBOX
		, ID_SHOW_INDISK_MENU
		, ID_SHOW_INPLUGINS_MENU

		//depricated:, ID_TEXT_MENU_DISK_FAST_KEY
		//depricated:, ID_EDIT_MENU_DISK_FAST_KEY
		, ID_TEXT_SHOW_CATALOGS_IN_DISK_MENU

		, ID_LINE1
		, ID_CONFIRM_OVERRIDE
		, ID_CONFIRM_GO_TO_NEAREST
		, ID_CONFIRM_DELETE
		, ID_CONFIRM_DELETE_CATALOGS
		, ID_CONFIRM_IMPLICIT_CREATING
		, ID_CONFIRM_IMPLICIT_DELETION
		, ID_LINE2	
		, ID_ALWAYS_EXPAND_SHORTCUTS

		, ID_TEXT_ASTERIXMODE
		, ID_EDIT_ASTERIXMODE

		, ID_HISTORY_IN_APPLY_COMMAND_DIALOG
		, ID_USE_SINGLE_MENU_MODE
		, ID_FLAG_NETWORK_COMMANDS_THROUGH_COMMAND_LINE
		, ID_SUBDIRECTORIES_AS_ALIASES
		, ID_TEMPORARY_AS_HIDDEN
		, ID_ALLOW_ABBREVIATED_SYNTAX_FOR_DEEP_SEARCH

		, ID_TEXT_PREFIX
		, ID_EDIT_PREFIX	
		, ID_TEXT_PANELWIDTH
		, ID_EDIT_PANELWIDTH

		, ID_LINE3
		, ID_LINE4
		, ID_TEXT_MASKS_SOFT_EXCEPTIONS
		, ID_EDIT_MASKS_SOFT_EXCEPTIONS

		, ID_OK
		, ID_CANCEL
	};
} //Private

class CConfigureDialog : public dialogT {
	nf::tautobuffer_byte m_dwParams;
	nf::tvector_strings m_strParams;
	FarList m_FarList;

public:
	CConfigureDialog(void);
	~CConfigureDialog(void);

	UINT ShowModal();

	enum {DIALOG_WIDTH = 80, DIALOG_HEIGHT = 21};

	virtual void SetDialogItems()
	{
		using namespace nf::Private;
	//перечисляем все требуемые элементы диалога
		const int left = 3;
		const int right = DIALOG_WIDTH-4;
		const int left_half = DIALOG_WIDTH / 2;
		GetDialogItemsRef()
//			<< &far_di_button(ID_ADVANCED, lg::MSG_ADVANCED, 35, 13)
			<< &far_di_button(ID_CANCEL, lg::MSG_CANCEL, 35, 18, 0)
			<< &far_di_button(ID_OK, lg::MSG_OK, 25, 18, 0, FALSE, FALSE, TRUE) //default button

			<< &far_di_edit(ID_EDIT_MASKS_SOFT_EXCEPTIONS, left_half, 16, right-1, L"")
			<< &far_di_text(ID_TEXT_MASKS_SOFT_EXCEPTIONS, lg::CFG_MASKS_SOFT_EXCEPTIONS, left, 16, left_half)		

			<< &far_di_box(DI_SINGLEBOX, ID_LINE4, 0, left, 15, right, 15) 

			<< &far_di_edit(ID_EDIT_PANELWIDTH, left_half+18, 14, right-1, L"")
			<< &far_di_text(ID_TEXT_PANELWIDTH, lg::CFG_TEXT_PANELWIDTH, left_half, 14, left_half+16)

			<< &far_di_edit(ID_EDIT_PREFIX, left+20, 14, left_half-2, L"")
			<< &far_di_text(ID_TEXT_PREFIX, lg::CFG_PREFIX, left, 14, left+20)
			<< &far_di_box(DI_SINGLEBOX, ID_LINE3, 0, left, 13, right, 13) 

			<< &far_di_checkbox(ID_ALLOW_ABBREVIATED_SYNTAX_FOR_DEEP_SEARCH, lg::CFG_ALLOW_ABBREVIATED_SYNTAX_FOR_DEEP_SEARCH, left_half, 12)
			<< &far_di_checkbox(ID_TEMPORARY_AS_HIDDEN, lg::CFG_TEMPORARY_AS_HIDDEN, left, 12)

			<< &far_di_checkbox(ID_SUBDIRECTORIES_AS_ALIASES, lg::CFG_SUBDIRECTORIES_AS_ALIASES, left_half, 11)
			<< &far_di_checkbox(ID_FLAG_NETWORK_COMMANDS_THROUGH_COMMAND_LINE, lg::CFG_FLAG_NETWORK_COMMANDS_THROUGH_COMMAND_LINE, left, 11)
			<< &far_di_checkbox(ID_HISTORY_IN_APPLY_COMMAND_DIALOG, lg::CFG_USE_HISTORY_IN_DIALOG_APPLY_COMMAND, left, 10)
			<< &far_di_checkbox(ID_USE_SINGLE_MENU_MODE, lg::CFG_USE_SINGLE_MENU_MODE, left_half, 10)

			<< &far_di_combobox(ID_EDIT_ASTERIXMODE, left_half+25, 9, right-1, &m_FarList)
			<< &far_di_text(ID_TEXT_ASTERIXMODE, lg::CFG_ASTERIXMODE, left_half, 9, left_half+25)
// 
// 			<< &far_di_edit(ID_EDIT_ASTERIXMODE, left_half+25, 9, right-1, L"")
// 			<< &far_di_text(ID_TEXT_ASTERIXMODE, lg::CFG_ASTERIXMODE, left_half, 9, left_half+25)

			<< &far_di_checkbox(ID_ALWAYS_EXPAND_SHORTCUTS, lg::CFG_EXPAND_1SH, left, 9)

			<< &far_di_box(DI_SINGLEBOX, ID_LINE2, 0, left, 8, right, 8)
			<< &far_di_checkbox(ID_CONFIRM_IMPLICIT_DELETION, lg::CFG_CONFIRM_IMPLICIT_DELETION, left_half, 7)
			<< &far_di_checkbox(ID_CONFIRM_IMPLICIT_CREATING, lg::CFG_CONFIRM_IMPLICIT_CREATING, left_half, 6)
			<< &far_di_checkbox(ID_CONFIRM_DELETE_CATALOGS, lg::CFG_CONFIRM_DELETE_NON_EMPTY_CATALOGS, left_half, 5)
			<< &far_di_checkbox(ID_CONFIRM_DELETE, lg::CFG_CONFIRM_DELETE, left, 7)
			<< &far_di_checkbox(ID_CONFIRM_GO_TO_NEAREST, lg::CFG_CONFIRM_GO_TO_NEAREST, left, 6)
			<< &far_di_checkbox(ID_CONFIRM_OVERRIDE, lg::CFG_CONFIRM_OVERRIDE, left, 5)
			<< &far_di_box(DI_SINGLEBOX, ID_LINE1, lg::CFG_CONFIRMATIONS, left, 4, right, 4) 
			
			<< &far_di_checkbox(ID_TEXT_SHOW_CATALOGS_IN_DISK_MENU, lg::CFG_SHOW_CATALOGS_IN_DISK_MENU, left, 3)

			<< &far_di_checkbox(ID_SHOW_INPLUGINS_MENU, lg::CFG_SHOWINPLUGINSMENU, left_half, 2)
			<< &far_di_checkbox(ID_SHOW_INDISK_MENU, lg::CFG_SHOWINDISKMENU, left, 2) 
			<< &far_di_box(DI_DOUBLEBOX, ID_DOUBLEBOX, lg::CFG_TITLE, left-1, 1, right+1, DIALOG_HEIGHT-2) 
		;
	};
private:
	void dde_registry(bool bSaveToRegistry);
	void dde_main_dialog(bool bSaveToControls);

};

}; //nf