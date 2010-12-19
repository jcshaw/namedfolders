/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include <string>
#include <vector>
#include <list>
#include "enforce.h"

#pragma warning(disable: 4244 4267)
//������������ ���������� BOOST
//http://www.boost.org

#ifdef USE_BOOST_XPRESSIVE
#include <boost/xpressive/xpressive.hpp> //#include <boost/regex.hpp>
#define NF_BOOST_REGEX_LIB boost::xpressive
#define NF_BOOST_REGEX_COMPILE nf::tregex::compile
#else 
#define NF_BOOST_REGEX_LIB boost
#define BOOST_REGEX_STATIC_LINK
#define NF_BOOST_REGEX_COMPILE
#include <boost/regex.hpp>
#endif

#include <boost/shared_ptr.hpp> //#include <boost/ptr_container/ptr_vector.hpp>

#pragma warning(default: 4244 4267)

#include <stlsoft/memory/auto_buffer.hpp> 
typedef std::basic_string<wchar_t> tstring;
typedef std::pair<tstring, tstring> tpair_strings;

extern wchar_t const * SLASH_CATS;		//����������� ���������
extern wchar_t const * SLASH_DIRS;		//����������� ����������
extern wchar_t const SLASH_CATS_CHAR;		//����������� ���������
extern wchar_t const SLASH_DIRS_CHAR;		//����������� ����������
extern wchar_t const* LEVEL_UP_TWO_POINTS;

//����������� ��� ������ � �������������� ������� ����������� - ����� � ����
extern wchar_t const*const MC_DEEP_DIRECT_SEARCH_LONG; // \**
extern wchar_t const*const MC_DEEP_REVERSE_SEARCH_LONG; // \..*
extern wchar_t const*const MC_SEARCH_BACKWORD_LONG; // L"..";
extern wchar_t const*const MC_SEARCH_FORWARD_LONG; // L"\*";

//����������� ��� ������ � �������������� ������� ����������� - ����� � ����
extern wchar_t const*const MC_DEEP_DIRECT_SEARCH_SHORT; //one char equivalent for DEEP_DIRECT_SEARCH_STRING
extern wchar_t const*const MC_DEEP_REVERSE_SEARCH_SHORT; //one char equivalent for DEEP_REVERSE_SEARCH_STRING
extern wchar_t const*const MC_SEARCH_BACKWORD_SHORT; // "\.."
extern wchar_t const*const MC_SEARCH_FORWARD_SHORT; // "\*"
extern wchar_t const*const MC_SEARCH_BACKWORD_SHORT_WITHOUT_SLASH; // ".."


extern const wchar_t CHAR_LEADING_VALUE_ENVVAR;	//������ ��� ������������� ���������� ����� � �����
extern const wchar_t CHAR_LEADING_VALUE_REGKEY;	//������ ��� ������������� ������ ������� � ����� 

extern wchar_t const* HISTORY_APPLY_COMMANDS;
extern wchar_t const* HISTORY_COMMAND_PATTERNS;

extern struct PluginStartupInfo g_PluginInfo; 
extern struct FarStandardFunctions g_FSF;


enum {
	DR_CANCEL = 0
	, DR_DELETE = 1
	, DR_DELETE_AND_DELETEALL = 2
	, DR_SKIP = 3
};

namespace nf {

#ifdef USE_BOOST_XPRESSIVE
	typedef boost::xpressive::wsregex tregex;
	typedef boost::xpressive::wsmatch tsmatch;
	typedef boost::xpressive::wsregex_iterator tregex_iterator;
	typedef boost::xpressive::wsregex_token_iterator tregex_token_iterator;	
#else 
	typedef boost::wregex tregex;
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

	typedef struct tshortcut_value_parsed {
		bool bValueEnabled;
		tshortcutvalue_type ValueType;
		tstring value;
	} _tshortcut_value_parsed;
	typedef std::pair<tshortcut_value_parsed, tshortcut_value_parsed> tshortcut_value_parsed_pair;

	enum twhat_to_search_t {
		WTS_DIRECTORIES = 1
		, WTS_FILES = 2
		, WTS_DIRECTORIES_AND_FILES = 3
	};

	typedef enum tcommand_flags {
		FGC_ENABLED_SHORTCUT = 0x1			//�������� ������� �������
		, FGC_ENABLED_LOCAL_DIRECTORY = 0x2	//���������� ���� ������ (������ ��� OPEN)
		, FGC_ENABLED_PARAM = 0x4			//������� ���� ��� ��������� ����������
		, FGC_ENABLED_CATALOG = 0x10			//������ �������
		, FGC_ENABLED_PREFIX = 0x20			//������ �������
	} _tcommand_flags;

	typedef enum tcommands_kinds {	
		QK_OPEN_DIRECTORY_DIRECTLY		//cd:path
		, QK_OPEN_SHORTCUT				//cd:
		, QK_INSERT_SHORTCUT				//cd::shortcut_name
		, QK_INSERT_SHORTCUT_TEMPORARY	//cd:+
		, QK_INSERT_BOTH					//cd:::
		, QK_INSERT_BOTH_TEMPORARY		//cd:+:
		, QK_DELETE_SHORTCUT				//cD:-shortcut_name
		, QK_DELETE_CATALOG				//cD:-catalog_name/
		, QK_OPEN_NETWORK				/*cd:\\*/
		, QK_OPEN_PANEL					//cd:
		, QK_OPEN_BY_PATH				//cd:~ 
		, QK_INSERT_SHORTCUT_IMPLICIT	//cd:: 
		, QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT	//cd:+ 
		, QK_DELETE_SHORTCUT_IMPLICIT	//cd:-
		, QK_OPEN_ENVIRONMENT_VARIABLE	//cd:%
		, QK_START_SOFT_SHORTCUT			//cd: soft
		, QK_SEARCH_FILE					//cd:--f path
		, QK_SEARCH_DIRECTORIES_AND_FILES //cd:--df path
		, QK_INSERT_CATALOG	            //cd::catalog_name/
		// ��� ���������� �������� ������� get_command_as_string � ������
	} _tcommands_kinds;

	typedef struct tparsed_command {
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

	typedef struct tshortcut_info {
		tstring catalog;	//������� � ������� ��������� ���������
		tstring shortcut;	//�������� ������
		bool bIsTemporary;	//���������
	} _tshortcut_info;

	enum tpath_selection_result {
		ID_PATH_SELECTED
		, ID_PATH_NOT_FOUND
		, ID_MENU_CANCELED
	};

	typedef tstring tcatalog_info;	//���������� � �������� (���� ������������ ��������� ��������)

	typedef std::list<tstring> tlist_strings;
	typedef std::list<tpair_strings> tlist_pairs_strings;	
	typedef std::vector<tstring> tvector_strings;
	typedef std::list<nf::tshortcut_info> tshortcuts_list;
	typedef std::list<nf::tcatalog_info> tcatalogs_list;

	typedef stlsoft::auto_buffer<wchar_t> tautobuffer_char;
// 	typedef stlsoft::auto_buffer<BYTE> tautobuffer_byte;
// 	typedef boost::auto_buffer<wchar_t> tautobuffer_char;
// 	typedef boost::auto_buffer<BYTE> tautobuffer_byte;
	typedef std::vector<BYTE> tautobuffer_byte; 

	typedef boost::shared_ptr<tautobuffer_char> tstring_buffer;
	//typedef boost::ptr_vector<tstring_buffer> tlist_buffers;
	typedef std::vector<tstring_buffer> tlist_buffers;

}