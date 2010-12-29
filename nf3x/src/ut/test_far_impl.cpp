#include "stdafx.h"
#include "test_far_impl.h"

#include <boost/regex.hpp>
#include <cassert>
#include <shlwapi.h>

int nf::FarCmpName(const wchar_t *Pattern, const wchar_t *String, int SkipPath)
{
	assert(Pattern);

//���������� ������, �� ������� ����� ������� ���������� ���������
	const wchar_t* p = Pattern;

//���������� regex_search ���� �� ������������ ���������� ?
	bool bUseSearch = true;

//���� ����� ���������� ���������� - ���� ������ ��� �����
	if (SkipPath)
	{	
		p = ::PathFindFileName(Pattern);
	}

//������ ���������� ��������� �� ������ �������� ������
//���������� �������, ��������� �����������
	tstring r;
	r.reserve(lstrlen(Pattern)*2);
	while (*p != 0)
	{
		switch (*p) {
		case L'[':
			while (*p != 0 ) {
				r += *p;
				if (*p == L']') break;
				++p;
			}
			if (*p == 0) break;
			break;
		case L'?':
			bUseSearch = false;
			r += L".";
			break;
		case L'*':
			r += L".*";
			break;
		case L'.':
		case L'|':
		case L'+':
		case L'(':
		case L')':
		case L'{':
		case L'}':
		case L'^':
		case L'$':
		case L'\\':	
			r += L"\\";
			r += *p;
			break;
		default:
			r += *p;
		}
		++p;
	}
//���������� ������� ������ ����������� ���������� ����������
	if (bUseSearch)
	{
		r += L".*";
		boost::wregex re(r, boost::regex_constants::icase);
		if (boost::regex_match(String, re)) return 1;
	} else {
		boost::wregex re(r, boost::regex_constants::icase);
		if (boost::regex_match(String, re)) return 1;
	}
	return 0;
}
