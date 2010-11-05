// SearcherPathsTest.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "SearcherPathsTest.h"

#include <iostream>
#include "./utilites/searcherpaths.h"

#include <boost/test/unit_test.hpp>

using boost::unit_test::test_suite;

void test_CSearchFarPolice()
{

//	BOOST_CHECK(get_result(var_path_p, "path", "", true)); 
}

void test_CSearchSystemPolice()
{

	//	BOOST_CHECK(get_result(var_path_p, "path", "", true)); 
}

namespace 
{
	int count_dirs_fp(wchar_t const* dir, wchar_t const* mask)
	{
		nf::tlist_strings results;
		nf::Search::CSearchFarPolice sfp(nf::WTS_DIRECTORIES);
		nf::Search::SearchByPattern(mask, dir, sfp, results);
		int n = static_cast<int>(results.size());

		std::wcout << mask << std::endl;
		nf::tlist_strings::const_iterator p = results.begin();
		while (p != results.end()) 
		{
			std::wcout <<*p << std::endl;
			++p;
		}
		std::cout<<results.size() << std::endl << std::endl;
		return n;
	}
}
void nf::ut::test_CSearcherPaths()
{
/*	Варианты, которые должны быть учтены при тестировании
	Маски
		cd:sh\[a-z]
		cd:sh\z
		cd:sh\z*
		cd:sh\z?
		cd:sh\*z
	Направление поиска - вниз
		cd:sh\a
		cd:sh\\a
		cd:sh\a\a
	Направление поиска - вверх
		cd:sh.a
		cd:sh..z
		cd:sh.a.b
	Поиск везде
		cd:sh\*\ab
		cd:sh.*.ab
	Смешанный поиск
		cd:sh\a\b\.c
	Русские буквы
		cd:sh\г
		cd:sh\Г
*/

	wchar_t const* SH = L"_nfpolygon";
	wchar_t const* SHDIR = L"Z:\\projects\\nf2\\polygon";
	wchar_t const* SHDIR_DEEP = L"Z:\\projects\\nf2\\polygon\\a\\eeeeee\\abcdef\\";

	BOOST_CHECK(2 == count_dirs_fp(SHDIR_DEEP, L"..a")); 
	BOOST_CHECK(1 == count_dirs_fp(SHDIR_DEEP, L".a")); 

	BOOST_CHECK(6 == count_dirs_fp(SHDIR, L"\\[a-z]")); 
	BOOST_CHECK(3 == count_dirs_fp(SHDIR, L"\\z")); //чтобы нашелся вариант *z* нужно использовать другой police
	BOOST_CHECK(1 == count_dirs_fp(SHDIR, L"\\z?")); 
	BOOST_CHECK(3 == count_dirs_fp(SHDIR, L"\\z*")); 
	BOOST_CHECK(4 == count_dirs_fp(SHDIR, L"\\*z")); 

	BOOST_CHECK(3 == count_dirs_fp(SHDIR, L"\\a")); 
	BOOST_CHECK(8 == count_dirs_fp(SHDIR, L"\\\\a")); 
	BOOST_CHECK(1 == count_dirs_fp(SHDIR, L"\\a\\a")); 


}

  