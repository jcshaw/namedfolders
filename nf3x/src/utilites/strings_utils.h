/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include <list>
#include <boost/bind.hpp>

namespace Utils {
// 	struct CmpStringLessCI
// 		: public std::binary_function<tstring const&, tstring const&, bool> 
// 	{
// 		bool operator()(tstring const& s1, tstring const &s2) const {
// 			return lstrcmpi(s1.c_str(), s2.c_str()) < 0;
// 		}
// 	};
// 
	struct CmpStringEqualCI 
		: public std::binary_function<tstring const&, tstring const&, bool>
	{
		bool operator()(tstring const& s1, tstring const &s2) {
			return ! lstrcmpi(s1.c_str(), s2.c_str()) ? 1 : 0;
		}
	};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//combine two strings; fixed width is specified for first one 
	tstring CombineStrings(tstring const& Value1, tstring const& Value2, size_t Width1);
	tstring CombineStrings(tstring const& Value1, tstring const& Value2, tstring Value3, size_t Width1, size_t Width2);
	tstring CombinePath(tstring const& Path1, tstring const& Path2, wchar_t const* Delimiter);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// splits string on tokens according reqexp
	template <class Container>
		void SplitStringByRegex(tstring const& SrcStr, Container &Dest, wchar_t const* RegExDelimiter) {
		nf::tregex reg(RegExDelimiter);
		nf::tregex_token_iterator p(SrcStr.begin(), SrcStr.end(), reg, -1);
		nf::tregex_token_iterator end;
		while (p != end) { 
			Dest.push_back(*p);
			++p;
		}
	}

// path functions
	tstring ExtractParentDirectory(tstring const& SrcDir);
	void DividePathFilename(tstring const &src, tstring &path, tstring &filename, wchar_t SlashChar, bool bRemoveTrailingChar);
	tpair_strings DivideString(tstring const& SrcStr, wchar_t ch);

// string helper functions
	tstring ReplaceStringAll(tstring SrcStr, tstring const& FromStr, tstring const& ToStr);
	void RemoveLeadingCharsOnPlace(tstring &str, wchar_t charToRemove);
	void RemoveTrailingCharsOnPlace(tstring &str, wchar_t charToRemove);
	inline tstring RemoveLeadingChars(tstring const &str, wchar_t charToRemove) {
		tstring ret = str;
		RemoveLeadingCharsOnPlace(ret, charToRemove);
		return ret;
	}
	inline tstring RemoveTrailingChars(tstring const &str, wchar_t charToRemove) {
		tstring ret = str;
		RemoveTrailingCharsOnPlace(ret, charToRemove);
		return ret;
	}
	inline tstring TrimChar(tstring const& srcStr, wchar_t charToRemove) {
		tstring ret = srcStr;
		RemoveLeadingCharsOnPlace(ret, charToRemove);
		RemoveTrailingCharsOnPlace(ret, charToRemove);
		return ret;
	}

	void AddLeadingCharIfNotExistsOnPlace(tstring &str, wchar_t const* ch);
	void AddTrailingCharIfNotExists(tstring &str, wchar_t const* ch);

	bool iFindFirst(tstring const& SrcStr, tstring const& SubStr);

	inline bool IsAliasIsDisk(tstring ShortcutName) {
		return (ShortcutName.size() == 2 && ShortcutName[1] == L':');
	}

	// expand single %s to srcSubStr
	inline tstring SubstituteSubstringLikePrintf(tstring const& srcStr, tstring const& srcSubStr) {
		return Utils::ReplaceStringAll(srcStr, L"%s", srcSubStr);
	}

	inline tstring SubstituteSubstringLikePrintf(wchar_t const* srcStr, wchar_t const* srcSubStr) {
		return Utils::ReplaceStringAll(srcStr, L"%s", srcSubStr);
	}

	inline bool IsLastCharEqualTo(tstring const& srcStr, wchar_t charToEqual) {
		if (srcStr.empty()) return false;
		return *(--srcStr.end()) == charToEqual;
	}
}//Utils
