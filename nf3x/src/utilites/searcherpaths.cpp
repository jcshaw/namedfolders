/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
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

using namespace nf;

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

	inline WinSTL::findfile_sequence_t::flags_type get_search_flags(nf::twhat_to_search_t whatToSearch) {
		return WTS_DIRECTORIES == whatToSearch
			? WinSTL::findfile_sequence_t::directories
			: (WTS_FILES == whatToSearch 
				? WinSTL::findfile_sequence_t::files
				: WinSTL::findfile_sequence_t::files | WinSTL::findfile_sequence_t::directories); //=WTS_DIRECTORIES_AND_FILES
	}

	const wchar_t CH_SEARCH_BACKWORD = L'2';
	const wchar_t CH_SEARCH_FORWARD = L'1';
	const wchar_t CH_SEARCH_FORWARD_UNLIMITED = L'3';
	const wchar_t CH_SEARCH_BACKWORD_UNLIMITED = L'4';
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

void nf::Search::CSearchEngine::SearchItems(tstring const& rootDir0, tstring const& Name, nf::tlist_strings &destList, twhat_to_search_t whatToSearch) const {
	try {
		tstring name = Parser::ConvertMaskToReqex(Name);
		tregex expression(NF_BOOST_REGEX_COMPILE(name), boost::regex_constants::icase);
		tsmatch what;

		tstring RootDir = rootDir0;
		if (Utils::IsAliasIsDisk(RootDir)) RootDir += L"\\";

		WinSTL::findfile_sequence_t f(RootDir.c_str(), L"*", get_search_flags(whatToSearch));
		BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
			tstring filename = t.get_filename();
			if (NF_BOOST_REGEX_LIB::regex_match(filename, what, expression)) {
				destList.push_back(t.get_path());
			}
		}
	} catch (...) {
		SearchBySystemFunctions(rootDir0, Name, destList, whatToSearch);
	}
}

void nf::Search::CSearchEngine::SearchBySystemFunctions(tstring const& RootDir0, tstring const& Name, nf::tlist_strings &destList
														, twhat_to_search_t whatToSearch) const {	
	//search using system functions (they don't support [] metachar)
	tstring smask = Parser::ConvertToMask(Name);

	//если в качестве RootDir передается z: то поиск не срабатывает
	//проверяем и добавляем слеш
	tstring RootDir = RootDir0;
	if (Utils::IsAliasIsDisk(RootDir)) RootDir += L"\\";

	WinSTL::findfile_sequence_t f(RootDir.c_str(), smask.c_str(), get_search_flags(whatToSearch));
	BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
		tstring full_filename = t.get_path(); //Utils::CombinePath(RootDir0, t.get_filename(), SLASH_DIRS);
		destList.push_back(full_filename);
	}
}

wchar_t const* nf::Search::Private::extract_name(wchar_t const* srcPattern, tstring &destName, tstring &destLevel) {
	//извлечь из srcPattern первое имя, определить на каком кол-ве уровней вложенности
	//искать, вернуть остаток 
	//имена могут быть разделены \* и \.. (поиск вглубь, переход вверх)
	//специальный синтаксис для поиска без ограничения уровня вложенности: \** и \..*
	//destLevel contains sequence of required actions (CH_SEARCH_FORWARD | CH_SEARCH_BACKWORD | CH_SEARCH_FORWARD_UNLIMITED | CH_SEARCH_BACKWORD_UNLIMITED)
	wchar_t const* ps = srcPattern;
	assert(lstrlen(srcPattern) > 0);
	bool bshort_commands_allowed = (nf::CSettings::GetInstance().GetValue(nf::ST_ALLOW_ABBREVIATED_SYNTAX_FOR_DEEP_SEARCH) != 0);
	
	destLevel = L"";
	wchar_t prev_char = 0;
	while (is_slash(*ps)) {
		if (*ps == L'\\') { 
			if (bshort_commands_allowed || prev_char != L'\\') {
				destLevel += CH_SEARCH_FORWARD; // "\*\*\*" is equal to "\\\" 
			}
		} else if (*ps == MC_SEARCH_FORWARD_SHORT[0]) {
			destLevel += CH_SEARCH_FORWARD; // "\*\*\*" is equal to "\\\" 
		} else if (*ps == MC_SEARCH_BACKWORD_SHORT[0]) {
			destLevel += CH_SEARCH_BACKWORD;
		} else if (*ps == MC_SEARCH_BACKWORD_SHORT_WITHOUT_SLASH[0]) {
			if (bshort_commands_allowed || prev_char != MC_SEARCH_BACKWORD_SHORT_WITHOUT_SLASH[0]) {
				destLevel += CH_SEARCH_BACKWORD;
			}
		} else if (MC_DEEP_DIRECT_SEARCH_SHORT[0] == *ps) {
			destLevel += CH_SEARCH_FORWARD_UNLIMITED;
			++ps;
			break;
		} else if (MC_DEEP_REVERSE_SEARCH_SHORT[0] == *ps) {
			destLevel += CH_SEARCH_BACKWORD_UNLIMITED;
			++ps;
			break;
		}
		prev_char = *ps;
		++ps;
	}

	wchar_t const* psend = ps;
	while ((*psend) && (! is_slash(*psend))) ++psend;

	destName.assign(ps, psend - ps);
	return psend;
}

bool nf::Search::Private::search_multisubdir(tstring const& rootDir, tstring const& srcName
											 , unsigned int levelIndex
											 , tstring const& level
											 , CSearchEngine &searchEngine
											 , nf::tlist_strings &dest)
{	//поиск среди директорий любого уровня вложенности
	if (check_for_esc()) return false;	//exit by esc
	if (levelIndex >= level.size()) return true; //current directory is required one

	if (level[levelIndex] == CH_SEARCH_FORWARD || level[levelIndex] == CH_SEARCH_FORWARD_UNLIMITED) {
		//находим все непосредственно вложенные директории, 
		//удовлетворяющие маске и заносим их в результирующий список
		searchEngine.SearchItems(rootDir, srcName, dest);

		if (level[levelIndex] != CH_SEARCH_FORWARD_UNLIMITED && level[levelIndex] != CH_SEARCH_BACKWORD_UNLIMITED) levelIndex++;
		if (levelIndex >= level.size()) return true; //дальше не ищем, достигли требуемого уровня вложенности

		//рекурсивно ищем во всех вложенных директориях
		nf::tlist_strings list_subdirs;
		searchEngine.SearchItems(rootDir, L"*", list_subdirs, nf::WTS_DIRECTORIES);

		BOOST_FOREACH(tstring const& subdir, list_subdirs) {
			if (!search_multisubdir(subdir, srcName, levelIndex, level, searchEngine, dest)) {
				return false;	//exit by esc
			}
		}
	} else {
		//поиск в обратном направлении
		//переходим в директорию выше и проверяем, есть ли в ней директории
		//которые удовлетворяют маске
		
		//в rootDir удаляем последнюю директорию
		tstring parent = Utils::ExtractParentDirectory(rootDir);
		if (parent.empty()) return true; //если родительской директории нет, поиск завершен
		Utils::RemoveTrailingCharsOnPlace(parent, SLASH_DIRS_CHAR);	

		if (srcName.empty()) {
			if (levelIndex == level.size() - 1) {
				Utils::AddTrailingCharIfNotExists(parent, SLASH_DIRS);
				dest.push_back(parent); // cd:nf\.. ; add slash for to replace "c:" by "c:\"
			}
		} else {
			searchEngine.SearchItems(parent, srcName, dest);	
		}

		if (level[levelIndex] != CH_SEARCH_FORWARD_UNLIMITED && level[levelIndex] != CH_SEARCH_BACKWORD_UNLIMITED) levelIndex++;
		if (levelIndex >= level.size()) return true; //дальше не ищем, достигли требуемого уровня вложенности

		return search_multisubdir(parent, srcName, levelIndex, level, searchEngine, dest);
	}
	return true;
}

bool nf::Search::SearchByPattern(tstring const& Pattern, tstring const &RootDir, CSearchEngine &searchPolice
								 , nf::tlist_strings& dest)
{	// поиск директории, вложенной в текущую, по шаблону {[\dir]|[\\dir]|[\\\dir]|..}+
	// вариант поиска имени в текущей директории определяется кол-вом слешей.
	// "\dir"  - поиск имени dir только в текущей директории
	// "\\dir" - поиск имени dir во всех вложенных директориях и в директориях вложенных в них
	// "\\\dir" и т.д.
	// Неограниченная глубина через \t и \n
	nf::tlist_strings variants;

	tstring name; 
	tstring level;	//на скольких уровнях вложенности искать
	wchar_t const* next_pattern = Private::extract_name(Pattern.c_str(), name, level);
	
//Подгонка под #7, комментарий 48
//Команда far... приведет к лишней точке и ненужной маске = ".*". Точку убираем
	if (name == L".") name = L"";
//Команда cd:far\\\\\\\\\\ приводит к пустому name и level="1". Учитываем текущую директорию
	if (! level.empty() 
		&& searchPolice.AddAllFollowingPathsToResuts 
		&& next_pattern == 0 // cd:a\*\* , but not cd:\*\a*\b*, see #7.54
		&& Parser::ContainsMetachars(name)) {
		if (level[0] == CH_SEARCH_FORWARD || level[0] == CH_SEARCH_FORWARD_UNLIMITED) {
			variants.push_back(RootDir);
		}
	}

	if (! name.empty()) {
		if (searchPolice.AutoConvertNamesToMetachars) {
			name = Parser::ConvertToMask(name);
		}
	} else {
		if (level[0] == CH_SEARCH_FORWARD || level[0] == CH_SEARCH_FORWARD_UNLIMITED) {
			name = L"*"; //противоречие: cd:nf\** и cd:nf\..*
		}
	}

	Private::search_multisubdir(RootDir, name, 0, level, searchPolice, variants);

	if (! *next_pattern) {
		// поиск завершен, variants содержит искомые директории
		// копируем их в массив результатов
		std::copy(variants.begin(), variants.end(), std::insert_iterator<nf::tlist_strings >(dest, dest.begin()));
	} else {
		//в каждой из найденных директорий ищем следующие имена 
		BOOST_FOREACH(tstring const& dir, variants) {
			SearchByPattern(next_pattern, dir, searchPolice, dest);
		}
	}
	return true;
}

bool nf::Search::SearchMatched(tstring const& srcPathPattern, CSearchEngine &searchPolice, nf::tlist_strings& dest)
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
		return SearchByPattern(path_pattern.c_str(), disk_pattern + L":", searchPolice, dest);
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
				SearchByPattern(path_pattern, drive_name_short, searchPolice, dest);		
			}
		} //for
	}
	return true;
}