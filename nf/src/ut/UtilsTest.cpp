#include "StdAfx.h"
#include "UtilsTest.h"

#include "strings_utils.h"
#include <boost/test/unit_test.hpp>
#include "catalog_names.h"

using boost::unit_test::test_suite;


void nf::ut::test_ExpandCatalogPath() {
	wchar_t const* tests[][3] = {	
		{L"a/b", L"..", L"/b"}
		, {L"a/b/c", L"..", L"/a/c"}
		, {L"a/b", L"../d", L"/d"}
		, {L"a/b", L"c/d", L"/a/c/d"}
		, {L"a/b", L"d", L"/a/d"}
		, {L"a/b", L"./d", L"/a/d"}
		, {L"a/b", L"../../../../../../d", L"/d"}
		, {L"a/b", L"a/b/c/d", L"/a/a/b/c/d"}
		, {L"a/b", L"a/b/c/../../../d", L"/a/d"}
		, {L"a/b", L"a/b/c/../../../d/e/../../../f", L"/f"}
		, {L"a/b", L".", L"/a/b"}
	};
	unsigned int const count_tests = sizeof(tests) / sizeof(wchar_t const*[3]);

	for (unsigned int i = 0; i < count_tests; ++i) {
		tstring dest;
		Utils::ExpandCatalogPath(tests[i][0], tests[i][1], dest);

		BOOST_CHECK(dest == tests[i][2]); 
	}
}


void nf::ut::test_PrepareMovingShortcut() {
	wchar_t const* tests[][5] = {	
		{L"/a/b", L"s1", L"/a/b/s2", L"/a/b", L"s2"}
		, {L"/a/b", L"s1", L"..", L"/a", L"s1"}
		, {L"/a/b", L"s1", L"../..", L"/", L"s1"}
		, {L"/a/b", L"s1", L"c/s2", L"/a/b/c", L"s2"}
		, {L"/a/b", L"s1", L"c/d/", L"/a/b/c/d", L"s2"}
		, {L"/a/b", L"s1", L"./s2", L"/a/b", L"s2"}
	};
	unsigned int const count_tests = sizeof(tests) / sizeof(wchar_t const*[3]);

	for (unsigned int i = 0; i < count_tests; ++i) {
		nf::tshortcut_info sh1;
		sh1.bIsTemporary = false; //this value doesn't matter here
		sh1.catalog = tests[i][0];
		sh1.catalog = tests[i][1];
		nf::tshortcut_info sh2;
		bool bret = Utils::PrepareMovingShortcut(sh1, tests[i][2], sh2);
		BOOST_CHECK(bret);
		BOOST_CHECK(sh2.catalog == tests[i][3]);
		BOOST_CHECK(sh2.catalog == tests[i][4]);
	}

}