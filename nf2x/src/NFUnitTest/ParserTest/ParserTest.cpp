// ParserTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "./utilites/Parser.h"
#include "./utilites/listpaths.h"

#include <boost/test/unit_test.hpp>

using boost::unit_test::test_suite;


//////////////////////////////////////////////////////////////////////////
namespace env {

	bool get_result(tstring const& Src, tstring const& VarName, tstring const& LocalPath, bool bIncomplete)
	{
		tstring varname;
		tstring localpath;
		bool b = nf::Parser::ParseEnvVarPath(Src, varname, localpath);
		bool bResult = (VarName == varname) && (LocalPath == localpath) && (b == bIncomplete);
		return bResult;
	}

	void test_ParseEnvVarPath()
	{
		tstring var_path_p = _T("path%");
		tstring var_path = _T("path");
		tstring var_pp_path = _T("%path%");
		tstring var_p_path = _T("%path");
		tstring var_pp_path_local = _T("%path%\\doc");
		
		BOOST_CHECK(get_result(var_path_p, "path", "", true)); 
		BOOST_CHECK(get_result(var_path, "path", "", false)); 
		BOOST_CHECK(get_result(var_pp_path, "path", "", true)); 
		BOOST_CHECK(get_result(var_p_path, "path", "", false)); 
		BOOST_CHECK(get_result(var_pp_path_local, "path", "\\doc", true)); 
	}

	void test_GetListEncodedPaths()
	{
		std::list<tstring> list_paths;
		nf::Env::GetListEncodedPaths("path", "", list_paths);
		BOOST_CHECK(list_paths.size() != 0);

		list_paths.clear();
		nf::Env::GetListEncodedPaths("32", "", list_paths);
		BOOST_CHECK(list_paths.size() != 0);
	}

	void test_ExtractPrefix()
	{
		TCHAR const* cmd1 = "nf:";
		TCHAR const* cmd2 = "nf:abc";
		TCHAR const* cmd3 = "";
		TCHAR const* cmd4 = "::";
		TCHAR const* cmd5 = ":::";

		BOOST_CHECK(nf::Parser::ExtractPrefix(cmd1) == "nf:"); 
		BOOST_CHECK(nf::Parser::ExtractPrefix(cmd2) == "nf:"); 
		BOOST_CHECK(nf::Parser::ExtractPrefix(cmd3) == ""); 
		BOOST_CHECK(nf::Parser::ExtractPrefix(cmd4) == "::"); 
		BOOST_CHECK(nf::Parser::ExtractPrefix(cmd5) == "::"); 
	}

} //env


bool GetCommandKind(TCHAR const* SrcCmd, kr::tcommands_kinds &Kind)
{
	nf::Parser::CParser parser;
	kr::tparsed_command cmd;
	bool bRet = parser.ParseString(SrcCmd, cmd); 
	Kind = cmd.kind;
	return bRet;
}
void test_ImplicitCommands()
{
	TCHAR const* cmd1 = "cd::";
	TCHAR const* cmd2 = "cd:-";
	TCHAR const* cmd3 = "cd:+";
	TCHAR const* cmd11 = "cd::tvk/";
	TCHAR const* cmd12 = "cd:-tvk/";
	TCHAR const* cmd13 = "cd:+tvk/";

	kr::tcommands_kinds kind;
//неявные команды без каталога
	BOOST_CHECK(GetCommandKind(cmd11, kind)); 
	BOOST_CHECK(kind == kr::QK_INSERT_SHORTCUT_IMPLICIT); 

	BOOST_CHECK(GetCommandKind(cmd12, kind)); 
	BOOST_CHECK(kind == kr::QK_DELETE_SHORTCUT_IMPLICIT); 

	BOOST_CHECK(GetCommandKind(cmd13, kind)); 
	BOOST_CHECK(kind == kr::QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT); 

//неявные команды с каталогом
	BOOST_CHECK(GetCommandKind(cmd1, kind)); 
	BOOST_CHECK(kind == kr::QK_INSERT_SHORTCUT_IMPLICIT); 

	BOOST_CHECK(GetCommandKind(cmd2, kind)); 
	BOOST_CHECK(kind == kr::QK_DELETE_SHORTCUT_IMPLICIT); 

	BOOST_CHECK(GetCommandKind(cmd3, kind)); 
	BOOST_CHECK(kind == kr::QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT); 
}


//////////////////////////////////////////////////////////////////////////
test_suite*
init_unit_test_suite( int, char* [] ) {
	test_suite* test= BOOST_TEST_SUITE( "Parser" );

	// this example will pass cause we know ahead of time number of expected failures
	test->add( BOOST_TEST_CASE( &test_ImplicitCommands));
	test->add( BOOST_TEST_CASE( &env::test_ExtractPrefix));
	test->add( BOOST_TEST_CASE( &env::test_ParseEnvVarPath ));
	test->add( BOOST_TEST_CASE( &env::test_GetListEncodedPaths ));

	return test;
}


