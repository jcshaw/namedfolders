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
//поиск директорий ведетс€ системными функци€м
//системные функции позвол€ют задать маску поиска с использованием метасимволов * и ?
//если требуетс€ поддержка дополнительного метасимвола [], объ€вленного в FAR
//необходимо вначале найти все директории, затем проверить их совпадение с шаблоном средствами FAR
//в итоге два варианта поиска дл€ класса CSearcherPaths задаютс€ в Search
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
		//поиск системными средствами; метасимволы [] не поддерживаютс€
		//все найденные имена автоматически считаютс€ совпавшими с шаблоном
	public:
		explicit CSearchSystemPolice(twhat_to_search_t WhatToSearch) : CSearchPolice(WhatToSearch) {}
		virtual void SearchSubdir(tstring const& RootDir, tstring const& Name, nf::tlist_strings &list) const;
	};

	class CSearchFarPolice : public CSearchPolice
	{	//находим все директорий, а затем провер€ем совпадение имен с шаблоном
		//средствами Far
	public:
		explicit CSearchFarPolice(twhat_to_search_t WhatToSearch) : CSearchPolice(WhatToSearch) {}
		virtual void SearchSubdir(tstring const& RootDir, tstring const& Name, nf::tlist_strings &list) const;
	};

	// поиск директории, вложенной в текущую, по шаблону {[\dir]|[\\dir]|[\\\dir]|..}+
	// "\*", "\**", "\..", "\..*"
	bool SearchByPattern(tstring const& srcPattern, tstring const &rootDir, Search::CSearchPolice &searchPolice, nf::tlist_strings& dest);

	//найти все директории, удовлетвор€ющие паттерну
	//PathPattern должен содержать полный локальный путь вида
	//C:\path1\path2\...\pathN
	//причем C - люба€ одна буква или один из метасимволов ?, * или [a-z]
	//¬ токенах pathXXX могут встречатьс€ метасимволы ?, * или [a,b-z]
	bool SearchMatched(tstring const& PathPattern, Search::CSearchPolice &searchPolice, nf::tlist_strings& dest);

	namespace Private {
		wchar_t const* extract_name(wchar_t const* srcPattern, tstring &destName, tstring &destLevel);
		bool search_multisubdir(tstring const& RootDir, tstring const& Name
			, unsigned levelIndex
			, tstring const& level
			, Search::CSearchPolice &searchPolice
			, nf::tlist_strings &dest);
	}
}
}