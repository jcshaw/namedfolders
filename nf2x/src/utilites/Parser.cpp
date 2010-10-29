/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "parser.h"
#include "far_impl.h"
#include "strings_utils.h"
#include "settings.h"

namespace re	//���������� ���������
{
//������� ���� �������
//������ LIST_COMMANDS ������ ������������������ ������
//� ������� RE_LIST_COMMANDS � ��������������� ������������������
//����������� ���������� ���������, ����������� ���������� ��� �������
//� ���������� ��������� �� ������ ������� (�� �������� ��������� ���������� ���������� RE_PREFIX)
//������ ���������� ��������� ������������ ������ �� ��� ����� - ������� (���� ��� ��������� ������������)
//� ���� � ������ � ����������
	const int NUM_COMMANDS = 17;
	nf::tcommands_kinds LIST_COMMANDS[NUM_COMMANDS] = {
		nf::QK_OPEN_SHORTCUT		
		,nf::QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT	
		,nf::QK_INSERT_SHORTCUT_IMPLICIT	
		,nf::QK_SEARCH_FILE
		,nf::QK_SEARCH_DIRECTORIES_AND_FILES
		,nf::QK_DELETE_SHORTCUT_IMPLICIT	
		,nf::QK_INSERT_SHORTCUT
		,nf::QK_INSERT_SHORTCUT_TEMPORARY
		,nf::QK_INSERT_BOTH_TEMPORARY
		,nf::QK_INSERT_BOTH
		,nf::QK_DELETE_SHORTCUT
		,nf::QK_DELETE_CATALOG
		,nf::QK_OPEN_NETWORK
		,nf::QK_OPEN_PANEL
		,nf::QK_OPEN_BY_PATH
		,nf::QK_OPEN_ENVIRONMENT_VARIABLE
		,nf::QK_START_SOFT_SHORTCUT

	};	//������������������ ������ � ������ RE_LIST_COMMANDS
	TCHAR const* LIST_RE[NUM_COMMANDS] = {
		_T("()([\\*\\?\\w\\d_\\.@#\\(\\)].*)")
		,_T("(\\+)((?:.+\\/)?)$")
		,_T("(:)((?:.+\\/)?)$")
		,_T("(\\-\\-f\\s+)(.*)")
		,_T("(\\-\\-df\\s+)(.*)")
		,_T("(\\-)((?:.+\\/)?)$")
		,_T("(:)([^:\\+].*)")
		,_T("(\\+)([^:\\+].*)")
		,_T("(:\\+)(.*)")	//QK_INSERT_BOTH_TEMPORARY
		,_T("(::)(.*)")
		,_T("(\\-)(.*[^\\/]\\s*)")
		,_T("(\\-)(.*\\/\\s*)")
		,_T("(\\\\)(.*)")
		,_T("(\\s*)()$")
		,_T("(~)(.*)")
		,_T("(%)(.*)")
		,_T("( )(.*)")
	};
//������� �������
	TCHAR const * RE_PREFIX = _T("^((?:[\\w]+)|(?::)):");
//������� �������/�������\����������
	TCHAR const * RE_CSD = _T("([^\\s\\.\\\\]*\\/)?(\\.?[^\\s\\.\\/\\\\]*)?([\\.\\\\][^\\s]*)?(\\s+.+)?");
//������� ���� ���������
	TCHAR const * RE_SOFT = _T("([^\\s]+)(\\s+\"?([^\"]*)\"?)?");
//		_T("([^\\\\]*\\/)?([^\\/\\\\]*)?(\\\\[^\\s]+)?(\\s+.+)?");
//������� ���� ����������� ���������� �����
	TCHAR const * RE_EV = _T("%?([^%]+)(%?)(.*)");

//����� ������������
	TCHAR const* RE_SEARCH_META = 
		_T("(?:.+\\\\\\\\)|(?:\\*)|(?:\\?)|(?:\\[[^]]+\\])");
	TCHAR const* RE_SEARCH_META_INTOKENS_ONLY =	//����������� � ��������� (*, [], ?) �� �� ���� (�����)
		_T("(?:\\*)|(?:\\?)|(?:\\[[^]]+\\])");

}

using namespace nf;
using namespace Parser;

bool nf::Parser::ParseString(tstring const &source, 
						  nf::tparsed_command &t)
{
	tstring csdp;
	t.flags = 0;

	if (! GetCommandKind(source, t.kind, t.prefix,  csdp)) return false;
	if (! t.prefix.empty()) t.flags = t.flags | nf::FGC_ENABLED_PREFIX;

	if (t.kind == nf::QK_START_SOFT_SHORTCUT)
	{
		nf::tregex expression(re::RE_SOFT);
		nf::tsmatch what;
		if (boost::regex_match(csdp, what, expression))
		{
			t.catalog = _T("");
			t.local_directory = _T("");
			t.shortcut = what[1] + tstring(_T(" ")) + what[2];
			t.param = _T("");	//!TODO: ������ ��������� ��������� � shortcut
		}
	} else 
	if (ParseCSDP(csdp, t.catalog, t.shortcut, t.local_directory, t.param))
	{
		if (! t.catalog.empty()) t.flags = t.flags | nf::FGC_ENABLED_CATALOG;
		if (! t.shortcut.empty()) t.flags = t.flags | nf::FGC_ENABLED_SHORTCUT;
		if (! t.local_directory.empty()) t.flags = t.flags | nf::FGC_ENABLED_LOCAL_DIRECTORY;
		if (! t.param.empty()) t.flags = t.flags | nf::FGC_ENABLED_PARAM;
	};

	return true;
}

bool nf::Parser::GetCommandKind(tstring const& source
							, nf::tcommands_kinds &kind
							, tstring &prefix
							, tstring &csdp)
{
	for (int i = 0; i < re::NUM_COMMANDS; ++i)
	{
		tstring rexp(re::RE_PREFIX);
		rexp += re::LIST_RE[i];	

		nf::tregex expression(rexp.c_str());
		
		nf::tsmatch what;
		if (boost::regex_match(source, what, expression))
		{
			prefix = what[1];
			csdp = what[3];
			kind = re::LIST_COMMANDS[i];
			return true;
		}
	}

	return false;
}

namespace
{
	void remove_prefix_from_shortcut(tstring &s)
	{	//���� ������ s �������� ������� (\w\w+:), �� ������� ��� �� s
		//�������, ����� ��������� ������ cd:cd:
		size_t npos = s.find(_T(':'));
		if (npos != tstring::npos)
		if (npos > 1)
		{
			s.erase(0, npos+1);
		}
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
bool nf::Parser::ParseCSDP(tstring const&csdp, tstring &c, tstring &s, tstring &d, tstring &p)
{	//��������� �������/�������\���������� �� ������������
	tregex expression(re::RE_CSD);
	tsmatch what;
	if (boost::regex_match(csdp, what, expression))
	{
		c = what[1];
		s = what[2];
		remove_prefix_from_shortcut(s);
		d = what[3];
		if (d.size() == 1) d += _T('*'); //������ ������ �����������; ���������� �� �������
		p = what[4];
		return true;
	}

	return false;
}

bool nf::Parser::IsTokenMatchedToPattern(tstring const &stoken
										 , tstring const &spattern
										 , bool bAddTrailingAsterix)
{
	//������� FAR ��������� ������������ ������ � OEM
	tstring t1 = Utils::GetInternalToOem(stoken);
	tstring t2 = Utils::GetInternalToOem(spattern);
	return IsTokenMatchedToPatternOEM(t1, t2, bAddTrailingAsterix);
}

bool nf::Parser::IsTokenMatchedToPatternOEM(tstring const& stoken
										, tstring const &spattern
										, bool bAddTrailingAsterix)
{
	//	��������� ��� ����� �������� � ������ ������������ ? � * (���� ������ � ����� ���-�� ��������)
	//	"a*" ������������� "abc" � "a",  "b?" ������������� "ba" � "bc"
	tstring p = spattern;
	if (bAddTrailingAsterix) 
	{
		tstring p = spattern + tstring(_T("*"));
		return FarCmpName(p.c_str(), stoken.c_str(), FALSE);
	} else {
		return FarCmpName(spattern.c_str(), stoken.c_str(), FALSE);
	}
}

bool nf::Parser::IsContainsMetachars(tstring const& sToken)
{
	return (boost::regex_search(sToken, tsmatch(), tregex( re::RE_SEARCH_META)));
}

bool nf::Parser::IsContainsMetachars_InTokensOnly(tstring const& sToken)
{
	return (boost::regex_search(sToken, tsmatch(), tregex(re::RE_SEARCH_META_INTOKENS_ONLY)));
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// ���������� ���� � ������� ������� ���������� �����
// ��������� ��������: "VarName", "%VarName", "%VarName%", "%VarName%LocalPath", "VarName%"
// ���������� true, ���� �������� VarName ������ �� ����� (�������� 1, 2)
bool nf::Parser::ParseEnvVarPath(tstring const& Src
								 , tstring& VarName
								 , tstring& LocalPath)
{
	LocalPath.clear();
	tsmatch what;
	if (! boost::regex_search(Src, what, tregex(re::RE_EV))) 
	{
		VarName = Src;
		return false;
	}
	VarName = what[1];
	LocalPath = what[3];
	return ! tstring(what[2]).empty();
}

tstring nf::Parser::ExtractPrefix(tstring const &CommandString)
{
//����� �������� ��� ����������� ���������
	if (CommandString.empty()) return _T("");

	tstring::const_iterator p = std::find(++CommandString.begin(), CommandString.end(), _T(':'));
	if (p == CommandString.end()) return _T("");

	return tstring(CommandString.begin(), p + 1);
}

tstring nf::Parser::ConvertToMask(tstring const& SrcStr)
{	
	//��������� ��������� ������ ���� �����, ��������� �������������, �� �������� ������������
	//���� � ��� ����� \a\b*\c �� ��� ������ �������������� ��� ����� \*a*\b*\*c* 
	//������� ������������ a, b*, c �� �����������
	//������� �������������� ����� a -> *a* ������������ ������� ��������
	if (nf::Parser::IsContainsMetachars(SrcStr)) return SrcStr;

	tstring mode = CSettings::GetInstance().GetValue(nf::ST_ASTERIX_MODE);
	//1: a -> a*
	//2: a -> a
	//0: a -> *a*

	tstring result = SrcStr;
	if (mode == _T("2")) return SrcStr;
	result.push_back(_T('*'));	
	if (mode == _T("1")) return result;

	result.insert(0, _T("*"));	
	return result;
}
