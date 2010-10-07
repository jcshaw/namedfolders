// ParserTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ParserTest.h"

#include "./utilites/Parser.h"
//#include "./utilites/listpaths.h"

#include <boost/test/unit_test.hpp>

using boost::unit_test::test_suite;


//////////////////////////////////////////////////////////////////////////
bool get_result(tstring const& Src, tstring const& VarName, tstring const& LocalPath, bool bIncomplete)
{
	tstring varname;
	tstring localpath;
	bool b = nf::Parser::ParseEnvVarPath(Src, varname, localpath);
	bool bResult = (VarName == varname) && (LocalPath == localpath) && (b == bIncomplete);
	return bResult;
}

void nf::ut::test_ParseEnvVarPath()
{
	tstring var_path_p = L"path%";
	tstring var_path = L"path";
	tstring var_pp_path = L"%path%";
	tstring var_p_path = L"%path";
	tstring var_pp_path_local = L"%path%\\doc";
	
	BOOST_CHECK(get_result(var_path_p, L"path", L"", true)); 
	BOOST_CHECK(get_result(var_path, L"path", L"", false)); 
	BOOST_CHECK(get_result(var_pp_path, L"path", L"", true)); 
	BOOST_CHECK(get_result(var_p_path, L"path", L"", false)); 
	BOOST_CHECK(get_result(var_pp_path_local, L"path", L"\\doc", true)); 
}

void nf::ut::test_ExtractPrefix()
{
	wchar_t const* cmd1 = L"nf:";
	wchar_t const* cmd2 = L"nf:abc";
	wchar_t const* cmd3 = L"";
	wchar_t const* cmd4 = L"::";
	wchar_t const* cmd5 = L":::";

	BOOST_CHECK(nf::Parser::ExtractPrefix(cmd1) == L"nf:"); 
	BOOST_CHECK(nf::Parser::ExtractPrefix(cmd2) == L"nf:"); 
	BOOST_CHECK(nf::Parser::ExtractPrefix(cmd3) == L""); 
	BOOST_CHECK(nf::Parser::ExtractPrefix(cmd4) == L"::"); 
	BOOST_CHECK(nf::Parser::ExtractPrefix(cmd5) == L"::"); 
}


bool GetCommandKind(wchar_t const* SrcCmd, nf::tcommands_kinds &Kind)
{
	nf::tparsed_command cmd;
	bool bRet = nf::Parser::ParseString(SrcCmd, cmd); 
	Kind = cmd.kind;
	return bRet;
}
void nf::ut::test_ImplicitCommands()
{
	wchar_t const* cmd1 = L"cd::";
	wchar_t const* cmd2 = L"cd:-";
	wchar_t const* cmd3 = L"cd:+";
	wchar_t const* cmd11 = L"cd::ttk/";
	wchar_t const* cmd12 = L"cd:-ttk/";
	wchar_t const* cmd13 = L"cd:+ttk/";

	nf::tcommands_kinds kind;
//неявные команды без каталога
	BOOST_CHECK(GetCommandKind(cmd11, kind)); 
	BOOST_CHECK(kind == nf::QK_INSERT_SHORTCUT_IMPLICIT); 

	BOOST_CHECK(GetCommandKind(cmd12, kind)); 
	BOOST_CHECK(kind == nf::QK_DELETE_SHORTCUT_IMPLICIT); 

	BOOST_CHECK(GetCommandKind(cmd13, kind)); 
	BOOST_CHECK(kind == nf::QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT); 

//неявные команды с каталогом
	BOOST_CHECK(GetCommandKind(cmd1, kind)); 
	BOOST_CHECK(kind == nf::QK_INSERT_SHORTCUT_IMPLICIT); 

	BOOST_CHECK(GetCommandKind(cmd2, kind)); 
	BOOST_CHECK(kind == nf::QK_DELETE_SHORTCUT_IMPLICIT); 

	BOOST_CHECK(GetCommandKind(cmd3, kind)); 
	BOOST_CHECK(kind == nf::QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT); 
}
