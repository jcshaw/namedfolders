/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/ 
* e-mail: dvpublic0@gmail.com
*/

#include "StdAfx.h"
#include "PathsFinder.h"

#include <list>
#include <functional>
#include <boost/bind.hpp>
#include <Shlwapi.h>
#include <algorithm>
#include <cmath>
#include "stlsoft_def.h"

#include "strings_utils.h"
#include "settings.h"

#include "Parser.h"

namespace {
	inline bool is_slash(wchar_t ch) { 
		return SLASH_DIRS_CHAR == ch 
			|| MC_SEARCH_BACKWORD_SHORT[0] == ch 
			|| MC_DEEP_DIRECT_SEARCH_SHORT[0] == ch 
			|| MC_DEEP_REVERSE_SEARCH_SHORT[0] == ch
			|| MC_SEARCH_FORWARD_SHORT[0] == ch
			|| MC_SEARCH_BACKWORD_SHORT_WITHOUT_SLASH[0] == ch;
	}
	typedef enum tsearch_ch_kind {
		ID_SEARCH_FORWARD = L'f'
		, ID_SEARCH_BACKWORD = L'b'
	};
	const int MAX_DEEP_LEVEL = 9999;

	//” каждого метасимвола есть две характеристики: направление и глубина поиска.
	//—имвол "\*" имеет направление "вниз", символ "\.." - вверх.
	//√лубина поиска может быть неограниченной или ограниченной.
	//” "\" глубина равна 1, у "\**" - 256 (неограничена), у "\**4" - 4,  у "\\\\" - либо 1 либо 4 в зависимости от настроек.
	//√лубина поиска не может быть больше MAX_DEEP_SEARCH - если указана больша€ глубина, то поиск считаетс€ неограниченным.
	inline tsearch_ch_kind get_metachar_kind(tstring const& metaChar) {
		assert(metaChar.size() == 2);
		return static_cast<int>(metaChar[0]) == ID_SEARCH_BACKWORD
			? ID_SEARCH_BACKWORD
			: ID_SEARCH_FORWARD;			
	}
	inline int get_metachar_deep(tstring const& metaChar) {
		assert(metaChar.size() == 2);
		return static_cast<int>(metaChar[1]);
	}
	tstring create_metachar(tsearch_ch_kind slashKind, int deepSearch) {
		tstring dest;
		dest.resize(2);
		dest[0] = static_cast<wchar_t>(slashKind);
		dest[1] = static_cast<wchar_t>(deepSearch);
		return dest;
	}
}

bool check_for_esc(void) {
	bool bEsc = false;
	INPUT_RECORD rec;
	static HANDLE hConInp = GetStdHandle(STD_INPUT_HANDLE);
	DWORD read_count;
	while (1) {
		PeekConsoleInput(hConInp, &rec, 1, &read_count);
		if (read_count==0) break;
		ReadConsoleInput(hConInp, &rec, 1, &read_count);
		if (rec.EventType==KEY_EVENT)
			if (rec.Event.KeyEvent.wVirtualKeyCode==VK_ESCAPE &&
				rec.Event.KeyEvent.bKeyDown) bEsc = true;
	}
	return (bEsc);
}


nf::Search::PathsFinder::PathsFinder(CSearchEngine &searchPolice, bool bShortSyntaxInPathAllowed, tasterix_mode asterixMode012)
: m_SearchPolice(searchPolice)
, ShortSyntaxInPathAllowed(bShortSyntaxInPathAllowed) 
, AsterixMode012(asterixMode012)
{
}

nf::Search::PathsFinder::~PathsFinder(void)
{
}

wchar_t const* nf::Search::PathsFinder::extract_metachar(wchar_t const* srcPattern, tpair_strings& nameMetachar) {
	//извлечь из srcPattern первый метасимвол и, если есть, первое им€; вернуть остаток
	//повтор€ющиес€ метасимволы \\\ и ... объединить в один с фиксированной глубиной поиска
	wchar_t const* ps = srcPattern;
//определ€ем метасимвол
	if (*ps == L'\\' || *ps == MC_SEARCH_BACKWORD_SHORT_WITHOUT_SLASH[0]) { 
		int deep = 1;
		wchar_t ch = *ps++;
		while (*ps == ch) {
			++ps; 
			if (this->ShortSyntaxInPathAllowed) ++deep;
		}
		--ps;
		nameMetachar.second = create_metachar(ch == L'\\' ? ID_SEARCH_FORWARD : ID_SEARCH_BACKWORD, deep);
	} else if (*ps == MC_SEARCH_FORWARD_SHORT[0]) {
		nameMetachar.second = create_metachar(ID_SEARCH_FORWARD, 1); // "\*"
	} else if (*ps == MC_SEARCH_BACKWORD_SHORT[0]) {
		nameMetachar.second = create_metachar(ID_SEARCH_BACKWORD, 1); // "\.."
	} else if (MC_DEEP_DIRECT_SEARCH_SHORT[0] == *ps) {
		nameMetachar.second = create_metachar(ID_SEARCH_FORWARD, 0);
	} else if (MC_DEEP_REVERSE_SEARCH_SHORT[0] == *ps) {
		nameMetachar.second = create_metachar(ID_SEARCH_BACKWORD, 0);
	} else {
		nameMetachar.second = create_metachar(ID_SEARCH_FORWARD, 1);
	}
	++ps;

//находим им€
	wchar_t const* psend = ps;
	while ((*psend) && (! is_slash(*psend))) ++psend;

	if (psend != ps) {
		nameMetachar.first.assign(ps, psend - ps);
	} else {
		nameMetachar.first = L"*";
	}

//если поиск неограничен по глубине и им€ начинаетс€ на ":" - то это не им€, а ограничение поиска: \**:2
	int d = get_metachar_deep(nameMetachar.second);
	if (get_metachar_deep(nameMetachar.second) == 0) {
		unsigned int deep = Parser::ExtractDeepOfSearch(nameMetachar.first);
		if (deep != 0) {
			nameMetachar.first = L"*";
			nameMetachar.second = create_metachar(get_metachar_kind(nameMetachar.second), deep);
		}
	}
	return psend;
}

bool nf::Search::PathsFinder::SearchByPattern(tstring const& Pattern, tstring const &RootDir, nf::tlist_strings& dest) {
	nf::tlist_pairs_strings list; //список метасимволы-им€

	wchar_t const* p = Pattern.c_str();
	while (*p != 0) {
		tpair_strings name_metachar;
		p = extract_metachar(p, name_metachar);
		list.push_back(name_metachar);
	} 

	nf::tlist_strings results;
	bool bret = search(list.begin(), list, RootDir, results);
	dest.swap(results);

//сортируем результаты и отбрасываем повторени€
	dest.sort(Utils::CmpStringLessCI());
	dest.unique();

	return bret;
}

bool nf::Search::PathsFinder::search(nf::tlist_pairs_strings::const_iterator listItemsPos, nf::tlist_pairs_strings  const& listItems
									 , tstring const &rootDir, nf::tlist_strings& dest) {
	if (listItemsPos == listItems.end()) return true; 

	nf::tlist_strings list;
//находим все директории, в которых нужно вести поиск остальных элементов 
//если мы ищем файлы, то на последнем элементе в list попадут файлы, а не директории
	if (! deep_search(*listItemsPos, rootDir, list)) {
		std::copy(list.begin(), list.end(), std::insert_iterator<nf::tlist_strings >(dest, dest.begin()));
		return false;
	}

	nf::tlist_pairs_strings::const_iterator pnext = listItemsPos;
	++pnext;
	if (pnext == listItems.end()) {
		if (m_SearchPolice.GetWhatToSearch() != WTS_FILES) {
			std::copy(list.begin(), list.end(), std::insert_iterator<nf::tlist_strings >(dest, dest.begin()));
		} else {
			//копируем только файлы, директории игнорируем
			BOOST_FOREACH(tstring const& sfile, list) {
				if (! ::PathIsDirectory(sfile.c_str())) {
					dest.push_back(sfile);
				}
			}
		}
	} else { //продолжаем поиск остальных элементов относительно найденных директорий
		nf::tlist_pairs_strings::const_iterator pnext_next = pnext;
		++pnext_next;
		BOOST_FOREACH(tstring const& dir, list) {
			if (! search(pnext, listItems, dir, dest)) return false;
		}	
		if (m_SearchPolice.GetWhatToSearch() != WTS_DIRECTORIES && pnext_next == listItems.end()) { //помещаем найденные файлы в результаты
			MaskMatcher mm(pnext->first, this->AsterixMode012);
			BOOST_FOREACH(tstring const& dir, list) {
				if (! ::PathIsDirectory(dir.c_str())) {
					if (mm.MatchTo(dir.c_str())) {	//if (Parser::IsTokenMatchedToPattern(::PathFindFileName(dir.c_str()), mask)) {
						dest.push_back(dir);
					}
				}
			}
		}
	}
	return true;
}

bool nf::Search::PathsFinder::deep_search(tpair_strings nameMetachar, tstring const& rootDir, nf::tlist_strings &dest) {
	//при поиске на N уровней вглубь/вверх
	//1)находим все файлы/директории наход€щиес€ на данном уровне и удовлетвор€ющие name и заносим их в результаты
	//2)находим все директории на каждом уровне и затем ищем в них дальше 
	unsigned int deep = get_metachar_deep(nameMetachar.second);
	if (deep == 0) deep = MAX_DEEP_LEVEL; //вр€д ли уровень вложенности директорий будет больше..

	MaskMatcher name_mask(nameMetachar.first, this->AsterixMode012);
	static MaskMatcher mm_all(L"*");
	
	nf::tlist_strings dirs; 
	if (get_metachar_kind(nameMetachar.second) == ID_SEARCH_FORWARD) {
		dirs.push_back(rootDir);
		for (unsigned int i = 0; i < deep; ++i) {		
			nf::tlist_strings level_dirs;
			BOOST_FOREACH(tstring const& sdir, dirs) {
				if (check_for_esc()) {
					dirs.swap(level_dirs);
					return false;	//exit by esc
				}
				if (i != deep - 1) {
					m_SearchPolice.SearchItems(sdir, mm_all, level_dirs, WTS_DIRECTORIES);
				}
				m_SearchPolice.SearchItems(sdir, name_mask, dest, m_SearchPolice.GetWhatToSearch() == WTS_DIRECTORIES 
					? nf::WTS_DIRECTORIES
					: nf::WTS_DIRECTORIES_AND_FILES);
			}
			if (level_dirs.size() == 0) break; //нет больше вложенных директорий
			dirs.swap(level_dirs);
		}		
	} else {
		tstring parent = rootDir;
		for (unsigned int i = 0; i < deep; ++i) {
			if (check_for_esc()) return false;	//exit by esc
			//переходим в директорию выше и провер€ем, есть ли в ней директории, которые удовлетвор€ют name		
			parent = Utils::RemoveTrailingChars(Utils::ExtractParentDirectory(parent), SLASH_DIRS_CHAR);
			if (parent.empty()) return true; //если родительской директории нет, поиск завершен

 			if (name_mask.MatchTo(Utils::ExtractFileName(parent, true))) {
				Utils::AddTrailingCharIfNotExists(parent, SLASH_DIRS);
 				dest.push_back(parent);
 			}
		} 
	}

	return true;
}
