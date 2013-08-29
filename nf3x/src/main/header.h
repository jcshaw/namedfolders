/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include <string>
#include <vector>
#include <list>

#pragma warning(disable: 4244 4267)
//используетс€ библиотека BOOST
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

extern wchar_t const* PREFIX_CD;
extern wchar_t const* PREFIX_ST;

extern wchar_t const * SLASH_CATS;		//разделитель каталогов
extern wchar_t const * SLASH_DIRS;		//разделитель директорий
extern wchar_t const SLASH_CATS_CHAR;		//разделитель каталогов
extern wchar_t const SLASH_DIRS_CHAR;		//разделитель директорий
extern wchar_t const* LEVEL_UP_TWO_POINTS;

extern wchar_t const* COMMAND_PARAM_ADD_WIN7_LIBRARY;
extern wchar_t const* COMMAND_PARAM_DELETE_WIN7_LIBRARY;

//спецсимволы дл€ поиска с неограниченным уровнем вложенности - вверх и вниз
extern wchar_t const*const MC_DEEP_DIRECT_SEARCH_LONG; // \**
extern wchar_t const*const MC_DEEP_REVERSE_SEARCH_LONG; // \..*
extern wchar_t const*const MC_SEARCH_BACKWORD_LONG; // L"..";
extern wchar_t const*const MC_SEARCH_FORWARD_LONG; // L"\*";

//спецсимволы дл€ поиска с неограниченным уровнем вложенности - вверх и вниз
extern wchar_t const*const MC_DEEP_DIRECT_SEARCH_SHORT; //one char equivalent for DEEP_DIRECT_SEARCH_STRING
extern wchar_t const*const MC_DEEP_REVERSE_SEARCH_SHORT; //one char equivalent for DEEP_REVERSE_SEARCH_STRING
extern wchar_t const*const MC_SEARCH_BACKWORD_SHORT; // "\.."
extern wchar_t const*const MC_SEARCH_FORWARD_SHORT; // "\*"
extern wchar_t const*const MC_SEARCH_BACKWORD_SHORT_WITHOUT_SLASH; // ".."


extern const wchar_t CHAR_LEADING_VALUE_ENVVAR;	//символ дл€ идентификации переменных среды в пут€х
extern const wchar_t CHAR_LEADING_VALUE_REGKEY;	//символ дл€ идентификации ключей реестра в пут€х 

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
	{	//типы именованных директорий
		VAL_TYPE_LOCAL_DIRECTORY	//локальна€ директори€
		, VAL_TYPE_NET_DIRECTORY	//сетева€ директори€
		, VAL_TYPE_PLUGIN_DIRECTORY //директори€ плагина
		, VAL_ENVIRONMENT_VARIABLE	//переменна€ среды
		, VAL_DIRECT_PATH			//пр€мой путь к директории
		, VAL_REGISTRY_KEY			//ссылка на ключ реестра; переменные в этом ключе содержат пути, из которых нужно произвести выбор
		, VAL_KNOWN_FOLDER			//shell:KnownFolderName
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
		FGC_ENABLED_SHORTCUT = 0x1			//название €рлычка указано
		, FGC_ENABLED_LOCAL_DIRECTORY = 0x2	//дальнейший путь указан (только при OPEN)
		, FGC_ENABLED_PARAM = 0x4			//указаны один или несколько параметров
		, FGC_ENABLED_CATALOG = 0x10			//указан каталог
		, FGC_ENABLED_PREFIX = 0x20			//указан префикс
	} _tcommand_flags;

	typedef enum tcommands_kinds {	
		QK_OPEN_DIRECTORY_DIRECTLY		//cd:path
		, QK_OPEN_SHORTCUT				//cd:
		, QK_INSERT_SHORTCUT				//cd::shortcut_name
		, QK_INSERT_SHORTCUT_TEMPORARY	//cd:+
		, QK_INSERT_BOTH					//cd:::
		, QK_INSERT_BOTH_TEMPORARY		//cd::+
		, QK_INSERT_BOTH_IMPLICIT		//cd::: (shortcut is empty, catalog could be not empty)
		, QK_INSERT_BOTH_TEMPORARY_IMPLICIT //cd::+ (shortcut is empty, catalog could be not empty)
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
		// при добавлении изменить функцию get_command_as_string и парсер
	} _tcommands_kinds;

	typedef struct tparsed_command {
		unsigned long flags;//комбинаци€  tcommand_flags -
		//в какие пол€ занесены значени€ 
		//+ дополнительна€ информаци€ о команде
		tcommands_kinds kind;	//тип команды
		tstring prefix;			//префикс cd или др.
		tstring catalog;		//каталог 
		tstring shortcut;		//название €рлычка
		tstring local_directory;//дальнейший путь после €рлычка
		tstring param;			//один или несколько параметров
	} _tparsed_command;

	typedef struct tshortcut_info {
		tstring catalog;	//каталог в котором находитс€ псевдоним
		tstring shortcut;	//название €рлыка
		bool bIsTemporary;	//временный
	} _tshortcut_info;

	enum tpath_selection_result {
		ID_PATH_SELECTED
		, ID_PATH_NOT_FOUND
		, ID_MENU_CANCELED
	};

	typedef struct tcatalog_properties {
		bool show_in_disk_menu;
		bool flag_add_to_win7_lib_by_default;
		tstring default_win7_lib;
	} _tcatalog_properties;

	typedef tstring tcatalog_info;	//информаци€ о каталоге (путь относительно корневого каталога)

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

	typedef enum {
		ASTERIX_MODE_BOTH = 0  //c -> *c*
		, ASTERIX_MODE_POSTFIX = 1 //c -> c*
		, ASTERIX_MODE_AS_IS = 2 //c -> c
	} tasterix_mode;


	/// GUID плагина Named Folders (требуетс€ в FAR3)
	/// {C6A35DB8-6846-449E-A1DC-2FC152ED4B0B}
	extern const GUID NF_PLUGIN_GUID;
	/// {B90A8402-14F7-4382-9275-C79824DD8A24}
	extern const GUID NF_DISKMENU_GUID;

	// {FE4E3C90-3590-4F76-9D05-D30FA3338F9E}
	extern const GUID NF_PLUGINSMENU_GUID;

	// {F30CBBA9-397A-4BF9-81FB-3154B8FBCD90}
	extern const GUID NF_CONFIGMENU_GUID;

	extern const GUID NF_DIALOG_CONFIGURE;
	extern const GUID NF_DIALOG_APPLY_COMMAND;
	extern const GUID NF_DIALOG_EDIT_CATALOG;
	extern const GUID NF_DIALOG_EDIT_PATTERN;
	extern const GUID NF_DIALOG_EDIT_SHORTCUT;
	extern const GUID NF_DIALOG_MOVE;

	extern const GUID NF_MESSAGE_SAFE_PANEL_STATE;
}