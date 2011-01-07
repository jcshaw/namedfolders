// nfut.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "CommandPatternsTest.h"
#include "ParserTest.h"
#include "UtilsTest.h"

#include <boost/test/unit_test.hpp>
using boost::unit_test::test_suite;

struct PluginStartupInfo g_PluginInfo; //emulation
struct FarStandardFunctions g_FSF; //emulation

wchar_t __stdcall  ToLower(wchar_t UpperChar) {	
	return ::towlower(UpperChar);
}

//////////////////////////////////////////////////////////////////////////
test_suite*
init_unit_test_suite( int, char* [] ) {
	test_suite* test= BOOST_TEST_SUITE( "NF" );

	g_FSF.LLower = &::ToLower;

	test->add( BOOST_TEST_CASE( &nf::ut::test_replace_sequence_chars));
	
	test->add( BOOST_TEST_CASE( &nf::ut::test_GetCanonicalCatalogName));
	test->add( BOOST_TEST_CASE( &nf::ut::test_ExpandCatalogPath));
	test->add( BOOST_TEST_CASE( &nf::ut::test_PrepareMovingShortcut));
	
// 	test->add( BOOST_TEST_CASE( &nf::ut::test_DetailedCommand ));
// 	test->add( BOOST_TEST_CASE( &nf::ut::test_TransformCommand ));

	test->add( BOOST_TEST_CASE( &nf::ut::test_ImplicitCommands));
	test->add( BOOST_TEST_CASE( &nf::ut::test_ExtractPrefix));
	test->add( BOOST_TEST_CASE( &nf::ut::test_ParseEnvVarPath ));

	return test;
}
