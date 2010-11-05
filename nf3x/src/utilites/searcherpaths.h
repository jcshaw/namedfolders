/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

/************************************************************************/
// поиск директорий по заданным паттернам
// паттерны могут содержать стандартные метасимволы Far
// *, ? и [a-z]
/************************************************************************/
#include "strings_utils.h"

namespace nf {
//поиск директорий ведется системными функциям
//системные функции позволяют задать маску поиска с использованием метасимволов * и ?
//если требуется поддержка дополнительного метасимвола [], объявленного в FAR
//необходимо вначале найти все директории, затем проверить их совпадение с шаблоном средствами FAR
//в итоге два варианта поиска для класса CSearcherPaths задаются в Search
namespace Search {
	struct CSearchPolice {
		explicit CSearchPolice(twhat_to_search_t WhatToSearch) :  m_WhatToSearch(WhatToSearch) {}
	//выполнить поиск поддиректории по заданному шаблону имени директории
		virtual void  SearchSubdir(tstring const& RootDir, tstring const& Name, nf::tlist_strings &list) const = 0;
		twhat_to_search_t GetWhatToSearch() const {
			return m_WhatToSearch; }
	private:
		twhat_to_search_t m_WhatToSearch;
	};

	class CSearchSystemPolice : public CSearchPolice {
		//поиск системными средствами; метасимволы [] не поддерживаются
		//все найденные имена автоматически считаются совпавшими с шаблоном
	public:
		explicit CSearchSystemPolice(twhat_to_search_t WhatToSearch) : CSearchPolice(WhatToSearch) {}
		virtual void SearchSubdir(tstring const& RootDir, tstring const& Name, nf::tlist_strings &list) const;
	};

	class CSearchFarPolice : public CSearchPolice
	{	//находим все директорий, а затем проверяем совпадение имен с шаблоном
		//средствами Far
	public:
		explicit CSearchFarPolice(twhat_to_search_t WhatToSearch) : CSearchPolice(WhatToSearch) {}
		virtual void SearchSubdir(tstring const& RootDir, tstring const& Name, nf::tlist_strings &list) const;
	};

	// поиск директории, вложенной в текущую, по шаблону {[\dir]|[\\dir]|[\\\dir]|..}+
	// вариант поиска имени в текущей директории определяется кол-вом слешей.
	// "\dir"  - поиск dir только в текущей директории
	// "\\dir" - поиск dir во всех вложенных директориях и в директориях непосредственно в них вложенных 
	// "\\\dir" и т.д.
	//? "\*\dir" - поиск имени dir во всех вложенных директориях
	//? ".dir" - переход dir начиная с директории на один уровень выше, "..dir" на два уровня выше и т.д.
	bool SearchByPattern(tstring const& srcPattern, tstring const &rootDir, Search::CSearchPolice &searchPolice, nf::tlist_strings& dest);

	//найти все директории, удовлетворяющие паттерну
	//PathPattern должен содержать полный локальный путь вида
	//C:\path1\path2\...\pathN
	//причем C - любая одна буква или один из метасимволов ?, * или [a-z]
	//В токенах pathXXX могут встречаться метасимволы ?, * или [a,b-z]
	bool SearchMatched(tstring const& PathPattern, Search::CSearchPolice &searchPolice, nf::tlist_strings& dest);

	namespace Private {
		wchar_t const* extract_name(wchar_t const* srcPattern, tstring &destName, int &destLevel);
		bool search_multisubdir(tstring const& RootDir, tstring const& Name, int level
			, Search::CSearchPolice &searchPolice
			, nf::tlist_strings &dest);
	}
}
}