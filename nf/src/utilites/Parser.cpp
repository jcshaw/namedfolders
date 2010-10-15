/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
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
	wchar_t const* LIST_RE[NUM_COMMANDS] = {
		L"()([\\*\\?\\w\\d_\\.@#\\(\\)].*)"
		,L"(\\+)((?:.+\\/)?)$"
		,L"(:)((?:.+\\/)?)$"
		,L"(\\-\\-f\\s+)(.*)"
		,L"(\\-\\-df\\s+)(.*)"
		,L"(\\-)((?:.+\\/)?)$"
		,L"(:)([^:\\+].*)"
		,L"(\\+)([^:\\+].*)"
		,L"(:\\+)(.*)"	//QK_INSERT_BOTH_TEMPORARY
		,L"(::)(.*)"
		,L"(\\-)(.*[^\\/]\\s*)"
		,L"(\\-)(.*\\/\\s*)"
		,L"(\\\\)(.*)"
		,L"(\\s*)()$"
		,L"(~)(.*)"
		,L"(%)(.*)"
		,L"( )(.*)"
	};
//������� �������
	wchar_t const * RE_PREFIX = L"^((?:[\\w]+)|(?::)):";
//������� �������/�������\����������
	wchar_t const * RE_CSD = L"([^\\s\\.\\\\]*\\/)?(\\.?[^\\s\\.\\/\\\\]*)?([\\.\\\\][^\\s]*)?(\\s+.+)?";
//������� ���� ���������
	wchar_t const * RE_SOFT = L"([^\\s]+)(\\s+\"?([^\"]*)\"?)?";
//		L"([^\\\\]*\\/)?([^\\/\\\\]*)?(\\\\[^\\s]+)?(\\s+.+)?";
//������� ���� ����������� ���������� �����
	wchar_t const * RE_EV = L"%?([^%]+)(%?)(.*)";

//����� ������������
	wchar_t const* RE_SEARCH_META = 
		L"(?:.+\\\\\\\\)|(?:\\*)|(?:\\?)|(?:\\[[^]]+\\])";
	wchar_t const* RE_SEARCH_META_INTOKENS_ONLY =	//����������� � ��������� (*, [], ?) �� �� ���� (�����)
		L"(?:\\*)|(?:\\?)|(?:\\[[^]]+\\])";

}

using namespace nf;
using namespace Parser;

bool nf::Parser::ParseString(tstring const &srcStr, nf::tparsed_command &t)
{
	tstring csdp;
	t.flags = 0;

	if (! GetCommandKind(srcStr, t.kind, t.prefix,  csdp)) return false;
	if (! t.prefix.empty()) t.flags = t.flags | nf::FGC_ENABLED_PREFIX;

	if (t.kind == nf::QK_START_SOFT_SHORTCUT) {
		nf::tregex expression(re::RE_SOFT);
		nf::tsmatch what;
		if (boost::regex_match(csdp, what, expression)) {
			t.catalog = L"";
			t.local_directory = L"";
			t.shortcut = what[1] + tstring(L" ") + what[2];
			t.param = L"";	//!TODO: ������ ��������� ��������� � shortcut
		}
	} else if (ParseCSDP(csdp, t.catalog, t.shortcut, t.local_directory, t.param)) {
		if (! t.catalog.empty()) t.flags = t.flags | nf::FGC_ENABLED_CATALOG;
		if (! t.shortcut.empty()) t.flags = t.flags | nf::FGC_ENABLED_SHORTCUT;
		if (! t.local_directory.empty()) t.flags = t.flags | nf::FGC_ENABLED_LOCAL_DIRECTORY;
		if (! t.param.empty()) t.flags = t.flags | nf::FGC_ENABLED_PARAM;
	};
	return true;
}

bool nf::Parser::GetCommandKind(tstring const& source, nf::tcommands_kinds &kind, tstring &prefix, tstring &csdp) {
	for (int i = 0; i < re::NUM_COMMANDS; ++i) {
		tstring rexp(re::RE_PREFIX);
		rexp += re::LIST_RE[i];	

		nf::tregex expression(rexp.c_str());
		
		nf::tsmatch what;
		if (boost::regex_match(source, what, expression)) {
			prefix = what[1];
			csdp = what[3];
			kind = re::LIST_COMMANDS[i];
			return true;
		}
	}
	return false;
}

namespace {
	void remove_prefix_from_shortcut(tstring &s)
	{	//���� ������ s �������� ������� (\w\w+:), �� ������� ��� �� s; �������, ����� ��������� ������ cd:cd:
		size_t npos = s.find(L':');
		if (npos != tstring::npos)
		if (npos > 1) {
			s.erase(0, npos+1);
		}
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
bool nf::Parser::ParseCSDP(tstring const&csdp, tstring &c, tstring &s, tstring &d, tstring &p)
{	//��������� �������/�������\���������� �� ������������
	tregex expression(re::RE_CSD);
	tsmatch what;
	if (boost::regex_match(csdp, what, expression)) {
		c = what[1];
		s = what[2];
		remove_prefix_from_shortcut(s);
		d = what[3];
		if (d.size() == 1) d += L'*'; //������ ������ �����������; ���������� �� �������
		p = what[4];
		return true;
	}

	return false;
}

bool nf::Parser::IsTokenMatchedToPattern(tstring const& srcToken, tstring const &srcPattern, bool bAddTrailingAsterix) {
	//	��������� ��� ����� �������� � ������ ������������ ? � * (���� ������ � ����� ���-�� ��������)
	//	"a*" ������������� "abc" � "a",  "b?" ������������� "ba" � "bc"
	if (bAddTrailingAsterix) {
		tstring p = srcPattern + tstring(L"*");
		return FarCmpName(p.c_str(), srcToken.c_str(), FALSE) != 0;
	} else {
		return FarCmpName(srcPattern.c_str(), srcToken.c_str(), FALSE) != 0;
	}
}

bool nf::Parser::ContainsMetachars(tstring const& sToken) {
	return (boost::regex_search(sToken, tsmatch(), tregex(re::RE_SEARCH_META)));
}

bool nf::Parser::IsContainsMetachars_InTokensOnly(tstring const& sToken) {
	return (boost::regex_search(sToken, tsmatch(), tregex(re::RE_SEARCH_META_INTOKENS_ONLY)));
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// ���������� ���� � ������� ������� ���������� �����
// ��������� ��������: "VarName", "%VarName", "%VarName%", "%VarName%LocalPath", "VarName%"
// ���������� true, ���� �������� VarName ������ �� ����� (�������� 1, 2)
bool nf::Parser::ParseEnvVarPath(tstring const& srcStr, tstring& varName, tstring& localPath) {
	localPath.clear();
	tsmatch what;
	if (! boost::regex_search(srcStr, what, tregex(re::RE_EV))) {
		varName = srcStr;
		return false;
	}
	varName = what[1];
	localPath = what[3];
	return ! tstring(what[2]).empty();
}

tstring nf::Parser::ExtractPrefix(tstring const &srcCommand) {
//regexp is not necessary here
	if (srcCommand.empty()) return L"";

	tstring::const_iterator p = std::find(++srcCommand.begin(), srcCommand.end(), L':');
	if (p == srcCommand.end()) return L"";

	return tstring(srcCommand.begin(), p + 1);
}

tstring nf::Parser::ConvertToMask(tstring const& srcStr) {	
	//��������� ��������� ������ ���� �����, ��������� �������������, �� �������� ������������
	//���� � ��� ����� \a\b*\c �� ��� ������ �������������� ��� ����� \*a*\b*\*c* 
	//������� ������������ a, b*, c �� �����������
	//������� �������������� ����� a -> *a* ������������ ������� ��������
	if (nf::Parser::ContainsMetachars(srcStr)) return srcStr;

	tstring mode = CSettings::GetInstance().GetValue(nf::ST_ASTERIX_MODE);
	//1: a -> a*
	//2: a -> a
	//0: a -> *a*

	tstring result = srcStr;
	if (mode == L"2") return srcStr;
	result.push_back(L'*');	
	if (mode == L"1") return result;

	result.insert(0, L"*");	
	return result;
}
