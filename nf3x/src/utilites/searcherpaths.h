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
	struct CSearchEngine {
		explicit CSearchEngine(twhat_to_search_t WhatToSearch) :  m_WhatToSearch(WhatToSearch) {}
	//��������� ����� ������������� �� ��������� ������� ����� ����������
		void SearchItems(tstring const& RootDir, tstring const& Name, nf::tlist_strings &list, twhat_to_search_t whatToSearch) const;
		inline void SearchItems(tstring const& rootDir, tstring const& srcName, nf::tlist_strings &destList) const {
			SearchItems(rootDir, srcName, destList, m_WhatToSearch);
		}
		void SearchBySystemFunctions(tstring const& RootDir, tstring const& Name, nf::tlist_strings &list, twhat_to_search_t whatToSearch) const;
	private:
		twhat_to_search_t m_WhatToSearch;
	};

	// ����� ����������, ��������� � �������, �� ������� {[\dir]|[\\dir]|[\\\dir]|..}+
	// "\*", "\**", "\..", "\..*"
	bool SearchByPattern(tstring const& srcPattern, tstring const &rootDir, Search::CSearchEngine &searchPolice, nf::tlist_strings& dest);

	//����� ��� ����������, ��������������� ��������
	//PathPattern ������ ��������� ������ ��������� ���� ����
	//C:\path1\path2\...\pathN
	//������ C - ����� ���� ����� ��� ���� �� ������������ ?, * ��� [a-z]
	//� ������� pathXXX ����� ����������� ����������� ?, * ��� [a,b-z]
	bool SearchMatched(tstring const& PathPattern, Search::CSearchEngine &searchPolice, nf::tlist_strings& dest);

	namespace Private {
		wchar_t const* extract_name(wchar_t const* srcPattern, tstring &destName, tstring &destLevel);
		bool search_multisubdir(tstring const& RootDir, tstring const& Name
			, unsigned levelIndex
			, tstring const& level
			, Search::CSearchEngine &searchPolice
			, nf::tlist_strings &dest);
	}
}
}