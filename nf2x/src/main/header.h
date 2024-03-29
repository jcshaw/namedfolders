/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include <tchar.h>
#include <string>
#include <vector>

#pragma warning(disable: 4244 4267)
//������������ ���������� BOOST
//http://www.boost.org
#define BOOST_REGEX_STATIC_LINK
#include <boost/regex.hpp>
#pragma warning(default: 4244 4267)

#include <stlsoft/memory/auto_buffer.hpp>

//��������� ������ � ������� � OEM ���������
//��������: ������ ��������� ������ ��������� � ���� Ansi
//Far �������� � OEM ����������
//������� ������ ��� �� OEM

typedef std::basic_string<TCHAR> tstring;
typedef tstring tstring_oem;

//typedef std::basic_string<char> tstring;

extern TCHAR const * SLASH_CATS;		//����������� ���������
extern TCHAR const * SLASH_DIRS;		//����������� ����������
extern TCHAR const SLASH_CATS_CHAR;		//����������� ���������
extern TCHAR const SLASH_DIRS_CHAR;		//����������� ����������
extern TCHAR const* LEVEL_UP_TWO_POINTS;

//����������� ��� ������ � �������������� ������� ����������� - ����� � ����
extern TCHAR const*const DEEP_DIRECT_SEARCH;
extern TCHAR const*const DEEP_REVERSE_SEARCH;
extern TCHAR const*const DEEP_UP_DIRECTORY;


extern const TCHAR CHAR_LEADING_VALUE_ENVVAR;	//������ ��� ������������� ���������� ����� � �����
extern const TCHAR CHAR_LEADING_VALUE_REGKEY;	//������ ��� ������������� ������ ������� � ����� 

extern TCHAR const* HISTORY_APPLY_COMMANDS;
extern TCHAR const* HISTORY_COMMAND_PATTERNS;


extern struct PluginStartupInfo g_PluginInfo; 
extern struct FarStandardFunctions g_FSF;


enum {
	DR_CANCEL = 0
	, DR_DELETE = 1
	, DR_DELETE_AND_DELETEALL = 2
	, DR_SKIP = 3
};

namespace nf
{
#ifndef UNICODE
	typedef boost::regex tregex;
	typedef boost::cmatch tcmatch;
	typedef boost::smatch tsmatch;
	typedef boost::sregex_iterator tregex_iterator;
	typedef boost::sregex_token_iterator tregex_token_iterator;
#else
	typedef boost::wregex tregex;
	typedef boost::wcmatch tcmatch;
	typedef boost::wsmatch tsmatch;
	typedef boost::wsregex_iterator tregex_iterator;
	typedef boost::wsregex_token_iterator tregex_token_iterator;
#endif

	typedef enum tshortcutvalue_type
	{	//���� ����������� ����������
		VAL_TYPE_LOCAL_DIRECTORY	//��������� ����������
		, VAL_TYPE_NET_DIRECTORY	//������� ����������
		, VAL_TYPE_PLUGIN_DIRECTORY //���������� �������
		, VAL_ENVIRONMENT_VARIABLE	//���������� �����
		, VAL_DIRECT_PATH			//������ ���� � ����������
		, VAL_REGISTRY_KEY			//������ �� ���� �������; ���������� � ���� ����� �������� ����, �� ������� ����� ���������� �����
	} _tshortcutvalue_type;

	typedef struct tshortcut_value_parsed
	{
		bool bValueEnabled;
		tshortcutvalue_type ValueType;
		tstring value;
	} _tshortcut_value_parsed;
	typedef std::pair<tshortcut_value_parsed, tshortcut_value_parsed> tshortcut_value_parsed_pair;

	enum twhat_to_search_t
	{
		WTS_DIRECTORIES = 1
		, WTS_FILES = 2
		, WTS_DIRECTORIES_AND_FILES = 3
	};

	typedef enum tcommand_flags 
	{
		FGC_ENABLED_SHORTCUT = 0x1			//�������� ������� �������
		,FGC_ENABLED_LOCAL_DIRECTORY = 0x2	//���������� ���� ������ (������ ��� OPEN)
		,FGC_ENABLED_PARAM = 0x4			//������� ���� ��� ��������� ����������
		,FGC_ENABLED_CATALOG = 0x10			//������ �������
		,FGC_ENABLED_PREFIX = 0x20			//������ �������
	} _tcommand_flags;

	typedef enum tcommands_kinds 
	{	
		QK_OPEN_SHORTCUT				//cd:
		,QK_INSERT_SHORTCUT				//cd::����
		,QK_INSERT_SHORTCUT_TEMPORARY	//cd:+
		,QK_INSERT_BOTH					//cd:::
		,QK_INSERT_BOTH_TEMPORARY		//cd:+:
		,QK_DELETE_SHORTCUT				//cD:-��� ����������
		,QK_DELETE_CATALOG				//cD:-��� ��������/
		,QK_OPEN_NETWORK				/*cd:\\*/
		,QK_OPEN_PANEL					//cd:
		,QK_OPEN_BY_PATH				//cd:~ 
		,QK_INSERT_SHORTCUT_IMPLICIT	//cd:: 
		,QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT	//cd:+ 
		,QK_DELETE_SHORTCUT_IMPLICIT	//cd:-
		,QK_OPEN_ENVIRONMENT_VARIABLE	//cd:%
		,QK_START_SOFT_SHORTCUT			//cd: soft
		,QK_SEARCH_FILE					//cd:--f path
		,QK_SEARCH_DIRECTORIES_AND_FILES //cd:--df path
		// ��� ���������� �������� ������� GetCommandAsString � ������
	} _tcommands_kinds;

	typedef struct tparsed_command
	{
		unsigned long flags;//����������  tcommand_flags -
		//� ����� ���� �������� �������� 
		//+ �������������� ���������� � �������
		tcommands_kinds kind;	//��� �������
		tstring prefix;			//������� cd ��� ��.
		tstring catalog;		//������� 
		tstring shortcut;		//�������� �������
		tstring local_directory;//���������� ���� ����� �������
		tstring param;			//���� ��� ��������� ����������
	} _tparsed_command;

	typedef struct tshortcut_info
	{
		tstring catalog;	//������� � ������� ��������� ���������
		tstring shortcut;	//�������� ������
		bool bIsTemporary;	//���������
	} _tshortcut_info;

	typedef tstring tcatalog_info;	//���������� � �������� (���� ������������ ��������� ��������)

	typedef std::vector<tstring> tcatalogs_list;
	typedef std::list<nf::tshortcut_info> tshortcuts_list;
	typedef std::vector<tstring> tdirs_list;
	typedef std::vector<tstring> tvalues_list;
	typedef std::vector<tstring> tenvvars_list;

	typedef stlsoft::auto_buffer<TCHAR> tautobuffer_char; //!TODO: optimization
	typedef stlsoft::auto_buffer<TCHAR> tautobuffer_char;
	typedef stlsoft::auto_buffer<BYTE> tautobuffer_byte; //!TODO: optimization
	typedef stlsoft::auto_buffer<BYTE> tautobuffer_byte;
// 	typedef std::vector<BYTE> tautobuffer_byte;
// 	typedef std::vector<BYTE> tautobuffer_byte;
// 	typedef std::vector<TCHAR> tautobuffer_char;
// 	typedef std::vector<TCHAR> tautobuffer_char;
}