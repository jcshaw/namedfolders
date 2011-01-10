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
	PathsFinder(CSearchEngine &searchPolice, bool bShortSyntaxInPathAllowed, tasterix_mode asterixMode012);
	~PathsFinder(void);

	bool SearchByPattern(tstring const& Pattern, tstring const &RootDir, nf::tlist_strings& dest);
private:
	bool search(nf::tlist_pairs_strings::const_iterator listItemsPos, nf::tlist_pairs_strings const& listItems, tstring const &RootDir, nf::tlist_strings& dest);
	bool deep_search(tpair_strings nameMetachar, tstring const& rootDir, nf::tlist_strings &dest);
	wchar_t const* extract_metachar(wchar_t const* srcPattern, tpair_strings& nameMetachar);
	CSearchEngine m_SearchPolice;
	bool const ShortSyntaxInPathAllowed;
	tasterix_mode const AsterixMode012;
};

}
}