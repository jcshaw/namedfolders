/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

#include "searcherpaths.h"

namespace nf {
namespace Search {

//extended implementation of SearchByPattern function
class PathsFinder {
public:
	PathsFinder(CSearchEngine &searchPolice);
	~PathsFinder(void);

	bool SearchByPattern(tstring const& Pattern, tstring const &RootDir, nf::tlist_strings& dest);
private:
	bool search(unsigned int itemIndex, nf::tlist_pairs_strings listItems, tstring const &RootDir, nf::tlist_strings& dest);
	bool deep_search(tpair_strings nameMetachar, tstring const& rootDir, nf::tlist_strings &dest);
	wchar_t const* extract_name(wchar_t const* srcPattern, tpair_strings& nameMetachar);
	CSearchEngine m_SearchPolice;
	bool const ShortSyntaxInPathAllowed;
};

}
}