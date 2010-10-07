#include "stdafx.h"

#include <boost/regex.hpp>
#include <cassert>
#include <shlwapi.h>


int FarCmpName(const char *Pattern, const char *String, int SkipPath)
{
	assert(Pattern);

//определяем строку, по которой будем строить регулярное выражение
	const char* p = Pattern;

//используем regex_search если не используется метасимвол ?
	bool bUseSearch = true;

//если нужно пропустить директорию - ищем только имя файла
	if (SkipPath)
	{	
		p = ::PathFindFileName(Pattern);
	}

//строим регулярное выражение на основе исходной строки
//экранируем символы, подменяем метасимволы
	std::string r;
	r.reserve(lstrlen(Pattern)*2);
	while (*p != 0)
	{
		switch (*p)
		{
		case '[':
			while (*p != 0 )
			{
				r += *p;
				if (*p == ']') break;
				++p;
			}
			if (*p == 0) break;
			break;
		case '?':
			bUseSearch = false;
			r += ".";
			break;
		case '*':
			r += ".*";
			break;
		case '.':
		case '|':
		case '+':
		case '(':
		case ')':
		case '{':
		case '}':
		case '^':
		case '$':
		case '\\':	
			r += "\\";
			r += *p;
			break;
		default:
			r += *p;
		}
		++p;
	}
//провереяем искомую строку построенным регулярным выражением
	if (bUseSearch)
	{
		r += ".*";
		boost::regex re(r, boost::regex_constants::icase);
		if (boost::regex_match(String, re)) return 1;
	} else {
		boost::regex re(r, boost::regex_constants::icase);
		if (boost::regex_match(String, re)) return 1;
	}
	return 0;
}
