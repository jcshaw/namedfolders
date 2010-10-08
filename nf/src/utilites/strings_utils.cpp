/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "strings_utils.h"
#include <vector>
#include <cassert>
#include <boost/bind.hpp>
#include <Shlwapi.h>


void Utils::DividePathFilename(tstring const &src
						  , tstring &path
						  , tstring &filename
						  , wchar_t slashChar
						  , bool bRemoveTrailingChar)
{
	path = src;
	if (bRemoveTrailingChar) 
		RemoveTrailingChars(path, slashChar);
	
	tstring::size_type npos = path.find_last_of(slashChar);
	if (npos != tstring::npos) {
		filename.assign(path, npos, path.size()-npos);
		path.erase(npos, path.size()-npos);
	} else {
		path.clear();
		filename = src;
	}

	if (filename == LEVEL_UP_TWO_POINTS) {
		path += slashChar + filename;
		filename.clear();
	}

}

std::pair<tstring, tstring> Utils::DivideString(tstring const& srcStr, wchar_t ch)
{
	std::pair<tstring, tstring> result;
	size_t npos = srcStr.find_first_of(ch);
	if (npos != srcStr.npos) {
		result.first.assign(srcStr, 0, npos);
		result.second.assign(srcStr, npos + 1, srcStr.size() - npos);
	} else {
		result.first = srcStr;
		result.second = L"";
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////
tstring Utils::ExtractParentDirectory(tstring const& srcDir)
{	//extracts parent directory from srcDir 
	//returns empty string if there are no parent directory
	tstring result = srcDir;
	tstring::reverse_iterator p = result.rbegin();
	if (*p == SLASH_DIRS_CHAR || *p == SLASH_CATS_CHAR) ++p;
	while (p != result.rend()) {
		if (*p == SLASH_DIRS_CHAR || *p == SLASH_CATS_CHAR) {
			tstring::iterator pd = p.base();
			result.erase(pd, result.end());
			return result;
		}
		++p;
	}
	return L"";
}

//////////////////////////////////////////////////////////////////////////
inline tstring get_compact_path(tstring const& srcStr, size_t MaxSize)
{
	if (srcStr.size() <= MaxSize) return srcStr;
	nf::tautobuffer_char buf(srcStr.size() + 1);
	lstrcpy(&buf[0], srcStr.c_str());
	PathCompactPathEx(&buf[0], srcStr.c_str(), static_cast<unsigned int>(MaxSize), 0);
	return &buf[0];
}


tstring Utils::CombineStrings(tstring const& Value1, tstring const& Value2, size_t Width1)
{	//!TODO: попробовать реализовать через stream - код будет короче, как насчет размера плагина
	tstring result;
	result.reserve(Width1 + Value2.size() + 1);
	result = get_compact_path(Value1, Width1);
	size_t len1 = Width1 + 1 - (Value1.size() < Width1 ? Value1.size() : result.size());
	for (size_t i = 0; i < len1; ++i) result += L" ";
	result += Value2;
	return result;
}

tstring Utils::CombineStrings(tstring const& Value1, tstring const& Value2, tstring Value3, size_t Width1, size_t Width2)
{
	tstring result;
	result.reserve(Width1 + Width2 + Value3.size() + 2);

	result = get_compact_path(Value1, Width1);
	size_t len1 = Width1 + 1 - (Value1.size() < Width1 ? Value1.size() : result.size());
	for (size_t i = 0; i < len1; ++i) result += L" ";
	tstring prom = get_compact_path(Value2, Width2);
	result += prom;

	size_t len2 = Width2 + 1 - (Value2.size() < Width2 ? Value2.size() : prom.size());
	for (size_t i = 0; i < len2; ++i) result += L" ";
	result += Value3;

	return result;
}

tstring Utils::ReplaceStringAll(tstring srcStr, tstring const& fromStr, tstring const& toStr) //!TODO :убрать
{	
	size_t off = 0;
	size_t pos = 0;
	while ((pos = srcStr.find(fromStr, off)) != tstring::npos) {
		srcStr.replace(pos, fromStr.size(), toStr);
		off = pos + toStr.size();
	}
	//	boost::replace_all(srcStr, FromStr, ToStr);	//5 kb
	return srcStr;
}

void Utils::RemoveLeadingChars(tstring &str, wchar_t ch) {
	boost::trim_left_if(str, boost::bind(std::equal_to<wchar_t>(), _1, ch));
}
void Utils::RemoveTrailingChars(tstring &str, wchar_t ch) {
	boost::trim_right_if(str, boost::bind(std::equal_to<wchar_t>(), _1, ch));
}

void Utils::AddTrailingCharIfNotExists(tstring &Str, wchar_t const* ch) {
	if (! boost::ends_with(Str, ch)) 
		Str += ch;
}
void Utils::AddLeadingCharIfNotExists(tstring &Str, wchar_t const* ch) {
	if (! boost::starts_with(Str, ch)) 
		Str.insert(0, ch);
}

bool Utils::iFindFirst(tstring const& SrcStr, tstring const& SubStr) {
	return StrStrI(SrcStr.c_str(), SubStr.c_str()) != NULL;
	//return  boost::ifind_first(srcStr, SubStr); //additional 20 кб
}

tstring Utils::CombinePath(tstring const& Path1, tstring const& Path2, wchar_t const* Delimiter)
{
	if ( (! boost::starts_with(Path2, Delimiter)) && (! boost::ends_with(Path1, Delimiter))) {
		return Path1 + Delimiter + Path2;
	}
	return Path1 + Path2;	//delimeter already exists
}


tstring Utils::ExtractCatalogName(tstring const& srcPath) {
	tstring path;
	tstring filename;
	DividePathFilename(srcPath, path, filename, SLASH_CATS_CHAR, true);
	RemoveLeadingChars(filename, SLASH_CATS_CHAR);
	return filename;
}


bool Utils::ExpandCatalogPath(tstring const &srcCatalog
							  , tstring targetCatalog
							  , tstring& destCatalog)
{	//expand ".." in targetCatalog
	//"1/2/3/" ".." -> "1/2/../3" = "1/3"
	//"1/2/3/4/" "../../7" -> "1/2/3/../../7"="1/7"
	//"1/2/3/4/" "../../7" -> "1/2/3/../../7/4"="1/7/4"
	//"1/2/3/" "/5/6" -> "/5/6"
	//"1/2/3/" "/5/6/" -> /5/6/3"
	//"1/2/3/" "5/6" -> 1/2/5/6
	//"1/2/3/" "5/6/" -> 1/2/5/6/3
	const tstring ROOT(L"../");
	if (! targetCatalog.size()) return false;

	bool bRelatedRoot = (*(targetCatalog.begin()) == SLASH_CATS_CHAR);
	bool bRelatedCurrentCatalog = (*(targetCatalog.end()-1) == SLASH_CATS_CHAR);
	Utils::AddTrailingCharIfNotExists(targetCatalog, SLASH_CATS);
	if (targetCatalog.size() >= ROOT.size()) {
		tstring::const_reverse_iterator p1 = targetCatalog.rbegin();  
		tstring::const_reverse_iterator p2 = ROOT.rbegin();
		while (p2 != ROOT.rend()) {
			wchar_t const& ch1 = *p1;
			wchar_t const& ch2 = *p2;
			if (*p1 == *p2) {
				++p1;
				++p2;
			} else break;
		}
		if (p2 == ROOT.rend()) bRelatedCurrentCatalog = true;
	}

	tstring src_path;
	tstring src_catalog_name;
	Utils::DividePathFilename(srcCatalog, src_path, src_catalog_name, SLASH_CATS_CHAR, true);
	Utils::AddTrailingCharIfNotExists(src_path, SLASH_CATS);

	if (bRelatedRoot) {
		destCatalog.swap(targetCatalog);
	} else {
		destCatalog.swap(src_path);
		destCatalog.append(targetCatalog);
	} 
	if (bRelatedCurrentCatalog) {
		Utils::RemoveTrailingChars(destCatalog, SLASH_CATS_CHAR);
		destCatalog.append(src_catalog_name);
	}

	destCatalog = MakePathCompact(destCatalog, ROOT);

	return true;
}

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

