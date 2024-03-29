#include "stdafx.h"
#include ".\Commandpatterns.h"

using namespace nf;
using namespace Patterns;

#include <boost/regex.hpp>

#include "stlsoft_def.h"
#include "parser.h"

#pragma warning(disable: 4244 4267)
//������������ ���������� BOOST
//http://www.boost.org
#define BOOST_REGEX_STATIC_LINK
#include <boost/regex.hpp>
#include <boost/bind.hpp>
#pragma warning(default: 4244 4267)

namespace 
{
	
	enum {	//������� �����, ��. GetResultString
		ID_PREFIX		//������� cd ��� ��.
		, ID_COMMAND	//�������
		, ID_CATALOG	//������� 
		, ID_CSDELIM	//����������� ����� ��������� � ��������� ����������
		, ID_SHORCUT	//�������� ����������
		, ID_SPDELIM	//����������� ����� ����������� � ��������� �����
		, ID_PATH		//���������� ���� ����� �������
		, ID_PARAM		//���� ��� ��������� ����������
		
		, NUM_PARTS
	};
	TCHAR const *VAR_NAMES[static_cast<int>(NUM_PARTS)] = 
	{
		_T("prefix")
		, _T("command")
		, _T("catalog")
		, _T("cndelim")
		, _T("name")
		, _T("npdelim")
		, _T("path")
		, _T("param")
	};
	tstring GetCommandAsString(nf::tcommands_kinds kind)
	{
		switch (kind)
		{
		case nf::QK_OPEN_SHORTCUT: return _T("");
		case nf::QK_INSERT_SHORTCUT: return _T(":");
		case nf::QK_INSERT_SHORTCUT_TEMPORARY: return _T("+");
		case nf::QK_INSERT_BOTH: return _T("::");
		case nf::QK_INSERT_BOTH_TEMPORARY: return _T("+:");
		case nf::QK_DELETE_SHORTCUT: return _T("-");
		case nf::QK_DELETE_CATALOG: return _T("-");
		case nf::QK_OPEN_NETWORK: return _T("\\\\");
		case nf::QK_OPEN_PANEL: return _T("");
		case nf::QK_OPEN_BY_PATH: return _T("~");
		case nf::QK_INSERT_SHORTCUT_IMPLICIT: return _T(":");
		case nf::QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT: return _T("+");
		case nf::QK_DELETE_SHORTCUT_IMPLICIT: return _T("-");	//cd:-
		case nf::QK_OPEN_ENVIRONMENT_VARIABLE: return _T("%");//cd:%
		case nf::QK_START_SOFT_SHORTCUT: return _T(" "); //cd: soft
		case nf::QK_SEARCH_FILE: return _T("--f");//cd:--f path
		case nf::QK_SEARCH_DIRECTORIES_AND_FILES: return _T("--df");//cd:--f path
		default: {
			assert(false);
			return _T("");
				 }
		} //switch
	}


	class applier
	{	
		boost::basic_regex<TCHAR>  m_RE;
		boost::basic_regex<TCHAR>  m_SubRE;
		typedef std::map<tstring, int, Utils::CmpStringLessCI> tvarnames;
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
			for (int i = 0; i < NUM_PARTS; ++i) 
			{
				m_VarNames.insert(std::make_pair(VAR_NAMES[i], i));
			}
			m_DestParts[ID_PREFIX] = _T("cd:");
		} 
		bool operator()(tstring const& AssignExp)
		{	//�������� ������� [Name]=[Prefix]
			//����� ����� - ��� ���������� - ������ ����� �������� ��� DestCmd
			//������ ����� - ����� � ���������� (���������������� �������� SrcCmd)
			boost::match_results<TCHAR const*> what;
			if (boost::regex_match(AssignExp.c_str(), what, m_RE))
			{
				tstring DestVarName = what[1];
				tvarnames::const_iterator pdest = m_VarNames.find(DestVarName);
				if (pdest == m_VarNames.end()) return false;	//left side contains unknown variable
				int nDestId = pdest->second;
				m_DestParts[nDestId] = parse_right_part(what[2]);
				return true;
			}
			return false;
		}
	private:
		tstring parse_right_part(tstring const& Expression)
		{
			//!std::basic_stringstream<TCHAR> result;	
				//����� ������������ string stream, �� ��� ����� 27 �� � ������...
				//������� �������� ��� ������� � ������� ���������� �������
			tstring result;
			result.reserve(256);
			boost::match_results<TCHAR const*> sub_what;
			std::size_t move = 0;
			int cVarsTotal = 0;
			int cEmptyVars = 0;
			while (move < Expression.size(), boost::regex_search(Expression.c_str() + move, sub_what, m_SubRE))
			{	
				move += sub_what[0].length();
				if (! tstring(sub_what[1]).empty())
				{	//variable
					tvarnames::const_iterator p = m_VarNames.find(tstring(sub_what[1]));
					if (p != m_VarNames.end())
					{
						tstring var_value = m_SrcParts[p->second];
						result += var_value;
						if (var_value.empty()) ++cEmptyVars;
					} else {
						result +=  tstring(_T("[")) + tstring(sub_what[2]) + tstring(_T("]"));	//unknown variable
					}
					++cVarsTotal;
				} else 
				{	//text
					result += sub_what[2];
				}
			}
//			r = result.str();
		//!TODO: ���� ��� ���������� ����� - �� ��������� ������ ���� �����
		//������� "zp:" ������ ��������� ����������, � �� ���������� ������ ���� �������������
			return (cVarsTotal == cEmptyVars && cEmptyVars) ? _T("") : result;
		}
		inline static tstring get_regexp()
		{	//���������� ��������� ������������ �� ���� ���������� �������� � VAR_NAMES
			return _T("\\[([^]]+)\\]\\s*=\\s*(.+)");	//(\\[([^]]+)\\])|(.+)
		}
		inline static tstring get_sub_regexp()
		{	//���������� ��������� ������������ �� ���� ���������� �������� � VAR_NAMES
			return _T("\\[([^]]+)\\]|([^][]+)");
		}

	};
};
//////////////////////////////////////////////////////////////////////////
// DetailedCommand
Private::DetailedCommand::DetailedCommand(nf::tparsed_command const &SrcCmd)
: m_SrcParts(commandToListParts(SrcCmd))
{
	m_DestParts = m_SrcParts;
}
	
void Private::DetailedCommand::ApplyPattern(tstring const& Pattern)
{
	//������ ������� �� ����� ��� ����� �������� ������������,
	//����������� ';'. ��������������� ��������� �� ���
	std::list<tstring> list_tokens;
	Utils::SplitStringByRegex(Pattern, list_tokens, _T(";"));
	applier a(m_SrcParts, m_DestParts);
	std::for_each(list_tokens.begin(), list_tokens.end(), a);
	
	//���� ���� ������� � ���������, � ��������� �� ������ - ��������� ���������
	//����������, ���� ���� ��������� � ��������� ���������� � ��� ����� ���� ����������� - ������ ���
	if ( (! m_DestParts[ID_CATALOG].empty()) 
		&& (m_DestParts[ID_CSDELIM].empty()) ) m_DestParts[ID_CSDELIM] = SLASH_CATS;

	if ( (! m_DestParts[ID_PATH].empty())
		//&& (! m_DestParts[ID_SHORCUT].empty()) 
		&& (m_DestParts[ID_SPDELIM].empty()) ) m_DestParts[ID_SPDELIM] = SLASH_DIRS;
	//!TODO: ���� ������ ������� � ��������� ����������, �� �� ������ ���������?
}
bool Private::DetailedCommand::GetResultCommand(nf::tparsed_command &DestCmd) const
{
	listPartsToCommand(m_DestParts, DestCmd);
	return true;
}
tstring Private::DetailedCommand::GetResultString() const
{
	tstring result;
	result.reserve(m_DestParts[ID_SHORCUT].size() + m_DestParts[ID_PATH].size() + 10);
	for (int i = 0; i < NUM_PARTS; ++i) 
	{
		result += m_DestParts[i];
	}
	return result;
}

Private::DetailedCommand::tlistparts 
Private::DetailedCommand::commandToListParts(nf::tparsed_command const &SrcCmd) const
{
	tlistparts parts(NUM_PARTS);
	//�������� ������������ ������� � ������ � ��������� ID_XXX
	//��������� ���������� � ������� ����� ��������� �����, ��������� �� � ��������� ������������
	//������� ���������� �� �� � ���� ������
	parts[ID_PREFIX] = SrcCmd.prefix;
	parts[ID_PARAM] = SrcCmd.param;
	parts[ID_SHORCUT] = SrcCmd.shortcut;
	parts[ID_COMMAND] = GetCommandAsString(SrcCmd.kind);
	if (! SrcCmd.catalog.empty())
	{	//��������� �� �������� �������� ����������� ����� � m_csdelim
		tstring & catalog = parts[ID_CATALOG];
		catalog = SrcCmd.catalog;
		tstring::size_type pos = catalog.find(SLASH_CATS);
		if (pos != tstring::npos) 
		{
			parts[ID_CSDELIM].assign(parts[ID_CATALOG], pos, catalog.size() - pos);
			catalog.erase(pos, catalog.size() - pos);
		}
	}

	if (! SrcCmd.local_directory.empty())
	{	//��������� �� ������ �������� ��������� ���������� ����� � ����� � m_spdelim
		tstring &path = parts[ID_PATH];
		path = SrcCmd.local_directory;
		tstring::iterator p = path.begin();
		while (p != path.end())
		{
			switch (*p)
			{
			case _T('.'):
			case _T('\\'):
				++p;
				continue;
			};
			break;
		}
		if (p != path.begin())
		{
			parts[ID_SPDELIM].assign(path.begin(), p);
			path.erase(path.begin(), p);
		}	
	}
	return parts;	//i hope, clever compiler will optimize it 	
}	//ExtractParts

void Private::DetailedCommand::listPartsToCommand(tlistparts const& SrcListParts, nf::tparsed_command &DestCmd) const
{
	tstring command = GetResultString();
	nf::Parser::ParseString(command, DestCmd);
}

//////////////////////////////////////////////////////////////////////////
// CommandPatterns
CommandPatterns::CommandPatterns(tlist_command_patterns const&ListPatterns)
{
	tlist_command_patterns::const_iterator p = ListPatterns.begin();
	while (p != ListPatterns.end())
	{	//!TODO: �������������� ����������� ����������
		m_PP.insert(*p);//std::make_pair(p->prefix, p->pattern)*/);
		++p;
	}
}

CommandPatterns::~CommandPatterns(void)
{
}

bool CommandPatterns::TransformCommand(nf::tparsed_command const &SrcCmd
									   , tstring &DestCmd) const
{
	//��������: � map �������� � ����������, � � SrcCmd - ���
	tstring prefix = SrcCmd.prefix + _T(":");
	tmap::const_iterator p = m_PP.find(prefix);
	if (p == m_PP.end()) return false;

	return TransformCommand(p->second, SrcCmd, DestCmd);
}

bool CommandPatterns::TransformCommand(tstring const& Pattern
									   , nf::tparsed_command const &SrcCmd
									   , tstring &DestCmd) const
{
	Private::DetailedCommand dc(SrcCmd);
	dc.ApplyPattern(Pattern);
	DestCmd = dc.GetResultString();
	return true;
}

//////////////////////////////////////////////////////////////////////////
//CommandsManager
namespace
{
	inline bool PrefixIsValid(tstring const& Prefix)
	{
		//�������� ������� ������������� ����������
		if (! Prefix.size()) return false;
		return Prefix[Prefix.size()-1] == _T(':');
	}
}

CommandsManager::CommandsManager(tstring const& RegKey)
: m_Key(HKEY_CURRENT_USER, RegKey.c_str(), true, true)
{
}

CommandsManager::~CommandsManager()
{

}

bool CommandsManager::SetCommand(tstring const& Prefix, tstring const& Pattern)
{
	assert(PrefixIsValid(Prefix));
	return m_Key.SetValue(Prefix.c_str(), Pattern.c_str());
}

bool CommandsManager::CheckIfPrefixIsFree(tstring const& Prefix)
{
	assert(PrefixIsValid(Prefix));
	tstring dummy;
	return ! m_Key.GetValue(Prefix.c_str(), dummy);
}

bool CommandsManager::RemoveCommand(tstring const& Prefix)
{
	assert(PrefixIsValid(Prefix));
	return m_Key.DeleteValue(Prefix.c_str());
}

inline bool equal_prefixes(tcommand_pattern &p, tstring const& Prefix)
{
	return p.first == Prefix;
}
bool CommandsManager::TransformCommandRecursively(tstring const &SrcCmd
												  , tstring &DestCmd) const
{
	tlist_command_patterns all_commands;
	std::list<tstring> used_commands;
	DestCmd = SrcCmd;

	GetListRegisteredCommands(all_commands);
	CommandPatterns cp(all_commands);
	
	while(true)
	{
		tstring prefix = nf::Parser::ExtractPrefix(DestCmd);
	//���� ����� ������� �� ��������������� � �������� ����������
	//��� ���� ����� ������� ��� ��� ���������� (��������� ��������)
	//�� ���������� ������� ������� ���������� � ���������� �������		
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

namespace 
{
	inline int append(std::list<tcommand_pattern> &DestList, WinSTL::reg_value_sequence_t::value_type const& Value)
	{
		DestList.push_back(std::make_pair(Value.name(), Value.value_sz())); 
		return 0;
	}
	inline int tstring_append(tstring& Src, tstring const& ToAppend) {return Src+= ToAppend, 0;}
	typedef WinSTL::reg_value_sequence_t tsequence;
}

void CommandsManager::GetListRegisteredCommands(std::list<tcommand_pattern> &ListPatterns) const
{
	assert(ListPatterns.empty());
	WinSTL::reg_key_t key(static_cast<HKEY>(m_Key), _T(""));
	::tsequence seq(key);
	std::for_each(seq.begin(), seq.end(), boost::bind(append, boost::ref(ListPatterns), _1));
}

tstring CommandsManager::GetListCommandPrefixes() const
{	//������ ��������� ������������������ ��� ��������� ������
	tstring list_prefixes;
	list_prefixes.reserve(256);
	WinSTL::reg_key_t key(static_cast<HKEY>(m_Key), _T(""));
	::tsequence seq(key);
	std::for_each(seq.begin(), seq.end()
		, boost::bind(&tstring_append, boost::ref(list_prefixes), boost::bind(&::tsequence::value_type::name, _1)));
	return list_prefixes;
}