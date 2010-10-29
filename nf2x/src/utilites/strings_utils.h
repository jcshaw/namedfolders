/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include <list>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

namespace Utils
{
	struct CmpStringLessCI
		: public std::binary_function<tstring const&, tstring const&, bool>
	{
		bool operator()(tstring const& s1, tstring const &s2) const
		{
			return lstrcmpi(s1.c_str(), s2.c_str()) < 0;
		}
	};

	struct CmpStringEqualCI 
		: public std::binary_function<tstring const&, tstring const&, bool>
	{
		bool operator()(tstring const& s1, tstring const &s2)
		{
			return ! lstrcmpi(s1.c_str(), s2.c_str()) ? 1 : 0;
		}
	};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//объединить строки; дл€ первой (первых двух) задана фиксированна€ ширина
	tstring CombineStrings(tstring const& Value1, tstring const& Value2, size_t Width1);
	tstring CombineStrings(tstring const& Value1, tstring const& Value2, tstring Value3, size_t Width1, size_t Width2);
	tstring CombinePath(tstring const& Path1, tstring const& Path2, TCHAR const* Delimiter);


	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// разделить строку на токены по регул€рному выражению
	template <class Container>
		void SplitStringByRegex(tstring const& SrcStr, Container &Dest, TCHAR const* RegExDelimiter)
	{
		nf::tregex reg(RegExDelimiter);
		nf::tregex_token_iterator p(SrcStr.begin(), SrcStr.end(), reg, -1);
		nf::tregex_token_iterator end;
		while (p != end)
		{ 
			Dest.push_back(*p);
			++p;
		}
	}

//работа с пут€ми
	tstring ExtractParentDirectory(tstring const& SrcDir);
	void DividePathFilename(tstring const &src, tstring &path
		, tstring &filename
		, TCHAR SlashChar
		, bool bRemoveTrailingChar);
	std::pair<tstring, tstring> DivideString(tstring const& SrcStr, TCHAR ch);

//строки
	tstring ReplaceStringAll(tstring SrcStr, tstring const& FromStr, tstring const& ToStr);
	void RemoveLeadingChars(tstring &str, TCHAR ch);
	void RemoveTrailingChars(tstring &str, TCHAR ch);
	void add_leading_char_if_not_exists(tstring &str, TCHAR const* ch);
	void add_trailing_char_if_not_exists(tstring &str, TCHAR const* ch);
	bool iFindFirst(tstring const& SrcStr, tstring const& SubStr);

//кодировки
	namespace CodePage 
	{
		void OemToAnsi(tstring & src);
		void AnsiToOem(tstring & src);

		//функции, преобразующие строку "на месте"
		inline void OemToInternal(tstring& src) { return CodePage::OemToAnsi(src); }
		inline void InternalToOem(tstring& src)	{ CodePage::AnsiToOem(src);}
	}


//функции, возвращающие преобразованную копию строк
	inline tstring GetOemToInternal(tstring const& src)
	{
		tstring s = src;
		CodePage::OemToInternal(s);
		return s;
	}
	inline tstring GetInternalToOem(tstring const& src) 
	{ 
		tstring s = src;
		CodePage::InternalToOem(s);
		return s;
	}
	inline tstring GetInternalToRegistry(tstring const& src) { return GetInternalToOem(src); }
	inline tstring GetRegistryToInternal(tstring const& src) { return GetOemToInternal(src); }

	inline bool IsAliasIsDisk(tstring ShortcutName)
	{
		return (ShortcutName.size() == 2 && ShortcutName[1] == _T(':'));
	}

}//Utils
