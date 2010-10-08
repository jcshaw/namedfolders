#include "StdAfx.h"
#include "UtilsTest.h"

#include "strings_utils.h"
#include <boost/test/unit_test.hpp>

using boost::unit_test::test_suite;


void nf::ut::test_ExpandCatalogPath() {
	//раскрыть ".." в TargetCatalog
	//"1/2/3/" ".." -> "1/2/../3" = "1/3"
	//"1/2/3/4/" "../../7" -> "1/2/3/../../7"="1/7"
	//"1/2/3/4/" "../../7" -> "1/2/3/../../7/4"="1/7/4"
	//"1/2/3/" "/5/6" -> "/5/6"
	//"1/2/3/" "/5/6/" -> /5/6/3"
	//"1/2/3/" "5/6" -> 1/2/5/6
	//"1/2/3/" "5/6/" -> 1/2/5/6/3
	unsigned int const count_tests = 7;
	wchar_t const* tests[count_tests][4] = {
		{L"/1/2/3/", L"..", L"1/2/../3", L"/1/3"}
		, {L"1/2/3/4/", L"../../7", L"1/2/3/../../7", L"/1/7"}
		, {L"1/2/3/4/", L"../../7/", L"1/2/3/../../7/4", L"/1/7/4"}
		, {L"1/2/3/", L"/5/6", L"/5/6", L"/5/6"}
		, {L"1/2/3/", L"/5/6/", L"/5/6/3", L"/5/6/3"}
		, {L"1/2/3/", L"5/6", L"1/2/5/6", L"/1/2/5/6"}
		, {L"1/2/3/", L"5/6/", L"1/2/5/6/3", L"/1/2/5/6/3"}
	};

	for (unsigned int i = 0; i < count_tests; ++i) {
		tstring dest;
		Utils::ExpandCatalogPath(tests[i][0], tests[i][1], dest);

		BOOST_CHECK(dest == tests[i][3]); 
	}
}