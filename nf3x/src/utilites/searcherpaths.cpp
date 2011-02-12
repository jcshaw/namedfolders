/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/ 
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "searcherpaths.h"

#include <list>
#include <functional>
#include <boost/bind.hpp>
#include <algorithm>
#include <cmath>
#include "stlsoft_def.h"

#include "strings_utils.h"
#include "settings.h"

#include "Parser.h"
#include "PathsFinder.h"

using namespace nf;

namespace {
	inline WinSTL::findfile_sequence_t::flags_type get_search_flags(nf::twhat_to_search_t whatToSearch) {
		return WTS_DIRECTORIES == whatToSearch
			? WinSTL::findfile_sequence_t::directories
			: (WTS_FILES == whatToSearch 
			? WinSTL::findfile_sequence_t::files
			: WinSTL::findfile_sequence_t::files | WinSTL::findfile_sequence_t::directories); //=WTS_DIRECTORIES_AND_FILES
	}
}

nf::Search::MaskMatcher::MaskMatcher(tstring const& srcMask, tasterix_mode const asterixMode012, bool bPositiveOR) 
: m_bPositiveOR(bPositiveOR)
{
	tpair_strings kvp = Utils::DivideString(srcMask, L'|');
	add_masks(kvp.first, asterixMode012, m_regexPositive);
	if (! kvp.second.empty()) {
		add_masks(kvp.second, asterixMode012, m_regexNegative);
	}
}

nf::Search::MaskMatcher::MaskMatcher(tlist_strings const& positiveMasks, tlist_strings const& negativeMasks
									 , nf::tasterix_mode const asterixMode012, bool bPositiveOR) 
: m_bPositiveOR(bPositiveOR)
{
	BOOST_FOREACH(tstring const& positive_mask, positiveMasks) {
		add_masks(positive_mask, asterixMode012, m_regexPositive);
	}
	BOOST_FOREACH(tstring const& negative_mask, negativeMasks) {
		add_masks(negative_mask, asterixMode012, m_regexNegative);
	}
}

void nf::Search::MaskMatcher::add_masks(tstring const& srcMask, tasterix_mode const asterixMode012, std::vector<tregex>& destList) {
	destList.push_back(tregex(NF_BOOST_REGEX_COMPILE(Parser::ConvertMaskToReqex(
		nf::Parser::ConvertToMask(Utils::TrimChar(srcMask, L' '), asterixMode012)
		)), boost::regex_constants::icase));
}

bool nf::Search::MaskMatcher::match_to(tstring const& srcText, std::vector<tregex> const& destList, bool bLogicalOR) {
	tsmatch what;
	if (bLogicalOR) {
		BOOST_FOREACH(tregex const& regex, destList) {
			if (NF_BOOST_REGEX_LIB::regex_match(srcText, what, regex)) return true;
		}
		return false;
	} else {
		BOOST_FOREACH(tregex const& regex, destList) {
			if (! NF_BOOST_REGEX_LIB::regex_match(srcText, what, regex)) return false;
		}
		return true;
	}
}


void nf::Search::CSearchEngine::SearchItems(tstring const& rootDir0, MaskMatcher const& maskMatcher, nf::tlist_strings &destList, twhat_to_search_t whatToSearch) const {
	try {
		tstring RootDir = rootDir0;
		if (Utils::IsAliasIsDisk(RootDir)) RootDir += L"\\";

		WinSTL::findfile_sequence_t f(RootDir.c_str(), L"*", get_search_flags(whatToSearch));
		BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
			tstring filename = t.get_filename();
			if (maskMatcher.MatchTo(filename)) {
				destList.push_back(t.get_path());
			}
		}
	} catch (...) {
		SearchBySystemFunctions(rootDir0, maskMatcher, destList, whatToSearch);
	}
}

void nf::Search::CSearchEngine::SearchBySystemFunctions(tstring const& RootDir0, MaskMatcher const& maskMatcher, nf::tlist_strings &destList
														, twhat_to_search_t whatToSearch) const {	
	//если в качестве RootDir передается z: то поиск не срабатывает
	//проверяем и добавляем слеш
	tstring RootDir = RootDir0;
	if (Utils::IsAliasIsDisk(RootDir)) RootDir += L"\\";

	WinSTL::findfile_sequence_t f(RootDir.c_str(), L"*", get_search_flags(whatToSearch));
	BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
		if (! maskMatcher.MatchTo(t.get_filename())) continue;
		tstring full_filename = t.get_path(); //Utils::CombinePath(RootDir0, t.get_filename(), SLASH_DIRS);
		destList.push_back(full_filename);
	}
}

bool nf::Search::SearchByPattern(tstring const& Pattern, tstring const &RootDir
								 , CSearchEngine &searchPolice
								 , tasterix_mode AsterixMaskMode012
								 , nf::tlist_strings& dest) {	
	PathsFinder finder(searchPolice
		, nf::CSettings::GetInstance().GetValue(nf::ST_ALLOW_ABBREVIATED_SYNTAX_FOR_DEEP_SEARCH) != 0
		, AsterixMaskMode012);
	return finder.SearchByPattern(Pattern, RootDir, dest);
}

bool nf::Search::SearchMatched(tstring const& srcPathPattern, CSearchEngine &searchPolice, tasterix_mode AsterixMaskMode012, nf::tlist_strings& dest)
{	//найти все директории, удовлетворяющие паттерну
	//PathPattern должен содержать полный локальный путь вида
	//C:\path1\path2\...\pathN
	//причем C - любая одна буква или один из метасимволов ?, * или [a-z]
	//В токенах pathXXX могут встречаться метасимволы ?, * или [a,b-z]

	tstring path_pattern = srcPathPattern;
	//определяем букву диска
	wchar_t const* DISK_PATH_DELIMETER = L":\\";
	if (path_pattern.size() < 2) {
		dest.push_back(path_pattern);
		return false;	//как минимум "c:"
	}
	size_t column_pos = path_pattern.find(DISK_PATH_DELIMETER);
	if (column_pos == tstring::npos) {
		dest.push_back(path_pattern);
		return false;
	}
	tstring disk_pattern(path_pattern, 0, column_pos);
	path_pattern.erase(0, disk_pattern.size() + 1); //удаляем имя диска (или маску) и ":", оставляем только путь, начинающийся с 1 слеша

	if (! disk_pattern.size()) {
		dest.push_back(path_pattern);
		return false;
	}

	if (disk_pattern.size() == 1 &&  disk_pattern[0] != L'*' &&  disk_pattern[0] != L'?') {	//диск указан явно
		if (! path_pattern.size()) {	//указан путь типа "c:"
			dest.push_back(path_pattern);
			return true;
		}
		return SearchByPattern(path_pattern.c_str(), disk_pattern + L":", searchPolice, AsterixMaskMode012, dest);
	} else {
		//вместо буквы диска указан метасимвол
		//перебираем все диски в системе, вызываем поиск
		//директории для тех, которые удовлетворяют паттерну
		DWORD drives_bitmask = GetLogicalDrives();
		for (int ibit = 2; ibit< 26; ++ibit) {
			if (! (drives_bitmask & (1 << ibit))) continue;

			wchar_t letter[2] = { wchar_t(L'A' + ibit), 0};
			tstring driver_letter = &letter[0];
			tstring drive_name = driver_letter + L":\\";

			if (GetDriveType(drive_name.c_str()) != DRIVE_FIXED)  
				continue;	//работаем только с hdd

			tstring drive_name_short = driver_letter + L":";
			if (nf::Parser::IsTokenMatchedToPattern(driver_letter.c_str(), disk_pattern, false)) {
				SearchByPattern(path_pattern, drive_name_short, searchPolice, AsterixMaskMode012, dest);		
			}
		} //for
	}
	return true;
}