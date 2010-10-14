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

namespace
{
	//в header объявлены соответствующие wchar_t* 
	//версии wchar_t нужны, чтобы работал case
	wchar_t const DEEP_DIRECT_SEARCH_CH = L'\n';
	wchar_t const DEEP_REVERSE_SEARCH_CH = L'\t';
	wchar_t const DEEP_UP_DIRECTORY_CH = L'\b'; //символ, обозначающий переход на уровень выше = '..'

	inline bool is_slash(wchar_t ch) 
	{ 
		return SLASH_DIRS_CHAR == ch 
			|| DEEP_UP_DIRECTORY_CH == ch 
			|| DEEP_DIRECT_SEARCH_CH == ch 
			|| DEEP_REVERSE_SEARCH_CH == ch;
	}

	const int UNLIMITED_SEARCH_LEVEL = 9999;
}

bool CheckForEsc(void)
{
	bool EC=false;
	INPUT_RECORD rec;
	static HANDLE hConInp=GetStdHandle(STD_INPUT_HANDLE);
	DWORD ReadCount;
	while (1)
	{
		PeekConsoleInput(hConInp,&rec,1,&ReadCount);
		if (ReadCount==0) break;
		ReadConsoleInput(hConInp,&rec,1,&ReadCount);
		if (rec.EventType==KEY_EVENT)
			if (rec.Event.KeyEvent.wVirtualKeyCode==VK_ESCAPE &&
				rec.Event.KeyEvent.bKeyDown) EC=true;
	}
	return(EC);
}

//////////////////////////////////////////////////////////////////////////
// private
bool CSearcherPaths::SearchByPatternOEM(wchar_t const* Pattern
										, tstring const &RootDir)
{	// поиск директории, вложенной в текущую, по шаблону {[\dir]|[\\dir]|[\\\dir]|..}+
	// вариант поиска имени в текущей директории определяется кол-вом слешей.
	// "\dir"  - поиск имени dir только в текущей директории
	// "\\dir" - поиск имени dir во всех вложенных директориях и в директориях вложенных в них
	// "\\\dir" и т.д.
	// Неограниченная глубина через \t и \n

	//для простоты (посимвольная обработка) на первом шаге заменяем .*. и slesh*slesh на спецсимволы

	tstring name; 
	int level = 0;	//на скольких уровнях вложенности искать

	wchar_t const* next_pattern = extract_name(Pattern, name, level);
	if (name.empty())  return false; //поиск завершен, директория не подходит...

	name = Parser::ConvertToMask(name);

	tlist variants;
	search_multisubdir(RootDir.c_str(), name.c_str(), level, variants);

	if (! *next_pattern)
	{	// поиск завершен, variants содержит искомые директории
		// копируем их в массив результатов
		std::copy(variants.begin(), variants.end(), 
			std::insert_iterator<tlist>(m_Results, m_Results.begin()));
	} else {
		//в каждой из найденных директорий ищем следующие имена 
		std::for_each(variants.begin(), variants.end()
			, boost::bind(&CSearcherPaths::SearchByPatternOEM, this, next_pattern, _1));
	}

	return true;
}

bool CSearcherPaths::SearchMatchedOEM(tstring PathPattern)
{	//найти все директории, удовлетворяющие паттерну
	//PathPattern должен содержать полный локальный путь вида
	//C:\path1\path2\...\pathN
	//причем C - любая одна буква или один из метасимволов ?, * или [a-z]
	//В токенах pathXXX могут встречаться метасимволы ?, * или [a,b-z]

	//определяем букву диска
	wchar_t const* DISK_PATH_DELIMETER = L":\\";
	if (PathPattern.size() < 2) 
	{
		insert_result(PathPattern);
		return false;	//как минимум "c:"
	}
	
	size_t column_pos = PathPattern.find(DISK_PATH_DELIMETER);
	if (column_pos == tstring::npos) 
	{
		insert_result(PathPattern);
		return false;
	}

	tstring disk_pattern(PathPattern, 0, column_pos);
	PathPattern.erase(0, disk_pattern.size() + 1); 
		//удаляем имя диска (или маску) и ":", оставляем только путь, начинающийся с 1 слеша

	if (! disk_pattern.size()) {
		insert_result(PathPattern);
		return false;
	}
	
	if (disk_pattern.size() == 1 &&  disk_pattern[0] != L'*' &&  disk_pattern[0] != L'?') {	//диск указан явно
		if (! PathPattern.size()) {	//указан путь типа "c:"
			insert_result(PathPattern);
			return true;
		}
		tstring root_path = disk_pattern + L":";
		
		return SearchByPatternOEM(PathPattern.c_str(), root_path);
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
				SearchByPatternOEM(PathPattern.c_str(), drive_name_short);		
			}
		} //for
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
// private

wchar_t const* CSearcherPaths::extract_name(wchar_t const* Pattern, tstring &Name, int &Level)
{	//извлечь из Pattern первое имя, определить на каком кол-ве уровней вложенности
	//искать, вернуть остаток
	//имена могут быть разделены слешами и точками (слеш - поиск вглубь, точка - вверх)
	//специальный синтаксис для поиска без ограничения уровня вложенности
	//.*. и slesh*slesh
	wchar_t const* ps = Pattern;
	int pattern_length = lstrlen(Pattern);
	assert(pattern_length > 0);
	
	bool bSearchDirectionToDeep = true;
	Level = 0;
	while (is_slash(*ps)) 
	{
		switch (*ps)
		{
		case L'\\': ++Level; break;
		case DEEP_UP_DIRECTORY_CH: --Level; break;
		case DEEP_DIRECT_SEARCH_CH: Level = UNLIMITED_SEARCH_LEVEL; break;
		case DEEP_REVERSE_SEARCH_CH: Level = -UNLIMITED_SEARCH_LEVEL; break;
		default: assert(false);
		}
		++ps;
	}

	wchar_t const* psend = ps;
	while ((*psend) && (! is_slash(*psend))) ++psend;

	Name.assign(ps, psend - ps);
	return psend;
}

bool CSearcherPaths::search_multisubdir(wchar_t const* RootDir
										, wchar_t const* Name
										, int level
										, tlist &list)
{	//поиск среди директорий любого уровня вложенности
	if (level == 0) ++level;	// \ и . сокращаются, тогда ищем просто в текущей директории

	if (CheckForEsc()) return false;	//exit by esc

	if (level >= 0)
	{
		//находим все непосредственно вложенные директории, 
		//удовлетворяющие маске и заносим их в результирующий список
		m_Sp.SearchSubdir(RootDir, Name, list);

		level--;

		if (! level) return true;	//дальше не ищем, достигли требуемого уровня вложенности

		//рекурсивно ищем во всех вложенных директориях
		tlist list_subdirs;
		SearchPathsPolices::CSearchSystemPolice sp(m_WhatToSearch); //!tODO
		sp.SearchSubdir(RootDir, L"*", list_subdirs);

		tlist::const_iterator p = list_subdirs.begin();
		while (p != list_subdirs.end())
		{
			if (!search_multisubdir(p->c_str(), Name, level, list)) 
			{
				return false;	//exit by esc
			}
			++p;
		}
	} else 
	{
		//поиск в обратном направлении
		//переходим в директорию выше и проверяем, есть ли в ней директории
		//которые удовлетворяют маске
		
		//в RootDir удаляем последнюю директорию
		tstring parent = RootDir;
		while (level)
		{
			parent = Utils::ExtractParentDirectory(parent);
			if (parent.empty()) return true; //если родительской директории нет, поиск завершен
			Utils::RemoveTrailingCharsOnPlace(parent, SLASH_DIRS_CHAR);	//!TODO: 

			m_Sp.SearchSubdir(parent, Name, list);
		
			level++;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void nf::SearchPathsPolices::
CSearchSystemPolice::SearchSubdir(tstring const& RootDir0
								  , tstring const& Name
								  , std::list<tstring> &list) const
{	//найти удовлетворяющие паттерну поддиректории системными функциями
	tstring smask = Parser::ConvertToMask(Name);

	//если в качестве RootDir передается z: то поиск не срабатывает
	//проверяем и добавляем слеш
	tstring RootDir = RootDir0;
	if (Utils::IsAliasIsDisk(RootDir)) RootDir += L"\\";

	nf::twhat_to_search_t what_to_search = GetWhatToSearch();
	WinSTL::findfile_sequence_t f(RootDir.c_str()
		, smask.c_str()
		, WTS_DIRECTORIES == what_to_search
			? WinSTL::findfile_sequence_t::directories
			: (WTS_FILES == what_to_search 
				? WinSTL::findfile_sequence_t::files
				: WinSTL::findfile_sequence_t::files | WinSTL::findfile_sequence_t::directories)
		);
	BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
		tstring full_filename = Utils::CombinePath(RootDir0, t.get_filename(), SLASH_DIRS);
		list.push_back(full_filename);
	}
}

void  nf::SearchPathsPolices::
CSearchFarPolice::SearchSubdir(tstring const& RootDir
							   , tstring const& Name
							   , std::list<tstring> &list) const
{	//найти удовлетворяющие паттерну поддиректории средствами FAR
	nf::twhat_to_search_t what_to_search = GetWhatToSearch();
	tstring root_dir = RootDir;
	if (root_dir.size() == 2) root_dir += SLASH_DIRS;	
		//!TODO: к имени диска НЕОБХОДИМО добавить \\
		//иначе в релизе findfile_sequence_t использует неправильную корневую директорию
		//баг stlsoft?
	WinSTL::findfile_sequence_t f(root_dir.c_str()
		, L"*"
		, WTS_DIRECTORIES == what_to_search
			? WinSTL::findfile_sequence_t::directories
			: (WTS_FILES == what_to_search 
				? WinSTL::findfile_sequence_t::files
				: WinSTL::findfile_sequence_t::files | WinSTL::findfile_sequence_t::directories)
 	);
	BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
		tstring file_name = t.get_filename();
		if (nf::Parser::IsTokenMatchedToPatternOEM(file_name, Name, false)
			|| (file_name == Name)	//прямое сравнение на случай, если в пути используются квадратные скобки
			)
		{
			tstring full_filename = tstring(RootDir) + tstring(SLASH_DIRS) + file_name;
			list.push_back(full_filename);
		};
	} 

}
