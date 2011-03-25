/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "stdafx.h"
#include "catalog_names.h"

//#include <locale> 
#include <boost/foreach.hpp>
#include "strings_utils.h"
using namespace Utils;

extern struct FarStandardFunctions g_FSF;

extern bool IsNamedFoldersCatalogExists(tstring const& catalogName);

tstring Utils::ExtractCatalogName(tstring const& srcPath) {
	return DividePathFilename(srcPath, SLASH_CATS_CHAR, true).second;
}

// a/b, c/d -> a/c/d
// a/b, ../d -> d
// a/b, d -> a/d
// a/b, ./d -> a/d
// a/b, ../../../../../../d -> d
// a/b, a/b/c/d -> a/a/b/c/d
// a/b, a/b/c/../../../d -> a/d
// a/b, a/b/c/../../../d/e/../../../f -> f
// a/b .. -> b
// a/b . -> a/b
tstring Utils::Private::mix_paths(nf::tlist_strings const& s1
								  , nf::tlist_strings const& s2
								  , wchar_t destDelimeter
								  , bool bForMovingShortcuts) {
	nf::tlist_strings dest;
	std::copy(s1.begin(), s1.end(), std::back_inserter(dest));
	if (s2.size() == 1 && *s2.begin() == L"..") {
		if (! bForMovingShortcuts) {
			tstring last_name = dest.back();
			dest.pop_back();
			if (! dest.empty()) dest.pop_back();
			dest.push_back(last_name);
		} else {
			if (! dest.empty()) dest.pop_back();
		}
	} else {
		bool bfirst = ! bForMovingShortcuts;
		BOOST_FOREACH(tstring const& token, s2) {
			if (token != L".") {
				if (token == L".." ) {
					if (! dest.empty()) dest.pop_back();
				} else {
					if (bfirst) {
						if (! dest.empty()) dest.pop_back();
					}
					dest.push_back(token);
					bfirst = false;
				}
			}
		}
	}
	return Private::sequence_to_string(dest, destDelimeter);
}

tstring Utils::Private::sequence_to_string(nf::tlist_strings const& srcSequence, wchar_t destDelimeter) {
	tstring ret;
	BOOST_FOREACH(tstring const& token, srcSequence) {
		ret += destDelimeter + token;
	}
	return ret;
}

//srcCatalog - current catalog, that should be renamed
//targetCatalog - new catalog name, that user has specified 
//destCatalog - full name of new catalog.
//targetCatalog can contain symbols '..' and '.' - parent and current catalog
//it can be started from '/' (this is absolute path) or from any other token (relative path)
bool Utils::ExpandCatalogPath(tstring const &srcCatalog
							  , tstring const& targetCatalog
							  , tstring& destCatalog
							  , bool bForMovingShortcuts)
{
//target catalog is empty
	if (! targetCatalog.size()) return false;  //catalog is not specified
	if (targetCatalog == SLASH_CATS) return false; // name of absolute catalog can't be empty

//absolute target catalog
	if (*(targetCatalog.begin()) == SLASH_CATS_CHAR) { 
		tstring dest = GetCanonicalCatalogName(targetCatalog);
		if (dest == GetCanonicalCatalogName(srcCatalog)) return false;
		if (dest != L"/..") {
			destCatalog.swap(dest);
			return true;
		}
	}
//relative target catalog
	bool bend_slash = (*(targetCatalog.end()-1) == SLASH_CATS_CHAR);

	nf::tlist_strings src_tokens;
	Utils::SplitStringByRegex(Utils::TrimChar(srcCatalog, SLASH_CATS_CHAR).c_str(), src_tokens, SLASH_CATS);
	
	nf::tlist_strings target_tokens;
	Utils::SplitStringByRegex(Utils::TrimChar(targetCatalog, SLASH_CATS_CHAR).c_str(), target_tokens, SLASH_CATS);

	if (target_tokens.size() != 0) {
	//пусть у нас есть каталоги 
	// /1/10/100
	// /1/20/200
	// мы находимся в каталоге 1/20 и даем команду переместить его в 10/100
	// в итоге у нас должен создасться каталог 1/10/100/20/200
		tstring parent_sibling = Utils::DividePathFilename(srcCatalog, SLASH_CATS_CHAR, false).first;

		BOOST_FOREACH(tstring const& token, target_tokens) {
			tstring sibling_catalog = Utils::CombinePath(parent_sibling, token, SLASH_CATS);
			
			if (::IsNamedFoldersCatalogExists(sibling_catalog)) {
				destCatalog = Utils::CombinePath(Utils::CombinePath(parent_sibling, token, SLASH_CATS)
					, ExtractCatalogName(srcCatalog), SLASH_CATS);
			}
			parent_sibling = Utils::CombinePath(parent_sibling, token, SLASH_CATS);
		}
		if (! destCatalog.empty()) return true;
	} 
	destCatalog = Private::mix_paths(src_tokens, target_tokens, SLASH_CATS_CHAR, bForMovingShortcuts);
	return true;
}

tstring Utils::MakePathCompact(tstring const &srcCatalog, bool bKeepExceededColons) {
	// make path compact (all ".." will be collapsed)
	nf::tlist_strings src_tokens;
	Utils::SplitStringByRegex(Utils::TrimChar(srcCatalog, SLASH_CATS_CHAR).c_str(), src_tokens, SLASH_CATS);

	unsigned int count_reduced_names = 0;
	nf::tlist_strings dest_tokens;
	BOOST_REVERSE_FOREACH(tstring const& token, src_tokens) {
		if (token == L"..") {
			++count_reduced_names;
		} else if (token == L".") {
			//just skip it
		} else {
			if (0 != count_reduced_names) {
				--count_reduced_names;
			} else {
				dest_tokens.push_front(token);
			}
		}
	}
	if (bKeepExceededColons) {
		while (0 != count_reduced_names) {
			dest_tokens.push_front(L"..");
			--count_reduced_names;
		}
	}
	return Private::sequence_to_string(dest_tokens, SLASH_CATS_CHAR);
}


tstring Utils::GetCanonicalCatalogName(tstring const& srcCatalog) {
//make path compact
	tstring dest = MakePathCompact(srcCatalog, false);
	if (! dest.empty()) Utils::AddLeadingCharIfNotExistsOnPlace(dest, SLASH_CATS);
//convert to lower case
	tstring ret;
	ret.reserve(dest.size());
	BOOST_FOREACH(wchar_t ch, dest) {
		ret.push_back(g_FSF.LLower(ch));
	}
	return ret;	
	//boost::algorithm::to_lower(dest, std::locale("")); //!TODO: replace by smth else to reduce size ??
}

bool Utils::PrepareMovingShortcut(nf::tshortcut_info const &srcSh, tstring const &targetPath, nf::tshortcut_info &destSh) {
	//we are going to move/copy shortcut to specified path
	//if path is finished with "/", then whole path is catalog
	//otherwise, last name is path is new name of shortcut
	//path can contain ".." and "."
	destSh = srcSh;
	if (! targetPath.empty()) {
		tpair_strings kvp = Utils::DividePathFilename(targetPath, SLASH_CATS_CHAR, false);
		Utils::RemoveLeadingCharsOnPlace(kvp.second, SLASH_CATS_CHAR);

		if (Utils::IsCatalogPathRelated(targetPath)) {
			kvp.first = MakePathCompact(kvp.first, true);
			if (! kvp.first.empty()) {
				destSh.catalog = destSh.catalog + kvp.first;
			}
		} else {
			destSh.catalog.swap(kvp.first);
		}

		if (! kvp.second.empty()) {
			if (kvp.second == L"..") {
				destSh.catalog += L"/..";
			} else {
				destSh.shortcut.swap(kvp.second);
			}
		}
	}

	if (! destSh.catalog.empty()) {
		Utils::ExpandCatalogPath(srcSh.catalog, destSh.catalog
			, destSh.catalog
			, true //moving/copying shortcuts
		);
	}

	return true;
}
