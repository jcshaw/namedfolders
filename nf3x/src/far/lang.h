/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma	once

namespace lg {
enum {
	NAMEDFOLDERS
	,DISKMENUSTRING
	,PLUGINMENUSTRING
	,PLUGINMENU_OPENPANEL
	,PLUGINMENU_APPLYCOMMAND
	,PLUGINMENU_PATTERNS
	,PANEL_TITLE

	,PLUGINSCONFIGSTRING
	,DELETE_SHORTCUT
	,F8
	,F7
	,F6
	,F5

	,CFG_TITLE
	,CFG_PREFIX			//semicolon separated list of NF prefixes
	,CFG_SHOWINPLUGINSMENU
	,CFG_SHOWINDISKMENU
	,CFG_TEMPORARY_AS_HIDDEN
	,CFG_ALLOW_ABBREVIATED_SYNTAX_FOR_DEEP_SEARCH
	,CFG_EXPAND_1SH		//Always expand single-letter shortcuts
	,CFG_NF_HOTKEY		//Hot key to call NF
	,CFG_FLAG_NETWORK_COMMANDS_THROUGH_COMMAND_LINE //net: и \\ через команду строку 
	,CFG_SUBDIRECTORIES_AS_ALIASES	//субдиректории как алиасы

	,CFG_CONFIRMATIONS  //title: confirmations
	,CFG_CONFIRM_OVERRIDE //Confirm overriding shortcut
	,CFG_CONFIRM_DELETE	//Confirm deleting shortcut
	,CFG_CONFIRM_DELETE_NON_EMPTY_CATALOGS  //Confirm deleting catalogs
	,CFG_CONFIRM_IMPLICIT_CREATING //Confirm implicit shortcut creation cd::, cd:+
	,CFG_CONFIRM_IMPLICIT_DELETION //Confirm implicit shortcut deletion cd:-
	,CFG_CONFIRM_GO_TO_NEAREST	//Confirm going to nearest directory

	,CFG_USE_HISTORY_IN_DIALOG_APPLY_COMMAND
	,CFG_USE_SINGLE_MENU_MODE
	,CFG_SHOW_KEYS_IN_MENU //show keys in menu
	,CFG_MASKS_SOFT_EXCEPTIONS 
	//depricated:,CFG_MENU_DISK_FAST_KEY	//клавиша быстрого запуска 
	,CFG_SHOW_CATALOGS_IN_DISK_MENU
	,CFG_TEXT_PANELWIDTH //настройки ширины панели
	,CFG_ASTERIXMODE

	,MSG_OK
	,MSG_CANCEL
	,MSG_ADVANCED
	,MSG_DELETE_SHORTCUT	//Delete shortcut
	,MSG_DELETE_CATALOG	//Delete catalog
	,MSG_DELETE_CATALOG_NOT_EMPTY //Delete not empty catalog
	,MSG_DELETE	//Delete
	,MSG_DELETE_ALL	//Delete
	,MSG_OVERRIDE //Override
	,MSG_OVERRIDE_SHORTCUT //Override shortcut
	,MSG_INSERT_SHORTCUT //Create shortcut
	,MSG_CATALOG	//catalog
	,MSG_ISTEMPORARY //is temporary
	,MSG_PATH_NOT_FOUND //
	,MSG_JUMP_TO_THE_NEAREST_EXISTING_FOLDER //
	,MSG_ERROR	//Error
	,MSG_SKIP
	
	,CONFIRM_DELETE_SHORTCUT //The following shortcut will be deleted
	,CONFIRM_DELETE_SEVERAL_SHORTCUTS //The following shortcut will be deleted
	,CONFIRM_DELETE_CATALOG	//The following catalog will be deleted
	,CONFIRM_OVERRIDE_SHORTCUT	//The following value of shortcut will be overrided
	,CONFIRM_INSERT_SHORTCUT //The following shortcut will be created
	,CONFIRM_DELETE	//The following item will be deleted
	,CONFIRM_INSERT_CATALOG

	,DLG_MOVE_CATALOG_TITLE	//Move catalog to
	,DLG_COPY_CATALOG_TITLE	//Copy catalog to
	,DLG_MOVE_SHORTCUT_TITLE //Move shortcut to
	,DLG_COPY_SHORTCUT_TITLE //Copy shortcut to
	,DLG_MOVE_CATALOG_MOVE_MESSAGE_S //Move "%s" to
	,DLG_COPY_CATALOG_MOVE_MESSAGE_S //Copy "%s" to
	,DLG_MOVE_SHORTCUT_MOVE_MESSAGE_S
	,DLG_COPY_SHORTCUT_MOVE_MESSAGE_S
	,DLG_MOVE_ITEMS
	,DLG_COPY_ITEMS
	,DLG_CREATE_CATALOG_TITLE
	,DLG_MAKE_CATALOG
	
	,DLG_SHORTCUT	//shortcut
	,DLG_ACTIVE_VALUE//Directory on active panel
	,DLG_INACTIVE_VALUE//Directory on inactive panel
	,DGL_EDIT_SHORTCUT_TITLE
	,DGL_INSERT_SHORTCUT_TITLE

	,DLG_APPLY_COMMAND_TITLE
	,DLG_APPLY_COMMAND_TEXT

	,STATE_UNKNOWN
	,STATE_NO
	,STATE_OK
	,STATE_PARTLY_OK
	,STATE_NOT_FOUND

	,NF_PANEL_NAME
	,NF_PANEL_DESCRIPTION
	,NF_PANEL_STATE

	,MSG_SAVE_SETUP_MESSAGE
//command patterns
	,CP_PATTERNS_DIALOG_TITLE
	,DLG_CREATE_COMMAND_PATTERN
	,DLG_EDIT_COMMAND_PATTERN
	,DLG_TEXT_PREFIX
	,DLG_TEXT_PATTERN
};

}


