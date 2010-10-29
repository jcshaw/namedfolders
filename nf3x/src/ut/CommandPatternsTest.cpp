// CommandPatternsTest.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "CommandPatternsTest.h"
#include <boost/test/unit_test.hpp>

#include "utilites/CommandPatterns.h"
#include "utilites/Parser.h"


using boost::unit_test::test_suite;

wchar_t const* FAR_KEY_CP = L"Software\\Far\\Plugins\\NamedFolders\\CommandPatterns";

wchar_t const* test_cmd_nf_src = L"nf:src";
wchar_t const* test_cmd_nf_result = L"cd:nf\\src";
wchar_t const* test_cmd_nf_pattern = L"[name]=[prefix]\\[name]";

wchar_t const* test_cmd_r2_src = L"nfr2:src";
wchar_t const* test_cmd_r2_result = L"uu:src\\abc";
wchar_t const* test_cmd_r2_pattern = L"[path]=abc;[prefix]=uu:";

wchar_t const* test_cmd_recursion_src = L"nfrec:src";
wchar_t const* test_cmd_recursion_result = L"cd:nf\\src\\abc";
wchar_t const* test_cmd_recursion_pattern = L"[path]=abc;[prefix]=nf:";

wchar_t const* cmd_search_src = L"cf:src";
wchar_t const* cmd_search_result = L"cd:nf\\src\\abc";
wchar_t const* cmd_search_pattern = L"[path]=abc;[prefix]=nf:";


void nf::ut::test_DetailedCommand()
{
	nf::tparsed_command cmd;
	nf::Parser::ParseString(test_cmd_nf_src, cmd);

	nf::Patterns::Private::DetailedCommand dc(cmd);
	dc.ApplyPattern(test_cmd_nf_pattern);
	tstring result_string = dc.GetResultString();
	BOOST_CHECK(result_string == tstring(test_cmd_nf_result));
}

void nf::ut::test_TransformCommand()
{
	nf::Patterns::CommandsManager cm(FAR_KEY_CP);
	nf::Patterns::tlist_command_patterns list;
	tstring destcmd;

	cm.SetCommand(L"zp:", test_cmd_nf_pattern);

	BOOST_CHECK(cm.SetCommand(L"nf:", test_cmd_nf_pattern));
	BOOST_CHECK(cm.SetCommand(L"nfr2:", test_cmd_r2_pattern));
	BOOST_CHECK(cm.SetCommand(L"nfrec:", test_cmd_recursion_pattern));

//рекурсивная раскрутка
	cm.TransformCommandRecursively(test_cmd_recursion_src, destcmd);
	BOOST_CHECK(destcmd == test_cmd_recursion_result);

//разделитель между псевдонимом и путем автоматом
	cm.TransformCommandRecursively(test_cmd_r2_src, destcmd);
	BOOST_CHECK(destcmd == test_cmd_r2_result);

//простой тест
	cm.TransformCommandRecursively(test_cmd_nf_src, destcmd);
	BOOST_CHECK(destcmd == test_cmd_nf_result);

	BOOST_CHECK(cm.RemoveCommand(L"nf:"));
	BOOST_CHECK(cm.RemoveCommand(L"nfr2:"));
	BOOST_CHECK(cm.RemoveCommand(L"nfrec:"));
}

