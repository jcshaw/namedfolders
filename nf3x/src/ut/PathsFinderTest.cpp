#include "stdafx.h"
#include "PathsFinderTest.h"

#include <boost/test/unit_test.hpp>
#include <iostream>

#include "PathsFinder.h"

using boost::unit_test::test_suite;


typedef struct {
	wchar_t const* SrcPath;
	int CountResults;
} test_t;

void nf::ut::test_PathsFinder_with_shortsyntax() {
	wchar_t const* root_folder = L"Z:\\projects\\GoogleCode\\namedfolders\\ut-data";
	test_t tests[] = {	
		{L"\\a1\\..a", 1}
		, {L"\\*\\*\\..*:2", 5} //директории всего 5, всего результатов 18
		, {L"\\a1\\..b", 0}
		, {L"\\a1\\..", 1}
		, {L"\\a1\\..\\*", 3 + 1} //.svn
		, {L"\\a1\\*\\..1", 1} //директория одна, но возвращается три раза
		, {L"\\a1\\..\\b\\b", 3}
		, {L"\\..$", 0} //символ которого заведомо нет в названиях директорий, расположенных выше ut-data
		, {L"\\..*", 4}

		, {L"\\**:1\\a", 3} //2 добавляются из-за .svn
		, {L"\\*\\*", 9}
		, {L"\\*\\*\\*", 0}
		, {L"\\a1", 1}
		, {L"\\*\\a", 1}
		, {L"\\**", 12}
	};
	unsigned int const count_tests = sizeof(tests) / sizeof(wchar_t const*[3]);


	tasterix_mode asterix_mode = ASTERIX_MODE_BOTH;
	bool ballow_short_syntax = true;

	nf::Search::CSearchEngine engine(nf::WTS_DIRECTORIES, true);
	nf::Search::PathsFinder f(engine, ballow_short_syntax, asterix_mode);

	for (unsigned int i = 0; i < count_tests; ++i) {
		nf::tlist_strings dest;
		f.SearchByPattern(Utils::SubstituteSearchMetachars(tests[i].SrcPath, ballow_short_syntax)
			, root_folder, dest);

		BOOST_CHECK(tests[i].CountResults == dest.size()); 
		if (tests[i].CountResults != dest.size()) {
			std::wcout << dest.size() << L" != " <<  tests[i].CountResults << std::endl;
		}
	}

}