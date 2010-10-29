// CommandPatternsTest.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <boost/test/unit_test.hpp>

#include "utilites/CommandPatterns.h"
#include "utilites/Parser.h"


using boost::unit_test::test_suite;

char const* FAR_KEY_CP = "Software\\Far\\Plugins\\NamedFolders\\CommandPatterns";

char const* test_cmd_nf_src = "nf:src";
char const* test_cmd_nf_result = "cd:nf\\src";
char const* test_cmd_nf_pattern = "[name]=[prefix]\\[name]";

char const* test_cmd_r2_src = "nfr2:src";
char const* test_cmd_r2_result = "uu:src\\abc";
char const* test_cmd_r2_pattern = "[path]=abc;[prefix]=uu:";

char const* test_cmd_recursion_src = "nfrec:src";
char const* test_cmd_recursion_result = "cd:nf\\src\\abc";
char const* test_cmd_recursion_pattern = "[path]=abc;[prefix]=nf:";

char const* cmd_search_src = "cf:src";
char const* cmd_search_result = "cd:nf\\src\\abc";
char const* cmd_search_pattern = "[path]=abc;[prefix]=nf:";


void test_DetailedCommand()
{
	kr::tparsed_command cmd;
	nf::Parser::CParser p;
	p.ParseString(test_cmd_nf_src, cmd);

	nf::Patterns::Private::DetailedCommand dc(cmd);
	dc.ApplyPattern(test_cmd_nf_pattern);
	tstring result_string = dc.GetResultString();
	BOOST_CHECK(result_string == tstring(test_cmd_nf_result));
}

void test_TransformCommand()
{
	nf::Patterns::CommandsManager cm(FAR_KEY_CP);
	nf::Patterns::tlist_command_patterns list;
	tstring destcmd;

	cm.SetCommand("zp:", test_cmd_nf_pattern);

	BOOST_CHECK(cm.SetCommand("nf:", test_cmd_nf_pattern));
	BOOST_CHECK(cm.SetCommand("nfr2:", test_cmd_r2_pattern));
	BOOST_CHECK(cm.SetCommand("nfrec:", test_cmd_recursion_pattern));

//рекурсивная раскрутка
	cm.TransformCommandRecursively(test_cmd_recursion_src, destcmd);
	BOOST_CHECK(destcmd == test_cmd_recursion_result);

//разделитель между псевдонимом и путем автоматом
	cm.TransformCommandRecursively(test_cmd_r2_src, destcmd);
	BOOST_CHECK(destcmd == test_cmd_r2_result);

//простой тест
	cm.TransformCommandRecursively(test_cmd_nf_src, destcmd);
	BOOST_CHECK(destcmd == test_cmd_nf_result);

	BOOST_CHECK(cm.RemoveCommand("nf:"));
	BOOST_CHECK(cm.RemoveCommand("nfr2:"));
	BOOST_CHECK(cm.RemoveCommand("nfrec:"));
}

//////////////////////////////////////////////////////////////////////////
test_suite*
init_unit_test_suite( int, char* [] ) {
	test_suite* test= BOOST_TEST_SUITE( "Parser" );

	// this example will pass cause we know ahead of time number of expected failures
	test->add( BOOST_TEST_CASE( &test_DetailedCommand ));
	test->add( BOOST_TEST_CASE( &test_TransformCommand ));

	return test;
}
