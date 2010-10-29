/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/


#include "stdafx.h"
#include "header.h"


//определения констант

TCHAR const * SLASH_CATS = _T("/");		//разделитель каталогов
TCHAR const * SLASH_DIRS = _T("\\");	//разделитель директорий
TCHAR const SLASH_CATS_CHAR = _T('/');	//разделитель каталогов
TCHAR const SLASH_DIRS_CHAR = _T('\\');	//разделитель директорий
TCHAR const*const DEEP_DIRECT_SEARCH = _T("\n");
TCHAR const*const DEEP_REVERSE_SEARCH = _T("\t");
TCHAR const*const DEEP_UP_DIRECTORY = _T("\b"); //символ, обозначающий переход на уровень выше = '..'


TCHAR const* LEVEL_UP_TWO_POINTS = _T("..");

const TCHAR CHAR_LEADING_VALUE_ENVVAR = _T('%');	//!TODO: remove it
const TCHAR CHAR_LEADING_VALUE_REGKEY = _T('$');	//!TODO: remove it


TCHAR const* HISTORY_APPLY_COMMANDS = _T("nf_history_apply_commands");
TCHAR const* HISTORY_COMMAND_PATTERNS = _T("nf_history_command_patterns");
