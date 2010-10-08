#include "stdafx.h"
#include "catalog_names.h"

#include <locale> 
#include <boost/foreach.hpp>
#include "strings_utils.h"
using namespace Utils;

tstring Utils::ExtractCatalogName(tstring const& srcPath) {
	tstring path;
	tstring filename;
	DividePathFilename(srcPath, path, filename, SLASH_CATS_CHAR, true);
	RemoveLeadingChars(filename, SLASH_CATS_CHAR);
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
tstring Utils::Private::mix_paths(std::list<tstring> const& s1, std::list<tstring> const& s2, wchar_t destDelimeter) {
	std::list<tstring> dest;
	std::copy(s1.begin(), s1.end(), std::back_inserter(dest));
	if (s2.size() == 1 && *s2.begin() == L"..") {
		tstring last_name = dest.back();
		dest.pop_back();
		if (! dest.empty()) dest.pop_back();
		dest.push_back(last_name);
	} else {
		bool bfirst = true;
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
	tstring ret;
	BOOST_FOREACH(tstring const& token, dest) {
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
							  , tstring& destCatalog)
{
//target catalog is empty
	if (! targetCatalog.size()) return false;  //catalog is not specified
	if (targetCatalog == SLASH_CATS) return false; // name of absolute catalog can't be empty

//absolute target catalog
	if (*(targetCatalog.begin()) == SLASH_CATS_CHAR) { 
		tstring dest = GetCanonicalCatalogName(targetCatalog);
		if (dest == GetCanonicalCatalogName(srcCatalog)) return false;
		destCatalog.swap(dest);
		return true;
	}
//relative target catalog
	bool bend_slash = (*(targetCatalog.end()-1) == SLASH_CATS_CHAR);

	std::list<tstring> src_tokens;
	Utils::SplitStringByRegex(srcCatalog.c_str(), src_tokens, SLASH_CATS);
	
	std::list<tstring> target_tokens;
	Utils::SplitStringByRegex(targetCatalog.c_str(), target_tokens, SLASH_CATS);

	destCatalog = Private::mix_paths(src_tokens, target_tokens, SLASH_CATS_CHAR);
	return true;
}

// bool Utils::ExpandCatalogPath(tstring const &srcCatalog
// 							  , tstring targetCatalog
// 							  , tstring& destCatalog)
// {	//expand ".." in targetCatalog
// 	//"1/2/3/" ".." -> "1/2/../3" = "1/3"
// 	//"1/2/3/4/" "../../7" -> "1/2/3/../../7"="1/7"
// 	//"1/2/3/4/" "../../7" -> "1/2/3/../../7/4"="1/7/4"
// 	//"1/2/3/" "/5/6" -> "/5/6"
// 	//"1/2/3/" "/5/6/" -> /5/6/3"
// 	//"1/2/3/" "5/6" -> 1/2/5/6
// 	//"1/2/3/" "5/6/" -> 1/2/5/6/3
// 	const tstring ROOT(L"../");
// 	if (! targetCatalog.size()) return false;
// 
// 	bool bRelatedRoot = (*(targetCatalog.begin()) == SLASH_CATS_CHAR);
// 	bool bRelatedCurrentCatalog = (*(targetCatalog.end()-1) == SLASH_CATS_CHAR);
// 	Utils::AddTrailingCharIfNotExists(targetCatalog, SLASH_CATS);
// 	if (targetCatalog.size() >= ROOT.size()) {
// 		tstring::const_reverse_iterator p1 = targetCatalog.rbegin();  
// 		tstring::const_reverse_iterator p2 = ROOT.rbegin();
// 		while (p2 != ROOT.rend()) {
// 			wchar_t const& ch1 = *p1;
// 			wchar_t const& ch2 = *p2;
// 			if (*p1 == *p2) {
// 				++p1;
// 				++p2;
// 			} else break;
// 		}
// 		if (p2 == ROOT.rend()) bRelatedCurrentCatalog = true;
// 	}
// 
// 	tstring src_path;
// 	tstring src_catalog_name;
// 	Utils::DividePathFilename(srcCatalog, src_path, src_catalog_name, SLASH_CATS_CHAR, true);
// 	Utils::AddTrailingCharIfNotExists(src_path, SLASH_CATS);
// 
// 	if (bRelatedRoot) {
// 		destCatalog.swap(targetCatalog);
// 	} else {
// 		destCatalog.swap(src_path);
// 		destCatalog.append(targetCatalog);
// 	} 
// 	if (bRelatedCurrentCatalog) {
// 		Utils::RemoveTrailingChars(destCatalog, SLASH_CATS_CHAR);
// 		destCatalog.append(src_catalog_name);
// 	}
// 
// 	destCatalog = MakePathCompact(destCatalog, ROOT);
// 
// 	return true;
// }

tstring Utils::MakePathCompact(tstring const &srcCatalog, tstring const &root) {
	// make path compact (all ".." will be collapsed)
	tstring destCatalog = srcCatalog;
	Utils::AddLeadingCharIfNotExists(destCatalog, SLASH_CATS);
	tstring::size_type npos;
	while ((npos = destCatalog.find(root)) != tstring::npos) {
		tstring::size_type npos2 = 
			destCatalog.find_last_of(SLASH_CATS_CHAR, npos-root.size());
		if ((npos2 == tstring::npos)  || npos2 > npos) {
			destCatalog = L"";
			return destCatalog;	//дальше подниматься некуда
		}
		destCatalog.erase(npos2, npos-npos2+root.size()-1);
	}

	Utils::RemoveTrailingChars(destCatalog, SLASH_CATS_CHAR);
	return destCatalog;
}


tstring Utils::GetCanonicalCatalogName(tstring const& srcCatalog) {
// 	//!TODO: makepathcompact, lowercase	
	tstring dest = MakePathCompact(srcCatalog, L"../");
	if (*(dest.begin()) != SLASH_CATS_CHAR) dest = SLASH_CATS_CHAR + dest;
 	
	return dest; //!TODOstd::tolower(dest, std::locale(""));
}