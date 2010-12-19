/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/


#include "stdafx.h"
#include "header.h"


//определения констант

wchar_t const * SLASH_CATS = L"/";		//разделитель каталогов
wchar_t const * SLASH_DIRS = L"\\";	//разделитель директорий
wchar_t const SLASH_CATS_CHAR = L'/';	//разделитель каталогов
wchar_t const SLASH_DIRS_CHAR = L'\\';	//разделитель директорий
wchar_t const*const MC_DEEP_DIRECT_SEARCH_SHORT = L"\n";
wchar_t const*const MC_DEEP_REVERSE_SEARCH_SHORT = L"\t";

wchar_t const*const MC_SEARCH_BACKWORD_SHORT = L"\b"; //символ, обозначающий переход на уровень выше = '..'
wchar_t const*const MC_SEARCH_BACKWORD_SHORT_WITHOUT_SLASH = L"\a"; //символ, обозначающий переход на уровень выше = '..', когда используется "..", а не "\.."
wchar_t const*const MC_SEARCH_FORWARD_SHORT = L"\r"; // \*\* is equal to \\

wchar_t const*const MC_DEEP_DIRECT_SEARCH_LONG = L"\\**"; //see also Utils::SubstituteSearchMetachars - "**" and "..*" are specified there
wchar_t const*const MC_DEEP_REVERSE_SEARCH_LONG = L"\\..*";
wchar_t const*const MC_SEARCH_BACKWORD_LONG =  L"\\..";
wchar_t const*const MC_SEARCH_FORWARD_LONG = L"\\*";

wchar_t const* LEVEL_UP_TWO_POINTS = L"..";

const wchar_t CHAR_LEADING_VALUE_ENVVAR = L'%';	//!TODO: remove it
const wchar_t CHAR_LEADING_VALUE_REGKEY = L'$';	//!TODO: remove it


wchar_t const* HISTORY_APPLY_COMMANDS = L"nf_history_apply_commands";
wchar_t const* HISTORY_COMMAND_PATTERNS = L"nf_history_command_patterns";

