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
	const int UNLIMITED_SEARCH_LEVEL = 9999;
	const int MAX_DEEP_SEARCH = 100;
	const int CHAR_FORWARD_SEARCH_BASE = 1;
	const int CHAR_BACKWORD_SEARCH_BASE = 120;

	const wchar_t CH_SEARCH_BACKWORD = L'2';
	const wchar_t CH_SEARCH_FORWARD = L'1';
	const wchar_t CH_SEARCH_FORWARD_UNLIMITED = L'3';
	const wchar_t CH_SEARCH_BACKWORD_UNLIMITED = L'4';

	typedef enum tsearch_ch_kind {
		ID_SEARCH_FORWARD
		, ID_SEARCH_BACKWORD
	};

	//У каждого метасимвола есть две характеристики: направление и глубина поиска.
	//Символ "\*" имеет направление "вниз", символ "\.." - вверх.
	//Глубина поиска может быть неограниченной или ограниченной.
	//У "\" глубина равна 1, у "\**" - 256 (неограничена), у "\**4" - 4,  у "\\\\" - либо 1 либо 4 в зависимости от настроек.
	//Глубина поиска не может быть больше MAX_DEEP_SEARCH - если указана большая глубина, то поиск считается неограниченным.
	tsearch_ch_kind get_metachar_kind(tstring const& metaChar) {
		assert(metaChar.size() == 1);
		return (static_cast<int>(metaChar[0]) < CHAR_BACKWORD_SEARCH_BASE) 
			? ID_SEARCH_FORWARD
			: ID_SEARCH_BACKWORD;
	}
	int get_metachar_deep(tstring const& metaChar) {
		assert(metaChar.size() == 1);
		return static_cast<int>(metaChar[0]) - (get_metachar_kind(metaChar) == ID_SEARCH_FORWARD
			? CHAR_FORWARD_SEARCH_BASE 
			: CHAR_BACKWORD_SEARCH_BASE);
	}
	tstring combine_metachar(int baseChar, int deepSearch) {
		tstring dest;
		dest.resize(1);
		if (deepSearch == 0) {
			dest[0] = static_cast<wchar_t>(MAX_DEEP_SEARCH);
			return dest;
		}
		if (deepSearch < 0) deepSearch = 0;
		if (deepSearch > MAX_DEEP_SEARCH) deepSearch = MAX_DEEP_SEARCH;
		dest[0] = static_cast<wchar_t>(baseChar + deepSearch);
		return dest;
	}
	tstring create_metachar(tsearch_ch_kind slashKind, int deepSearch) {
		switch (slashKind) {
		case ID_SEARCH_FORWARD: return combine_metachar(CHAR_FORWARD_SEARCH_BASE, deepSearch);
		case ID_SEARCH_BACKWORD: return combine_metachar(CHAR_BACKWORD_SEARCH_BASE, deepSearch);
		}
		return 0;
	}
	tstring increment_deep(tstring const& metaChar, int incValue) {
		tsearch_ch_kind kind = get_metachar_kind(metaChar);
		int deep = get_metachar_deep(metaChar);
		deep += incValue;
		return create_metachar(kind, deep);
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


nf::Search::PathsFinder::PathsFinder(CSearchEngine &searchPolice)
: m_SearchPolice(searchPolice)
, ShortSyntaxInPathAllowed(nf::CSettings::GetInstance().GetValue(nf::ST_ALLOW_ABBREVIATED_SYNTAX_FOR_DEEP_SEARCH) != 0)
{
}

nf::Search::PathsFinder::~PathsFinder(void)
{
}

wchar_t const* nf::Search::PathsFinder::extract_name(wchar_t const* srcPattern, tpair_strings& nameMetachar) {
	//извлечь из srcPattern первый метасимвол и, если есть, первое имя; вернуть остаток
	//повторяющиеся метасимволы \\\ и ... объединить в один с фиксированной глубиной поиска
	wchar_t const* ps = srcPattern;
	nameMetachar.first = L"*";
	nameMetachar.second = L" ";
	tstring& m = nameMetachar.second;
	
	wchar_t current_ch = 0;
	while (is_slash(*ps)) {
		if (*ps == L'\\' || *ps == MC_SEARCH_BACKWORD_SHORT_WITHOUT_SLASH[0]) { 
			if (current_ch == 0) {
				m = create_metachar(ID_SEARCH_FORWARD, 1);
			} else if (current_ch != *ps) {
				return ps;
			} else if (this->ShortSyntaxInPathAllowed) {
				m = increment_deep(m, 1);
			}
		} else if (*ps == MC_SEARCH_FORWARD_SHORT[0]) {
			m = create_metachar(ID_SEARCH_FORWARD, 1); // "\*"
			++ps;
			break;
		} else if (*ps == MC_SEARCH_BACKWORD_SHORT[0]) {
			m = create_metachar(ID_SEARCH_BACKWORD, 1); // "\.."
			++ps;
			break;
		} else if (MC_DEEP_DIRECT_SEARCH_SHORT[0] == *ps) {
			m = create_metachar(ID_SEARCH_FORWARD, 0);
			++ps;
			break;
		} else if (MC_DEEP_REVERSE_SEARCH_SHORT[0] == *ps) {
			m = create_metachar(ID_SEARCH_BACKWORD, 0);
			++ps;
			break;
		}
		current_ch = *ps;
		++ps;
	}

	wchar_t const* psend = ps;
	while ((*psend) && (! is_slash(*psend))) ++psend;

	if (psend != ps) nameMetachar.first.assign(ps, psend - ps);
	return psend;
}

// bool nf::Search::PathsFinder::search_dirs(tstring const& rootDir, tstring const& srcName
// 										  , unsigned int levelIndex, tstring const& level, nf::tlist_strings &dest)
// {	//поиск среди директорий любого уровня вложенности
// 	if (check_for_esc()) return false;	//exit by esc
// 	if (levelIndex >= level.size()) return true; //current directory is required one
// 
// 	nf::tlist_strings dirs; //директории, в которых вести поиск дальшейших метасимволов
// 	if (get_metachar_kind(level[levelIndex]) == ID_SEARCH_FORWARD) {
// 		//находим все вложенные директории, удовлетворяющие маске; ищем на заданных N уровнях вложенности
// 		deep_search_forward(get_metachar_deep(level[levelIndex]), rootDir, srcName, dirs);
// 		BOOST_FOREACH(tstring const& sdir, dirs) {
// 			dest.push_back(sdir);
// 		}
// 	} else { //поиск в обратном направлении; 
// 		//переходим в директорию выше и проверяем, есть ли в ней директории, которые удовлетворяют маске		
// 		tstring parent = Utils::RemoveTrailingChars(Utils::ExtractParentDirectory(rootDir), SLASH_DIRS_CHAR);
// 		if (parent.empty()) return true; //если родительской директории нет, поиск завершен
// 
// 		if (srcName.empty()) {
// 			if (levelIndex == level.size() - 1) {
// 				Utils::AddTrailingCharIfNotExists(parent, SLASH_DIRS);
// 				dest.push_back(parent); // cd:nf\.. ; add slash to replace "c:" by "c:\"
// 			}
// 		} else {
// 			//находим все вложенные директории, удовлетворяющие маске; ищем на заданных N уровнях вложенности
// 			deep_search_backword(get_metachar_deep(level[levelIndex]), parent, srcName, dirs); TODO: здесь это не пройдет
// 		}
// 	}
// 
// 	if (++levelIndex >= level.size()) return true; //больше метасимволов для поиска нет
// 	//в найденных директориях ищем оставшиеся метасимволы
// 	BOOST_FOREACH(tstring const& subdir, dirs) {
// 		if (! search_dirs(subdir, srcName, levelIndex, level, m_SearchPolice, dest)) {
// 			return false;	//exit by esc
// 		}
// 	}
// 
// 	return true;
// }
// 
// bool nf::Search::PathsFinder::SearchByPattern(tstring const& Pattern, tstring const &RootDir, nf::tlist_strings& dest)
// {	// поиск директории, вложенной в текущую, по шаблону {[\dir]|[\\dir]|[\\\dir]|..}+
// 	// вариант поиска имени в текущей директории определяется кол-вом слешей.
// 	// "\dir"  - поиск имени dir только в текущей директории
// 	// "\\dir" - поиск имени dir во всех вложенных директориях и в директориях вложенных в них
// 	// "\\\dir" и т.д.
// 	// Неограниченная глубина через \t и \n
// 	nf::tlist_strings variants;
// 
// 	tstring name; 
// 	tstring level;	//на скольких уровнях вложенности искать
// 	wchar_t const* next_pattern = Private::extract_name(Pattern.c_str(), name, level);
// 
// 	//Подгонка под #7, комментарий 48
// 	//Команда far... приведет к лишней точке и ненужной маске = ".*". Точку убираем
// 	if (name == L".") name = L"";
// 	//Команда cd:far\\\\\\\\\\ приводит к пустому name и level="1". Учитываем текущую директорию
// 	if (! level.empty() 
// 		&& m_SearchPolice.AddAllFollowingPathsToResuts 
// 		&& next_pattern == 0 // cd:a\*\* , but not cd:\*\a*\b*, see #7.54
// 		&& Parser::ContainsMetachars(name)) {
// 			if (level[0] == CH_SEARCH_FORWARD || level[0] == CH_SEARCH_FORWARD_UNLIMITED) {
// 				variants.push_back(RootDir);
// 			}
// 	}
// 
// 	if (! name.empty()) {
// 		if (m_SearchPolice.AutoConvertNamesToMetachars) {
// 			name = Parser::ConvertToMask(name);
// 		}
// 	} else {
// 		if (level[0] == CH_SEARCH_FORWARD || level[0] == CH_SEARCH_FORWARD_UNLIMITED) {
// 			name = L"*"; //противоречие: cd:nf\** и cd:nf\..*
// 		}
// 	}
// 
// 	Private::search_dirs(RootDir, name, 0, level, m_SearchPolice, variants);
// 
// 	if (! *next_pattern) {
// 		// поиск завершен, variants содержит искомые директории
// 		// копируем их в массив результатов
// 		std::copy(variants.begin(), variants.end(), std::insert_iterator<nf::tlist_strings >(dest, dest.begin()));
// 	} else {
// 		//в каждой из найденных директорий ищем следующие имена 
// 		BOOST_FOREACH(tstring const& dir, variants) {
// 			SearchByPattern(next_pattern, dir, dest);
// 		}
// 	}
// 	return true;
// }

bool nf::Search::PathsFinder::SearchByPattern(tstring const& Pattern, tstring const &RootDir, nf::tlist_strings& dest) {
	nf::tlist_pairs_strings list; //список метасимволы-имя

	wchar_t const* p = Pattern.c_str();
	while (p != NULL) {
		tpair_strings name_metachar;
		p = extract_name(Pattern.c_str(), name_metachar);
		list.push_back(name_metachar);
	} 

	nf::tlist_strings results;
	bool bret = search(0, list, RootDir, results);
	dest.swap(results);

	return bret;
}

bool nf::Search::PathsFinder::search(unsigned int itemIndex, nf::tlist_pairs_strings listItems, tstring const &rootDir, nf::tlist_strings& dest) {
	if (itemIndex >= listItems.size()) return true; 

	nf::tlist_strings list;
//находим все директории, в которых нужно вести поиск остальных элементов 
//если мы ищем файлы, то на последнем элементе в list попадут файлы, а не директории
	nf::tlist_pairs_strings::const_iterator p = listItems.begin();
	std::advance(p, itemIndex);
	if (! deep_search(*p, rootDir, list)) return false;

	if (itemIndex == listItems.size() - 1) {
		std::copy(list.begin(), list.end(), std::insert_iterator<nf::tlist_strings >(dest, dest.begin()));
	} else { //продолжаем поиск остальных элементов относительно найденных директорий
		BOOST_FOREACH(tstring const& dir, list) {
			if (! search(itemIndex + 1, listItems, dir, dest)) return false;
		}	
	}
	return true;
}

bool nf::Search::PathsFinder::deep_search(tpair_strings nameMetachar, tstring const& rootDir, nf::tlist_strings &dest) {
	//при поиске на N уровней вглубь/вверх
	//1)находим все файлы/директории находящиеся на данном уровне и удовлетворяющие name и заносим их в результаты
	//2)находим все директории на каждом уровне и затем ищем в них дальше 
	unsigned int deep = get_metachar_deep(nameMetachar.second);
	if (deep == 0) deep = 9999; //вряд ли уровень вложенности директорий будет больше..


	nf::tlist_strings dirs; 
	if (get_metachar_kind(nameMetachar.second) == ID_SEARCH_FORWARD) {
		dirs.push_back(rootDir);
		for (unsigned int i = 0; i < deep; ++i) {
			nf::tlist_strings level_dirs;
			BOOST_FOREACH(tstring const& sdir, dirs) {
				if (check_for_esc()) return false;	//exit by esc
				m_SearchPolice.SearchItems(rootDir, L"*", level_dirs, WTS_DIRECTORIES);
				m_SearchPolice.SearchItems(rootDir, nameMetachar.first, dest);
			}
			if (level_dirs.size() == 0) break; //нет больше вложенных директорий
			dirs.swap(level_dirs);
		}		
	} else {
		tstring parent = rootDir;
		for (unsigned int i = 0; i < deep; ++i) {
			if (check_for_esc()) return false;	//exit by esc
			//переходим в директорию выше и проверяем, есть ли в ней директории, которые удовлетворяют name		
			parent = Utils::RemoveTrailingChars(Utils::ExtractParentDirectory(parent), SLASH_DIRS_CHAR);
			if (parent.empty()) return true; //если родительской директории нет, поиск завершен
			if (nameMetachar.first == L".") { //!TODO
				Utils::AddTrailingCharIfNotExists(parent, SLASH_DIRS);
				dest.push_back(parent); // cd:nf\.. ; add slash to replace "c:" by "c:\"
			} else {
				m_SearchPolice.SearchItems(parent, nameMetachar.first, dest);
			}
		} 
	}

	return true;
}
