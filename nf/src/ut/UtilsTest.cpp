#include "StdAfx.h"
#include "UtilsTest.h"

#include "strings_utils.h"
#include <boost/test/unit_test.hpp>
#include "catalog_names.h"

using boost::unit_test::test_suite;


void nf::ut::test_ExpandCatalogPath() {
	unsigned int const count_tests = 11;
	wchar_t const* tests[count_tests][3] = {	
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

	for (unsigned int i = 0; i < count_tests; ++i) {
		tstring dest;
		Utils::ExpandCatalogPath(tests[i][0], tests[i][1], dest);

		BOOST_CHECK(dest == tests[i][2]); 
	}
}