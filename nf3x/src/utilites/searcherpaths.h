/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
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
namespace Search {
	class MaskMatcher {	
	public:
		MaskMatcher(tstring const& srcMask);
		MaskMatcher(tstring const& srcMask, tasterix_mode const asterixMode012);
		bool MatchTo(tstring const& fileName) const;
	private:
		tregex m_R;
	};

	//search engine to search files and directories matched to specified mask
	//it's possible to use metachars "*", "?" and "[a-z]" in mask
	//engine is able to convert mask to appropriate regexp and check name matching using this regexp
	//if regexp failed then system search functions are used.
	class CSearchEngine {
	public:
		explicit CSearchEngine(twhat_to_search_t WhatToSearch
			, bool bAutoConvertNamesToMetachars = true // true: c->*c* according asterix mode;  false: c->c
		) :  m_WhatToSearch(WhatToSearch)
		, AutoConvertNamesToMetachars(bAutoConvertNamesToMetachars)
		{}
	//выполнить поиск поддиректории по заданному шаблону имени директории
		twhat_to_search_t GetWhatToSearch() const {
			return m_WhatToSearch;
		}
		void SearchItems(tstring const& RootDir, MaskMatcher const& maskMatcher, nf::tlist_strings &list, twhat_to_search_t whatToSearch) const;
		inline void SearchItems(tstring const& rootDir, MaskMatcher const& maskMatcher, nf::tlist_strings &destList) const {
			SearchItems(rootDir, maskMatcher, destList, m_WhatToSearch);
		}
		void SearchBySystemFunctions(tstring const& RootDir, MaskMatcher const& maskMatcher, nf::tlist_strings &list, twhat_to_search_t whatToSearch) const;
		const bool AutoConvertNamesToMetachars;
	private:
		twhat_to_search_t m_WhatToSearch;
	};

	// поиск директории, вложенной в текущую, по шаблону {[\dir]|[\\dir]|[\\\dir]|..}+
	// "\*", "\**", "\..", "\..*"
	bool SearchByPattern(tstring const& srcPattern, tstring const &rootDir, Search::CSearchEngine &searchPolice
		, tasterix_mode AsterixMaskMode012
		, nf::tlist_strings& dest);

	//найти все директории, удовлетвор€ющие паттерну
	//PathPattern должен содержать полный локальный путь вида
	//C:\path1\path2\...\pathN
	//причем C - люба€ одна буква или один из метасимволов ?, * или [a-z]
	//¬ токенах pathXXX могут встречатьс€ метасимволы ?, * или [a,b-z]
	bool SearchMatched(tstring const& PathPattern, Search::CSearchEngine &searchPolice, tasterix_mode AsterixMaskMode012, nf::tlist_strings& dest);
}
}