// SearcherPathsTest.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"

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
	int count_dirs_fp(char const* dir, char const* mask)
	{
		nf::CSearcherPaths::tlist results;
		nf::SearchPathsPolices::CSearchFarPolice sfp(nf::WTS_DIRECTORIES);
		nf::CSearcherPaths sp(results, sfp);
		sp.SearchByPattern(mask, dir);
		int n = static_cast<int>(results.size());

		std::cout<< mask << std::endl;
		nf::CSearcherPaths::tlist::const_iterator p = results.begin();
		while (p != results.end()) 
		{
			std::cout<<*p << std::endl;
			++p;
		}
		std::cout<<results.size() << std::endl << std::endl;
		return n;
	}
}
void test_CSearcherPaths()
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

	char const* SH = "_nfpolygon";
	char const* SHDIR = "Z:\\projects\\nf2\\polygon";
	char const* SHDIR_DEEP = "Z:\\projects\\nf2\\polygon\\a\\eeeeee\\abcdef\\";

	BOOST_CHECK(2 == count_dirs_fp(SHDIR_DEEP, "..a")); 
	BOOST_CHECK(1 == count_dirs_fp(SHDIR_DEEP, ".a")); 

	BOOST_CHECK(6 == count_dirs_fp(SHDIR, "\\[a-z]")); 
	BOOST_CHECK(3 == count_dirs_fp(SHDIR, "\\z")); //чтобы нашелся вариант *z* нужно использовать другой police
	BOOST_CHECK(1 == count_dirs_fp(SHDIR, "\\z?")); 
	BOOST_CHECK(3 == count_dirs_fp(SHDIR, "\\z*")); 
	BOOST_CHECK(4 == count_dirs_fp(SHDIR, "\\*z")); 

	BOOST_CHECK(3 == count_dirs_fp(SHDIR, "\\a")); 
	BOOST_CHECK(8 == count_dirs_fp(SHDIR, "\\\\a")); 
	BOOST_CHECK(1 == count_dirs_fp(SHDIR, "\\a\\a")); 


}


//////////////////////////////////////////////////////////////////////////
test_suite*
init_unit_test_suite( int, char* [] ) {
	test_suite* test= BOOST_TEST_SUITE( "SearchPaths" );

	// this example will pass cause we know ahead of time number of expected failures
//	test->add( BOOST_TEST_CASE( &test_CSearcherPaths ));

	return test;
}