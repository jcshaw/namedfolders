#include "stdafx.h"
#include ".\Commandpatterns.h"

using namespace nf;
using namespace Patterns;

#include "stlsoft_def.h"
#include "parser.h"

#pragma warning(disable: 4244 4267)
//используется библиотека BOOST
//http://www.boost.org
#define BOOST_REGEX_STATIC_LINK
#include <boost/regex.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#pragma warning(default: 4244 4267)

extern struct FarStandardFunctions g_FSF;

namespace {
	
	enum {	//order is important, see GetResultString
		ID_PREFIX		//префикс cd или др.
		, ID_COMMAND	//команда
		, ID_CATALOG	//каталог 
		, ID_CSDELIM	//разделитель между каталогом и названием псевдонима
		, ID_SHORCUT	//название псевдонима
		, ID_SPDELIM	//разделитель между псевдонимом и локальным путем
		, ID_PATH		//дальнейший путь после ярлычка
		, ID_PARAM		//один или несколько параметров
		
		, NUM_PARTS
	};
	wchar_t const *VAR_NAMES[static_cast<int>(NUM_PARTS)] =  {
		L"prefix"
		, L"command"
		, L"catalog"
		, L"cndelim"
		, L"name"
		, L"npdelim"
		, L"path"
		, L"param"
	};
	tstring get_command_as_string(nf::tcommands_kinds kind) {
		switch (kind) {
		case nf::QK_OPEN_DIRECTORY_DIRECTLY: return L"";
		case nf::QK_OPEN_SHORTCUT: return L"";
		case nf::QK_INSERT_SHORTCUT: return L":";
		case nf::QK_INSERT_SHORTCUT_TEMPORARY: return L"+";
		case nf::QK_INSERT_BOTH: return L"::";
		case nf::QK_INSERT_BOTH_TEMPORARY: return L"+:";
		case nf::QK_DELETE_SHORTCUT: return L"-";
		case nf::QK_DELETE_CATALOG: return L"-";
		case nf::QK_OPEN_NETWORK: return L"\\\\";
		case nf::QK_OPEN_PANEL: return L"";
		case nf::QK_OPEN_BY_PATH: return L"~";
		case nf::QK_INSERT_SHORTCUT_IMPLICIT: return L":";
		case nf::QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT: return L"+";
		case nf::QK_DELETE_SHORTCUT_IMPLICIT: return L"-";	//cd:-
		case nf::QK_OPEN_ENVIRONMENT_VARIABLE: return L"%";//cd:%
		case nf::QK_START_SOFT_SHORTCUT: return L" "; //cd: soft
		case nf::QK_SEARCH_FILE: return L"--f";//cd:--f path
		case nf::QK_SEARCH_DIRECTORIES_AND_FILES: return L"--df";//cd:--f path
		case nf::QK_INSERT_CATALOG: return L"::";
		default: {
			assert(false);
			return L"";
				 }
		} //switch
	}

	bool is_pair_first_equal_to(tpair_strings const& srcPattern, tstring const& Prefix) {
		return lstrcmpi(srcPattern.first.c_str(), Prefix.c_str()) == 0;
	}

	inline tstring itoa(int n) {
		wchar_t buffer[33]; //see description of far itoa function: 32 characters + 1 for "0"
		return g_FSF.itoa(n, &buffer[0], 10);
	}

	inline int atoi(tstring const& s) {
		return g_FSF.atoi(s.c_str());
	}

	class applier {	
		boost::basic_regex<wchar_t> m_RE;
		boost::basic_regex<wchar_t> m_SubRE;
		//typedef std::map<tstring, int, Utils::CmpStringLessCI> tvarnames; //map is removed for reducing size
		typedef std::vector<tpair_strings> tvarnames;
		tvarnames m_VarNames;
		std::vector<tstring> const& m_SrcParts;
		std::vector<tstring>& m_DestParts;
	public:
		applier(std::vector<tstring> const& SrcParts, std::vector<tstring> &DestParts)
			: m_SrcParts(SrcParts)
			, m_DestParts(DestParts)
			, m_RE(applier::get_regexp(), boost::regex_constants::icase)
			, m_SubRE(applier::get_sub_regexp(), boost::regex_constants::icase)
		{ 
			for (int i = 0; i < NUM_PARTS; ++i) {
				m_VarNames.push_back(std::make_pair(VAR_NAMES[i], itoa(i)) );
			}
			m_DestParts[ID_PREFIX] = L"cd:";
		} 
		bool operator()(tstring const& AssignExp) {
			//типичная команда [Name]=[Prefix]
			//левая часть - имя переменной - задает новое значение для DestCmd
			//правая часть - текст и переменные (идентифицирующие значение SrcCmd)
			boost::match_results<wchar_t const*> what;
			if (boost::regex_match(AssignExp.c_str(), what, m_RE)) {
				tstring DestVarName = what[1];
				int n = 0;
				if (! find_variable(DestVarName, n)) return false;
				m_DestParts[n] = parse_right_part(what[2]);
				return true;
			}
			return false;
		}
	private:
		tstring parse_right_part(tstring const& Expression) {
			//!std::basic_stringstream<wchar_t> result;	
				//можно использовать string stream, но это стоит 27 кб в релизе...
				//поэтому заменяем его строкой с заранее выделенной памятью
			tstring result;
			result.reserve(256);
			boost::match_results<wchar_t const*> sub_what;
			std::size_t move = 0;
			int cVarsTotal = 0;
			int cEmptyVars = 0;
			while (move < Expression.size(), boost::regex_search(Expression.c_str() + move, sub_what, m_SubRE)) {	
				move += sub_what[0].length();
				if (! tstring(sub_what[1]).empty()) {	//variable
					int n;
					if (! find_variable(tstring(sub_what[1]), n)) {
						result +=  tstring(L"[") + tstring(sub_what[2]) + tstring(L"]");	//unknown variable
					} else {
						tstring var_value = m_SrcParts[n];
						result += var_value;
						if (var_value.empty()) ++cEmptyVars;
					}					
					++cVarsTotal;
				} else { //text
					result += sub_what[2];
				}
			}
//			r = result.str();
		//!TODO: если все переменные пусты - то суммарная строка тоже пуста
		//команда "zp:" должна открывать директорию, а не показывать список всех поддиректорий
			return (cVarsTotal == cEmptyVars && cEmptyVars) ? L"" : result;
		}
		inline static tstring get_regexp() { //регулярное выражение конструируем из имен переменных заданных в VAR_NAMES
			return L"\\[([^]]+)\\]\\s*=\\s*(.+)";	//(\\[([^]]+)\\])|(.+)
		}
		inline static tstring get_sub_regexp() { //регулярное выражение конструируем из имен переменных заданных в VAR_NAMES
			return L"\\[([^]]+)\\]|([^][]+)";
		}
		inline bool find_variable(tstring const& varName, int& destVarId) {
			tvarnames::const_iterator p = std::find_if(m_VarNames.begin(), m_VarNames.end()
				, boost::bind(&::is_pair_first_equal_to, _1, boost::cref(varName) ) );
			if (p == m_VarNames.end()) return false;

			destVarId = atoi(p->second);
			return true;
		}
	};
};
//////////////////////////////////////////////////////////////////////////
// DetailedCommand
nf::Patterns::Private::DetailedCommand::DetailedCommand(nf::tparsed_command const &SrcCmd)
: m_SrcParts(commandToListParts(SrcCmd))
{
	m_DestParts = m_SrcParts;
}
	
void nf::Patterns::Private::DetailedCommand::ApplyPattern(tstring const& Pattern) {
	//шаблон состоит из одной или более операций присваивания,
	//разделенных ';'. Последовательно выполняем их все
	std::list<tstring> list_tokens;
	Utils::SplitStringByRegex(Pattern, list_tokens, L";");
	applier a(m_SrcParts, m_DestParts);
	std::for_each(list_tokens.begin(), list_tokens.end(), a);
	
	//если есть каталог и псевдоним, а разделить не указан - указываем автоматом
	//аналогично, если есть псевдоним и локальная директория и нет между ними разделителя - задаем его
	if ( (! m_DestParts[ID_CATALOG].empty()) 
		&& (m_DestParts[ID_CSDELIM].empty()) ) m_DestParts[ID_CSDELIM] = SLASH_CATS;

	if ( (! m_DestParts[ID_PATH].empty())
		//&& (! m_DestParts[ID_SHORCUT].empty()) 
		&& (m_DestParts[ID_SPDELIM].empty()) ) m_DestParts[ID_SPDELIM] = SLASH_DIRS;
	//!TODO: если указан каталог и локальная директория, но не указан псевдоним?
}

tstring nf::Patterns::Private::DetailedCommand::GetResultString() const {
	tstring result;
	result.reserve(m_DestParts[ID_SHORCUT].size() + m_DestParts[ID_PATH].size() + 10);
	for (int i = 0; i < NUM_PARTS; ++i) {
		result += m_DestParts[i];
	}
	return result;
}

nf::Patterns::Private::DetailedCommand::tlistparts 
nf::Patterns::Private::DetailedCommand::commandToListParts(nf::tparsed_command const &SrcCmd) const {
	tlistparts parts(NUM_PARTS);
	//копируем составляющие команды в вектор с индексами ID_XXX
	//локальная директория и каталог могут содержать слеши, вычленяем их в отдельные составляющие
	//команду записываем то же в виде строки
	parts[ID_PREFIX] = SrcCmd.prefix;
	parts[ID_PARAM] = SrcCmd.param;
	parts[ID_SHORCUT] = SrcCmd.shortcut;
	parts[ID_COMMAND] = get_command_as_string(SrcCmd.kind);
	if (! SrcCmd.catalog.empty()) {	//переносим из названия каталога завершающие слешы в m_csdelim
		tstring & catalog = parts[ID_CATALOG];
		catalog = SrcCmd.catalog;
		tstring::size_type pos = catalog.find(SLASH_CATS);
		if (pos != tstring::npos) {
			parts[ID_CSDELIM].assign(parts[ID_CATALOG], pos, catalog.size() - pos);
			catalog.erase(pos, catalog.size() - pos);
		}
	}

	if (! SrcCmd.local_directory.empty()) {	//переносим из начала названия локальной директории слешы и точки в m_spdelim
		tstring &path = parts[ID_PATH];
		path = SrcCmd.local_directory; 
		tstring::iterator p = path.begin();
		while (p != path.end()) {
			switch (*p) {
			case L'.':
			case L'\\':
				++p;
				continue;
			};
			break;
		}
		if (p != path.begin()) {
			parts[ID_SPDELIM].assign(path.begin(), p);
			path.erase(path.begin(), p);
		}	
	}
	return parts;	//i hope, clever compiler will optimize it 	
}

//////////////////////////////////////////////////////////////////////////
// CommandPatterns
CommandPatterns::CommandPatterns(tlist_command_patterns const& listPatterns) {
	m_PP.reserve(listPatterns.size());
	BOOST_FOREACH(nf::Patterns::tcommand_pattern const& pattern, listPatterns) {
		m_PP.push_back(pattern);
	}
}

CommandPatterns::~CommandPatterns(void)
{
}

bool CommandPatterns::TransformCommand(nf::tparsed_command const &SrcCmd, tstring &DestCmd) const {
//Prefixes in m_PP are with ":", prefixes in SrcCmd are without
	tstring prefix = SrcCmd.prefix + L":";
	tprefix_pattern_vector::const_iterator p = std::find_if(m_PP.begin(), m_PP.end(), boost::bind(&is_pair_first_equal_to, _1, boost::cref(prefix)));
// 	tmap::const_iterator p = m_PP.find(prefix);
 	if (p == m_PP.end()) return false;

	return TransformCommand(p->second, SrcCmd, DestCmd);
}

bool CommandPatterns::TransformCommand(tstring const& Pattern, nf::tparsed_command const &SrcCmd, tstring &DestCmd) const {
	Private::DetailedCommand dc(SrcCmd);
	dc.ApplyPattern(Pattern);
	DestCmd = dc.GetResultString();
	return true;
}

//////////////////////////////////////////////////////////////////////////
//CommandsManager
namespace {
	inline bool is_prefix_valid(tstring const& prefix) {
		//valid prefix is ended by char ':'
		if (! prefix.size()) return false;
		return prefix[prefix.size()-1] == L':';
	}

	inline bool equal_prefixes(tcommand_pattern &p, tstring const& Prefix) {
		return p.first == Prefix;
	}
}

CommandsManager::CommandsManager(tstring const& RegKey)
: m_Key(HKEY_CURRENT_USER, RegKey.c_str(), true, true)
{
}

CommandsManager::~CommandsManager()
{

}

bool CommandsManager::SetCommand(tstring const& Prefix, tstring const& Pattern) {
	assert(is_prefix_valid(Prefix));
	return m_Key.SetValue(Prefix.c_str(), Pattern.c_str());
}

bool CommandsManager::CheckIfPrefixIsFree(tstring const& Prefix) {
	assert(is_prefix_valid(Prefix));
	tstring dummy;
	return ! m_Key.GetValue(Prefix.c_str(), dummy);
}

bool CommandsManager::RemoveCommand(tstring const& Prefix) {
	assert(is_prefix_valid(Prefix));
	return m_Key.DeleteValue(Prefix.c_str());
}

bool CommandsManager::TransformCommandRecursively(tstring const &SrcCmd, tstring &DestCmd) const {
	tlist_command_patterns all_commands;
	std::list<tstring> used_commands;
	DestCmd = SrcCmd;

	GetListRegisteredCommands(all_commands);
	CommandPatterns cp(all_commands);
	
	while(true) {
		tstring prefix = nf::Parser::ExtractPrefix(DestCmd);
	//если такой префикс не зарегистрирован в качестве шаблонного
	//или если такой префикс уже был испольован (вложенная рекурсия)
	//то возвращаем команду которая получилась к настоящему моменту		
		tlist_command_patterns::const_iterator p = all_commands.begin();
		std::find_if(all_commands.begin(), all_commands.end(), boost::bind(equal_prefixes, _1, prefix));
		if ((p == all_commands.end())
			|| (std::find(used_commands.begin(), used_commands.end(), prefix) != used_commands.end()))
		{	
			return true;
		}
		assert(p != all_commands.end());
		
		nf::tparsed_command cmd;
		nf::Parser::ParseString(DestCmd, cmd);
		if (! cp.TransformCommand(cmd, DestCmd)) return true;
	};

	return true;
}

void CommandsManager::GetListRegisteredCommands(std::list<tcommand_pattern> &ListPatterns) const {
	assert(ListPatterns.empty());
	WinSTL::reg_key_t key(static_cast<HKEY>(m_Key), L"");
	WinSTL::reg_value_sequence_t seq(key);
	BOOST_FOREACH(WinSTL::reg_value_sequence_t::value_type const& value, seq) {
		ListPatterns.push_back(std::make_pair(value.name(), value.value_sz())); 
	}
}

tstring CommandsManager::GetListCommandPrefixes() const
{	//get total list of prefixes registered for template commands 
	tstring list_prefixes;
	list_prefixes.reserve(256);
	WinSTL::reg_key_t key (static_cast<HKEY>(m_Key), L"");
	WinSTL::reg_value_sequence_t seq(key);
	BOOST_FOREACH(WinSTL::reg_value_sequence_t::value_type const& value, seq) {
		list_prefixes += value.name();
	}
	return list_prefixes;
}