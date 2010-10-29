/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
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
						  , TCHAR SlashChar
						  , bool bRemoveTrailingChar)
{
	path = src;
	if (bRemoveTrailingChar) RemoveTrailingChars(path, SlashChar);
	
	tstring::size_type npos = path.find_last_of(SlashChar);
	if (npos != tstring::npos) 
	{
		filename.assign(path, npos, path.size()-npos);
		path.erase(npos, path.size()-npos);
	} else {
		path.clear();
		filename = src;
	}

	if (filename == LEVEL_UP_TWO_POINTS)
	{
		path += SlashChar + filename;
		filename.clear();
	}

}

std::pair<tstring, tstring> Utils::DivideString(tstring const& SrcStr, TCHAR ch)
{
	std::pair<tstring, tstring> result;
	size_t npos = SrcStr.find_first_of(ch);
	if (npos != SrcStr.npos)
	{
		result.first.assign(SrcStr, 0, npos);
		result.second.assign(SrcStr, npos + 1, SrcStr.size() - npos);
	} else {
		result.first = SrcStr;
		result.second = _T("");
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////
tstring Utils::ExtractParentDirectory(tstring const& SrcDir)
{	//извлекаем из переданного пути родительскую директори
	//если ее нет, возвращаем пустую строку
	tstring result = SrcDir;
	tstring::reverse_iterator p = result.rbegin();
	if (*p == SLASH_DIRS_CHAR || *p == SLASH_CATS_CHAR) ++p;
	while (p != result.rend())
	{
		if (*p == SLASH_DIRS_CHAR || *p == SLASH_CATS_CHAR) 
		{
			tstring::iterator pd = p.base();
			result.erase(pd, result.end());
			return result;
		}
		++p;
	}
	return _T("");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// oem & ansi
void Utils::CodePage::OemToAnsi(tstring & src)
{
	nf::tautobuffer_char sANSI(src.size() + 1);
	if (::OemToChar(src.c_str(), &sANSI[0])) src = &sANSI[0];
}

void Utils::CodePage::AnsiToOem(tstring & src)
{
	nf::tautobuffer_char sOEM(src.size() + 1);
	if (::CharToOem(src.c_str(), &sOEM[0])) src = &sOEM[0];
}

//////////////////////////////////////////////////////////////////////////
inline tstring get_compact_path(tstring const& SrcStr, size_t MaxSize)
{
	if (SrcStr.size() <= MaxSize) return SrcStr;
	nf::tautobuffer_char buf(SrcStr.size() + 1);
	lstrcpy(&buf[0], SrcStr.c_str());
	//tstring value;
	//value.assign(SrcStr, 0, MaxSize);
	PathCompactPathEx(&buf[0], SrcStr.c_str(), static_cast<unsigned int>(MaxSize), 0);
	return &buf[0];
}


tstring Utils::CombineStrings(tstring const& Value1, tstring const& Value2, size_t Width1)
{	//!TODO: попробовать реализовать через stream - код будет короче, как насчет размера плагина
	tstring result;
	result.reserve(Width1 + Value2.size() + 1);
	result = get_compact_path(Value1, Width1);
	size_t len1 = Width1 + 1 - (Value1.size() < Width1 ? Value1.size() : result.size());
	for (size_t i = 0; i < len1; ++i) result += _T(" ");
	result += Value2;
	return result;
}

tstring Utils::CombineStrings(tstring const& Value1, tstring const& Value2, tstring Value3, size_t Width1, size_t Width2)
{
	tstring result;
	result.reserve(Width1 + Width2 + Value3.size() + 2);

	result = get_compact_path(Value1, Width1);
	size_t len1 = Width1 + 1 - (Value1.size() < Width1 ? Value1.size() : result.size());
	for (size_t i = 0; i < len1; ++i) result += _T(" ");
	tstring prom = get_compact_path(Value2, Width2);
	result += prom;

	size_t len2 = Width2 + 1 - (Value2.size() < Width2 ? Value2.size() : prom.size());
	for (size_t i = 0; i < len2; ++i) result += _T(" ");
	result += Value3;

	return result;
}

//////////////////////////////////////////////////////////////////////////
//строки
tstring Utils::ReplaceStringAll(tstring SrcStr, tstring const& FromStr, tstring const& ToStr) //!TODO :убрать
{	
	size_t off = 0;
	size_t pos = 0;
	while ((pos = SrcStr.find(FromStr, off)) != tstring::npos)
	{
		SrcStr.replace(pos, FromStr.size(), ToStr);
		off = pos + ToStr.size();
	}
	//	boost::replace_all(SrcStr, FromStr, ToStr);	//5 kb
	return SrcStr;
}

void Utils::RemoveLeadingChars(tstring &str, TCHAR ch)
{
	boost::trim_left_if(str, boost::bind(std::equal_to<TCHAR>(), _1, ch));
}
void Utils::RemoveTrailingChars(tstring &str, TCHAR ch) 
{
	boost::trim_right_if(str, boost::bind(std::equal_to<TCHAR>(), _1, ch));
}

void Utils::add_trailing_char_if_not_exists(tstring &Str, TCHAR const* ch)
{
	if (! boost::ends_with(Str, ch)) Str += ch;
}
void Utils::add_leading_char_if_not_exists(tstring &Str, TCHAR const* ch)
{
	if (! boost::starts_with(Str, ch)) Str.insert(0, ch);
}

bool Utils::iFindFirst(tstring const& SrcStr, tstring const& SubStr)
{
	return StrStrI(SrcStr.c_str(), SubStr.c_str()) != NULL;
	//return  boost::ifind_first(SrcStr, SubStr); //стоит почти 20 кб
}

tstring Utils::CombinePath(tstring const& Path1, tstring const& Path2, TCHAR const* Delimiter)
{
	if ( (! boost::starts_with(Path2, Delimiter)) && (! boost::ends_with(Path1, Delimiter))) return Path1 + Delimiter + Path2;	

	return Path1 + Path2;	//разделитель уже есть
}