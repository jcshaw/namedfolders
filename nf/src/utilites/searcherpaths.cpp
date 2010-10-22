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

#include "Parser.h"

using namespace nf;

namespace {
	//в header объявлены соответствующие wchar_t* 
	//версии wchar_t нужны, чтобы работал case
	wchar_t const DEEP_DIRECT_SEARCH_CH = L'\n';
	wchar_t const DEEP_REVERSE_SEARCH_CH = L'\t';
	wchar_t const DEEP_UP_DIRECTORY_CH = L'\b'; //символ, обозначающий переход на уровень выше = '..'

	inline bool is_slash(wchar_t ch) { 
		return SLASH_DIRS_CHAR == ch 
			|| DEEP_UP_DIRECTORY_CH == ch 
			|| DEEP_DIRECT_SEARCH_CH == ch 
			|| DEEP_REVERSE_SEARCH_CH == ch;
	}
	const int UNLIMITED_SEARCH_LEVEL = 9999;

	inline WinSTL::findfile_sequence_t::flags_type get_search_flags(nf::twhat_to_search_t whatToSearch) {
		return WTS_DIRECTORIES == whatToSearch
			? WinSTL::findfile_sequence_t::directories
			: (WTS_FILES == whatToSearch 
				? WinSTL::findfile_sequence_t::files
				: WinSTL::findfile_sequence_t::files | WinSTL::findfile_sequence_t::directories); //=WTS_DIRECTORIES_AND_FILES
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

void nf::Search::CSearchSystemPolice::SearchSubdir(tstring const& RootDir0, tstring const& Name, std::list<tstring> &list) const
{	//найти удовлетворяющие паттерну поддиректории системными функциями
	tstring smask = Parser::ConvertToMask(Name);

	//если в качестве RootDir передается z: то поиск не срабатывает
	//проверяем и добавляем слеш
	tstring RootDir = RootDir0;
	if (Utils::IsAliasIsDisk(RootDir)) RootDir += L"\\";

	nf::twhat_to_search_t what_to_search = GetWhatToSearch();
	WinSTL::findfile_sequence_t f(RootDir.c_str(), smask.c_str(), get_search_flags(what_to_search));
	BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
		tstring full_filename = Utils::CombinePath(RootDir0, t.get_filename(), SLASH_DIRS);
		list.push_back(full_filename);
	}
}

void nf::Search::CSearchFarPolice::SearchSubdir(tstring const& RootDir, tstring const& Name, std::list<tstring> &list) const
{	//найти удовлетворяющие паттерну поддиректории средствами FAR
	nf::twhat_to_search_t what_to_search = GetWhatToSearch();
	tstring root_dir = RootDir;
	if (root_dir.size() == 2) root_dir += SLASH_DIRS;	
	//!TODO: к имени диска НЕОБХОДИМО добавить \\
	//иначе в релизе findfile_sequence_t использует неправильную корневую директорию
	//баг stlsoft?
	WinSTL::findfile_sequence_t f(root_dir.c_str(), L"*", get_search_flags(what_to_search));
	BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
		tstring file_name = t.get_filename();
		if (nf::Parser::IsTokenMatchedToPattern(file_name, Name, false)
			|| (file_name == Name)	//прямое сравнение на случай, если в пути используются квадратные скобки
			)
		{
			tstring full_filename = tstring(RootDir) + tstring(SLASH_DIRS) + file_name;
			list.push_back(full_filename);
		};
	} 
}

wchar_t const* nf::Search::Private::extract_name(wchar_t const* srcPattern, tstring &destName, int &destLevel)
{	//извлечь из srcPattern первое имя, определить на каком кол-ве уровней вложенности
	//искать, вернуть остаток
	//имена могут быть разделены слешами и точками (слеш - поиск вглубь, точка - вверх)
	//специальный синтаксис для поиска без ограничения уровня вложенности
	//.*. и slesh*slesh
	wchar_t const* ps = srcPattern;
	assert(lstrlen(srcPattern) > 0);
	
	destLevel = 0;
	while (is_slash(*ps)) {
		switch (*ps) {
		case L'\\': ++destLevel; break;
		case DEEP_UP_DIRECTORY_CH: --destLevel; break;
		case DEEP_DIRECT_SEARCH_CH: destLevel = UNLIMITED_SEARCH_LEVEL; break;
		case DEEP_REVERSE_SEARCH_CH: destLevel = -UNLIMITED_SEARCH_LEVEL; break;
		default: assert(false);
		}
		++ps;
	}

	wchar_t const* psend = ps;
	while ((*psend) && (! is_slash(*psend))) ++psend;

	destName.assign(ps, psend - ps);
	return psend;
}

bool nf::Search::Private::search_multisubdir(tstring const& rootDir, tstring const& srcName, int level
											 , CSearchPolice &searchPolice
											 , std::list<tstring> &dest)
{	//поиск среди директорий любого уровня вложенности
	if (level == 0) ++level;	// \ и . сокращаются, тогда ищем просто в текущей директории
	if (check_for_esc()) return false;	//exit by esc

	if (level >= 0) {
		//находим все непосредственно вложенные директории, 
		//удовлетворяющие маске и заносим их в результирующий список
		searchPolice.SearchSubdir(rootDir, srcName, dest);

		level--;
		if (! level) return true;	//дальше не ищем, достигли требуемого уровня вложенности

		//рекурсивно ищем во всех вложенных директориях
		std::list<tstring> list_subdirs;
		Search::CSearchSystemPolice sp(nf::WTS_DIRECTORIES); 
		sp.SearchSubdir(rootDir, L"*", list_subdirs);

		BOOST_FOREACH(tstring const& subdir, list_subdirs) {
			if (!search_multisubdir(subdir, srcName, level, searchPolice, dest)) {
				return false;	//exit by esc
			}
		}
	} else  {
		//поиск в обратном направлении
		//переходим в директорию выше и проверяем, есть ли в ней директории
		//которые удовлетворяют маске
		
		//в rootDir удаляем последнюю директорию
		tstring parent = rootDir;
		while (level) {
			parent = Utils::ExtractParentDirectory(parent);
			if (parent.empty()) return true; //если родительской директории нет, поиск завершен
			Utils::RemoveTrailingCharsOnPlace(parent, SLASH_DIRS_CHAR);	//!TODO: 

			searchPolice.SearchSubdir(parent, srcName, dest);	
			level++;
		}
	}
	return true;
}

bool nf::Search::SearchByPattern(tstring const&Pattern, tstring const &RootDir, CSearchPolice &searchPolice
								 , std::list<tstring>& dest)
{	// поиск директории, вложенной в текущую, по шаблону {[\dir]|[\\dir]|[\\\dir]|..}+
	// вариант поиска имени в текущей директории определяется кол-вом слешей.
	// "\dir"  - поиск имени dir только в текущей директории
	// "\\dir" - поиск имени dir во всех вложенных директориях и в директориях вложенных в них
	// "\\\dir" и т.д.
	// Неограниченная глубина через \t и \n

	//для простоты (посимвольная обработка) на первом шаге заменяем .*. и slesh*slesh на спецсимволы

	tstring name; 
	int level = 0;	//на скольких уровнях вложенности искать

	wchar_t const* next_pattern = Private::extract_name(Pattern.c_str(), name, level);
	if (name.empty())  return false; //поиск завершен, директория не подходит...

	name = Parser::ConvertToMask(name);

	std::list<tstring> variants;
	Private::search_multisubdir(RootDir, name, level, searchPolice, variants);

	if (! *next_pattern) {
		// поиск завершен, variants содержит искомые директории
		// копируем их в массив результатов
		std::copy(variants.begin(), variants.end(), 
			std::insert_iterator<std::list<tstring> >(dest, dest.begin()));
	} else {
		//в каждой из найденных директорий ищем следующие имена 
		BOOST_FOREACH(tstring const& dir, variants) {
			SearchByPattern(next_pattern, dir, searchPolice, dest);
		}
	}
	return true;
}

bool nf::Search::SearchMatched(tstring const& srcPathPattern, CSearchPolice &searchPolice, std::list<tstring>& dest)
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