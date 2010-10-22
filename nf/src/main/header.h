/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include <string>
#include <vector>
#include "enforce.h"

#pragma warning(disable: 4244 4267)
//������������ ���������� BOOST
//http://www.boost.org
#define BOOST_REGEX_STATIC_LINK
#include <boost/regex.hpp>
#pragma warning(default: 4244 4267)

#include <stlsoft/memory/auto_buffer.hpp>

typedef std::basic_string<wchar_t> tstring;
typedef std::pair<tstring, tstring> tpair_strings;

//typedef std::basic_string<char> tstring;

extern wchar_t const * SLASH_CATS;		//����������� ���������
extern wchar_t const * SLASH_DIRS;		//����������� ����������
extern wchar_t const SLASH_CATS_CHAR;		//����������� ���������
extern wchar_t const SLASH_DIRS_CHAR;		//����������� ����������
extern wchar_t const* LEVEL_UP_TWO_POINTS;

//����������� ��� ������ � �������������� ������� ����������� - ����� � ����
extern wchar_t const*const DEEP_DIRECT_SEARCH;
extern wchar_t const*const DEEP_REVERSE_SEARCH;
extern wchar_t const*const DEEP_UP_DIRECTORY;


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
	typedef boost::wregex tregex;
	typedef boost::wcmatch tcmatch;
	typedef boost::wsmatch tsmatch;
	typedef boost::wsregex_iterator tregex_iterator;
	typedef boost::wsregex_token_iterator tregex_token_iterator;

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

	typedef tstring tcatalog_info;	//���������� � �������� (���� ������������ ��������� ��������)

	typedef std::vector<tstring> tcatalogs_list;
	typedef std::list<nf::tshortcut_info> tshortcuts_list;
	typedef std::vector<tstring> tdirs_list;
	typedef std::vector<tstring> tvalues_list;
	typedef std::vector<tstring> tenvvars_list;

	typedef stlsoft::auto_buffer<wchar_t> tautobuffer_char;
	typedef stlsoft::auto_buffer<BYTE> tautobuffer_byte;
// 	typedef std::vector<BYTE> tautobuffer_byte;
// 	typedef std::vector<wchar_t> tautobuffer_char;


	namespace Private {
		struct Logger {
			template <class T>
			static void Output(const T& level, std::basic_string<wchar_t> const& Message) { 
				//!TODO: write message to log file
			}
		};
	}

	class nf_exception : std::exception {
	public:
		nf_exception(std::basic_string<wchar_t> const& Message, const wchar_t* Locus)
			: m_Message(Message)
		{		
			nf::Private::Logger::Output(0, Message + tstring(L"; ") + Locus);
		}
		inline tstring const& GetMessage() const { 
			return m_Message;}
	private:
		tstring m_Message;
	};

	namespace Private 
	{
		struct Raiser {
			template <class T>
			static void Throw(const T&, const std::basic_string<wchar_t>& message, const wchar_t* locus) {			
				throw nf_exception(message, locus);
			};
		}; //Raiser
	} //Private

	typedef nf::Private::Raiser traiser;

#define ENFORCE(exp) \
	*Ext::MakeEnforcer<Ext::Private::DefaultPredicate, nf::traiser>((exp), _T("Expression '") _TEXT(#exp) L"' failed in '" \
	_TEXT(__FILE__) _TEXT("', line: ") _T( STRINGIZE(__LINE__)) )

}