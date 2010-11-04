/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
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

tstring Utils::ExtractCatalogName(tstring const& srcPath) {
	tstring path;
	tstring filename;
	DividePathFilename(srcPath, path, filename, SLASH_CATS_CHAR, true);
	RemoveLeadingCharsOnPlace(filename, SLASH_CATS_CHAR);
	return filename;
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
tstring Utils::Private::mix_paths(std::list<tstring> const& s1
								  , std::list<tstring> const& s2
								  , wchar_t destDelimeter
								  , bool bForMovingShortcuts) {
	std::list<tstring> dest;
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

tstring Utils::Private::sequence_to_string(std::list<tstring> const& srcSequence, wchar_t destDelimeter) {
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

	std::list<tstring> src_tokens;
	Utils::SplitStringByRegex(Utils::TrimChar(srcCatalog, SLASH_CATS_CHAR).c_str(), src_tokens, SLASH_CATS);
	
	std::list<tstring> target_tokens;
	Utils::SplitStringByRegex(Utils::TrimChar(targetCatalog, SLASH_CATS_CHAR).c_str(), target_tokens, SLASH_CATS);

	destCatalog = Private::mix_paths(src_tokens, target_tokens, SLASH_CATS_CHAR, bForMovingShortcuts);
	return true;
}

tstring Utils::MakePathCompact(tstring const &srcCatalog, bool bKeepExceededColons) {
	// make path compact (all ".." will be collapsed)
	std::list<tstring> src_tokens;
	Utils::SplitStringByRegex(Utils::TrimChar(srcCatalog, SLASH_CATS_CHAR).c_str(), src_tokens, SLASH_CATS);

	unsigned int count_reduced_names = 0;
	std::list<tstring> dest_tokens;
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
		tstring name;
		tstring dest_catalog;
		Utils::DividePathFilename(targetPath, dest_catalog, name, SLASH_CATS_CHAR, false);
		Utils::RemoveLeadingCharsOnPlace(name, SLASH_CATS_CHAR);

		if (Utils::IsCatalogPathRelated(targetPath)) {
			dest_catalog = MakePathCompact(dest_catalog, true);
			if (! dest_catalog.empty()) {
				destSh.catalog = destSh.catalog + dest_catalog;
			}
		} else {
			destSh.catalog.swap(dest_catalog);
		}

		if (! name.empty()) {
			if (name == L"..") {
				destSh.catalog += L"/..";
			} else {
				destSh.shortcut.swap(name);
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