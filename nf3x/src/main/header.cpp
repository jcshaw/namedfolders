/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/


#include "stdafx.h"
#include "header.h"


//определения констант
wchar_t const* PREFIX_CD = L"cd:";
wchar_t const* PREFIX_ST = L"st:";

wchar_t const * SLASH_CATS = L"/";		//разделитель каталогов
wchar_t const * SLASH_DIRS = L"\\";	//разделитель директорий
wchar_t const SLASH_CATS_CHAR = L'/';	//разделитель каталогов
wchar_t const SLASH_DIRS_CHAR = L'\\';	//разделитель директорий
wchar_t const*const MC_DEEP_DIRECT_SEARCH_SHORT = L"\n";
wchar_t const*const MC_DEEP_REVERSE_SEARCH_SHORT = L"\t";

wchar_t const* COMMAND_PARAM_ADD_WIN7_LIBRARY = L"+lib";
wchar_t const* COMMAND_PARAM_DELETE_WIN7_LIBRARY = L"-lib";


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

/// GUID плагина Named Folders (требуется в FAR3)
/// {C6A35DB8-6846-449E-A1DC-2FC152ED4B0B}
const GUID nf::NF_PLUGIN_GUID = { 0xc6a35db8, 0x6846, 0x449e, { 0xa1, 0xdc, 0x2f, 0xc1, 0x52, 0xed, 0x4b, 0xb } };
/// {B90A8402-14F7-4382-9275-C79824DD8A24}
const GUID nf::NF_DISKMENU_GUID = { 0xb90a8402, 0x14f7, 0x4382, { 0x92, 0x75, 0xc7, 0x98, 0x24, 0xdd, 0x8a, 0x24 } };

// {FE4E3C90-3590-4F76-9D05-D30FA3338F9E}
const GUID nf::NF_PLUGINSMENU_GUID = { 0xfe4e3c90, 0x3590, 0x4f76, { 0x9d, 0x5, 0xd3, 0xf, 0xa3, 0x33, 0x8f, 0x9e } };

// {F30CBBA9-397A-4BF9-81FB-3154B8FBCD90}
const GUID nf::NF_CONFIGMENU_GUID = { 0xf30cbba9, 0x397a, 0x4bf9, { 0x81, 0xfb, 0x31, 0x54, 0xb8, 0xfb, 0xcd, 0x90 } };

// {F5EA3E3D-A578-4C1B-B1EA-D6CCB33175CF}
const GUID nf::NF_DIALOG_CONFIGURE = { 0xf5ea3e3d, 0xa578, 0x4c1b, { 0xb1, 0xea, 0xd6, 0xcc, 0xb3, 0x31, 0x75, 0xcf } };

// {38730031-17D8-4985-BFA6-DCD3613C2364}
const GUID nf::NF_DIALOG_APPLY_COMMAND = { 0x38730031, 0x17d8, 0x4985, { 0xbf, 0xa6, 0xdc, 0xd3, 0x61, 0x3c, 0x23, 0x64 } };

// {A5B778D5-A0A0-41C2-9F7B-350EB9BE6969}
const GUID nf::NF_DIALOG_EDIT_CATALOG = { 0xa5b778d5, 0xa0a0, 0x41c2, { 0x9f, 0x7b, 0x35, 0xe, 0xb9, 0xbe, 0x69, 0x69 } };

// {47D727D5-9699-4A4B-89D0-5809E426806E}
const GUID nf::NF_DIALOG_EDIT_PATTERN = { 0x47d727d5, 0x9699, 0x4a4b, { 0x89, 0xd0, 0x58, 0x9, 0xe4, 0x26, 0x80, 0x6e } };

// {4E3B3296-9B39-406A-864C-1723CF309C4D}
const GUID nf::NF_DIALOG_EDIT_SHORTCUT = { 0x4e3b3296, 0x9b39, 0x406a, { 0x86, 0x4c, 0x17, 0x23, 0xcf, 0x30, 0x9c, 0x4d } };

// {813CFDA1-711D-4760-99B0-1ED7D6C7D0C0}
const GUID nf::NF_DIALOG_MOVE = { 0x813cfda1, 0x711d, 0x4760, { 0x99, 0xb0, 0x1e, 0xd7, 0xd6, 0xc7, 0xd0, 0xc0 } };

// {A23F828E-FC4D-46B9-BEBD-6C9CC8F9435D}
const GUID nf::NF_MESSAGE_SAFE_PANEL_STATE = { 0xa23f828e, 0xfc4d, 0x46b9, { 0xbe, 0xbd, 0x6c, 0x9c, 0xc8, 0xf9, 0x43, 0x5d } };
