/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

/************************************************************************/
// ����� ���������� �� �������� ���������
// �������� ����� ��������� ����������� ����������� Far
// *, ? � [a-z]
/************************************************************************/
#include "strings_utils.h"

namespace nf {
//����� ���������� ������� ���������� ��������
//��������� ������� ��������� ������ ����� ������ � �������������� ������������ * � ?
//���� ��������� ��������� ��������������� ����������� [], ������������ � FAR
//���������� ������� ����� ��� ����������, ����� ��������� �� ���������� � �������� ���������� FAR
//� ����� ��� �������� ������ ��� ������ CSearcherPaths �������� � Search
namespace Search {
	struct CSearchPolice {
		explicit CSearchPolice(twhat_to_search_t WhatToSearch) :  m_WhatToSearch(WhatToSearch) {}
	//��������� ����� ������������� �� ��������� ������� ����� ����������
		virtual void  SearchSubdir(tstring const& RootDir, tstring const& Name, nf::tlist_strings &list) const = 0;
		twhat_to_search_t GetWhatToSearch() const {
			return m_WhatToSearch; }
	private:
		twhat_to_search_t m_WhatToSearch;
	};

	class CSearchSystemPolice : public CSearchPolice {
		//����� ���������� ����������; ����������� [] �� ��������������
		//��� ��������� ����� ������������� ��������� ���������� � ��������
	public:
		explicit CSearchSystemPolice(twhat_to_search_t WhatToSearch) : CSearchPolice(WhatToSearch) {}
		virtual void SearchSubdir(tstring const& RootDir, tstring const& Name, nf::tlist_strings &list) const;
	};

	class CSearchFarPolice : public CSearchPolice
	{	//������� ��� ����������, � ����� ��������� ���������� ���� � ��������
		//���������� Far
	public:
		explicit CSearchFarPolice(twhat_to_search_t WhatToSearch) : CSearchPolice(WhatToSearch) {}
		virtual void SearchSubdir(tstring const& RootDir, tstring const& Name, nf::tlist_strings &list) const;
	};

	// ����� ����������, ��������� � �������, �� ������� {[\dir]|[\\dir]|[\\\dir]|..}+
	// ������� ������ ����� � ������� ���������� ������������ ���-��� ������.
	// "\dir"  - ����� dir ������ � ������� ����������
	// "\\dir" - ����� dir �� ���� ��������� ����������� � � ����������� ��������������� � ��� ��������� 
	// "\\\dir" � �.�.
	//? "\*\dir" - ����� ����� dir �� ���� ��������� �����������
	//? ".dir" - ������� dir ������� � ���������� �� ���� ������� ����, "..dir" �� ��� ������ ���� � �.�.
	bool SearchByPattern(tstring const& srcPattern, tstring const &rootDir, Search::CSearchPolice &searchPolice, nf::tlist_strings& dest);

	//����� ��� ����������, ��������������� ��������
	//PathPattern ������ ��������� ������ ��������� ���� ����
	//C:\path1\path2\...\pathN
	//������ C - ����� ���� ����� ��� ���� �� ������������ ?, * ��� [a-z]
	//� ������� pathXXX ����� ����������� ����������� ?, * ��� [a,b-z]
	bool SearchMatched(tstring const& PathPattern, Search::CSearchPolice &searchPolice, nf::tlist_strings& dest);

	namespace Private {
		wchar_t const* extract_name(wchar_t const* srcPattern, tstring &destName, int &destLevel);
		bool search_multisubdir(tstring const& RootDir, tstring const& Name, int level
			, Search::CSearchPolice &searchPolice
			, nf::tlist_strings &dest);
	}
}
}