/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "strings_utils.h"

#include "Parser.h"
#include <vector>
#include <cassert>
#include <boost/bind.hpp>
#include <Shlwapi.h>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include "settings.h"

namespace {
	inline tstring get_compact_path(tstring const& srcStr, size_t MaxSize) {
		if (srcStr.size() <= MaxSize) return srcStr;
		nf::tautobuffer_char buf(srcStr.size() + 1);
		lstrcpy(&buf[0], srcStr.c_str());
		PathCompactPathEx(&buf[0], srcStr.c_str(), static_cast<unsigned int>(MaxSize), 0);
		return &buf[0];
	}
}

void Utils::DividePathFilename(tstring const &src
						  , tstring &path
						  , tstring &filename
						  , wchar_t slashChar
						  , bool bRemoveTrailingChar)
{
	path = src;
	if (bRemoveTrailingChar) 
		RemoveTrailingCharsOnPlace(path, slashChar);
	
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

tpair_strings Utils::DivideString(tstring const& srcStr, wchar_t ch) {
	tpair_strings result;
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

tstring Utils::ExtractParentDirectory(tstring const& srcDir) {	
//extracts parent directory from srcDir 
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

tstring Utils::CombineStrings(tstring const& Value1, tstring const& Value2, size_t Width1) {	
	tstring result;
	result.reserve(Width1 + Value2.size() + 1);
	tstring compact_path = get_compact_path(Value1, Width1);
	result += compact_path;
	size_t len1 = Width1 + 1 - (Value1.size() < Width1 ? Value1.size() : Width1);
 	for (unsigned int i = 0; i < len1; ++i) result += L" ";
 	result += Value2;
	return result;
}

tstring Utils::CombineStrings(tstring const& Value1, tstring const& Value2, tstring Value3, size_t Width1, size_t Width2) {
	tstring result;
	result.reserve(Width1 + Width2 + Value3.size() + 2);
	tstring compact_path = get_compact_path(Value1, Width1);
	result += compact_path;
	size_t len1 = Width1 + 1 - (Value1.size() < Width1 ? Value1.size() : result.size());
	for (size_t i = 0; i < len1; ++i) result += L" ";
	tstring prom = get_compact_path(Value2, Width2);
	result += prom;

	size_t len2 = Width2 + 1 - (Value2.size() < Width2 ? Value2.size() : prom.size());
	for (size_t i = 0; i < len2; ++i) result += L" ";
	result += Value3;

	return result;
}

tstring Utils::ReplaceStringAll(tstring srcStr, tstring const& fromStr, tstring const& toStr) {	
	size_t off = 0;
	size_t pos = 0;
	while ((pos = srcStr.find(fromStr, off)) != tstring::npos) {
		srcStr.replace(pos, fromStr.size(), toStr);
		off = pos + toStr.size();
	}
	//	boost::replace_all(srcStr, FromStr, ToStr);	//5 kb
	return srcStr;
}

void Utils::RemoveSingleLeadingCharOnPlace(tstring &str, wchar_t charToRemove) {
	if (! str.empty()) {
		if (str[0] == charToRemove) {
			str.assign(str, 1, str.size() - 1);
		}
	}
}

void Utils::RemoveSingleTrailingCharOnPlace(tstring &str, wchar_t charToRemove) {
	if (! str.empty()) {
		if (str[str.size() - 1] == charToRemove) {
			str.assign(str, 0, str.size() - 1);
		}
	}
}

void Utils::RemoveLeadingCharsOnPlace(tstring &str, wchar_t charToRemove) {
	boost::trim_left_if(str, boost::bind(std::equal_to<wchar_t>(), _1, charToRemove));
}
void Utils::RemoveTrailingCharsOnPlace(tstring &str, wchar_t charToRemove) {
	boost::trim_right_if(str, boost::bind(std::equal_to<wchar_t>(), _1, charToRemove));
}

void Utils::AddTrailingCharIfNotExists(tstring &Str, wchar_t const* ch) {
	if (! boost::ends_with(Str, ch)) {
		Str += ch;
	}
}
void Utils::AddLeadingCharIfNotExistsOnPlace(tstring &Str, wchar_t const* ch) {
	if (! boost::starts_with(Str, ch)) {
		Str.insert(0, ch);
	}
}

bool Utils::IsLastCharEqualTo(tstring const& srcStr, wchar_t charToEqual) {
	wchar_t end[2] = {charToEqual, 0};
	return boost::ends_with(srcStr, &end[0]);
}

bool Utils::iFindFirst(tstring const& SrcStr, tstring const& SubStr) {
	return StrStrI(SrcStr.c_str(), SubStr.c_str()) != NULL;
	//return  boost::ifind_first(srcStr, SubStr); //additional 20 кб
}

tstring Utils::CombinePath(tstring const& Path1, tstring const& Path2, wchar_t const* Delimiter) {
	if ( (! boost::starts_with(Path2, Delimiter)) && (! boost::ends_with(Path1, Delimiter)) && (!Path1.empty()) && (!Path2.empty())) {
		return Path1 + Delimiter + Path2;
	}
	return Path1 + Path2;	//delimeter already exists
}

namespace Utils {
namespace Private {
	//Заменяет в строке srcStr последовательность символов srcCh длиной не менее minCount на byString
	tstring replace_sequence_chars(tstring const& srcStr, wchar_t srcCh, tstring const& byString, unsigned int minCount, unsigned int startFrom) {
		tstring buf;
		tstring dest;
		dest.reserve(srcStr.size());

		for (unsigned int i = 0; i < startFrom; ++i) dest.push_back(srcStr[i]);

		for (unsigned int i = startFrom; i < srcStr.size(); ++i) {
			wchar_t const& ch = srcStr[i];
			if (ch == srcCh) {
				if (buf.size() < minCount) {
					buf.push_back(ch);
				}
			} else {
				if (! buf.empty()) {
					dest.insert(dest.end(), buf.begin(), buf.end());
					buf.clear();
				}
				dest.push_back(ch);
			}
		}
		dest.insert(dest.end(), buf.begin(), buf.end());
		return dest;
	}
} //Private
} //nf

tstring Utils::SubstituteSearchMetachars(tstring const& srcPath) {
	tstring s = (boost::starts_with(srcPath, tstring(L"**")) || boost::starts_with(srcPath, tstring(L"..*")) ) 
		? L"\\" + srcPath
		: srcPath;

	bool ballow_short_path_commands = (nf::CSettings::GetInstance().GetValue(nf::ST_ALLOW_ABBREVIATED_SYNTAX_FOR_DEEP_SEARCH) != 0);
		//replace sequence of \\\ by single \
		//	there is exception here: net:\\networkpath and \\networkpath shouldn't be transformed
		//replace sequence of ... by ..
	if (! ballow_short_path_commands) {
		s = Utils::Private::replace_sequence_chars(s, L'.', L"..", 2, 0);

		int first_ch = nf::Parser::GetNetworkPathPrefixLength(s);
		s = Utils::Private::replace_sequence_chars(s, L'\\', L"\\", 1, first_ch);
		Utils::RemoveTrailingCharsOnPlace(s, SLASH_DIRS_CHAR);
	}

	s = Utils::ReplaceStringAll(s, MC_DEEP_REVERSE_SEARCH_LONG, MC_DEEP_REVERSE_SEARCH_SHORT);
	s = Utils::ReplaceStringAll(s, MC_DEEP_DIRECT_SEARCH_LONG, MC_DEEP_DIRECT_SEARCH_SHORT);
	s = Utils::ReplaceStringAll(s, tstring(MC_SEARCH_FORWARD_LONG) + SLASH_DIRS_CHAR, tstring(MC_SEARCH_FORWARD_SHORT) + SLASH_DIRS_CHAR);
	s = Utils::ReplaceStringAll(s, MC_SEARCH_BACKWORD_LONG, MC_SEARCH_BACKWORD_SHORT);

	if (ballow_short_path_commands) {
		s = Utils::ReplaceStringAll(s, LEVEL_UP_TWO_POINTS, MC_SEARCH_BACKWORD_SHORT_WITHOUT_SLASH);
	} else {
		s = Utils::ReplaceStringAll(s, L".", L"");
	}
		//cd:nf.. -> .. is ignored; 
		//cd:nf\.. -> .. works 
	return s;	
}

void Utils::DivideDiskPath(tstring const &src, tstring &destDisk, tstring &destPath) {
	if (src.size() < 2) {
		destDisk = src;
		destPath = L"";
	} else {
		destDisk.assign(src, 0, 2);
		destPath.assign(src, 2, src.size() - 2);
	}
}