/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

/************************************************************************/
// ����� ���������� �� �������� ���������
// �������� ����� ��������� ����������� ����������� Far
// *, ? � [a-z]
/************************************************************************/
#include "strings_utils.h"

namespace nf
{
	namespace SearchPathsPolices
	{

		struct CSearchPolice
		{
			CSearchPolice(twhat_to_search_t WhatToSearch) :  m_WhatToSearch(WhatToSearch) {}
		//��������� ����� ������������� �� ��������� ������� ����� ����������
			virtual void  SearchSubdir(tstring const& RootDir
				, tstring const& Name
				, std::list<tstring> &list) const = 0;
			twhat_to_search_t GetWhatToSearch() const {return m_WhatToSearch;}
		private:
			twhat_to_search_t m_WhatToSearch;
		};

		class CSearchSystemPolice : public CSearchPolice
		{	//����� ���������� ����������; ����������� [] �� ��������������
			//��� ��������� ����� ������������� ��������� ���������� � ��������
		public:
			CSearchSystemPolice(twhat_to_search_t WhatToSearch) : CSearchPolice(WhatToSearch) {}
			virtual void SearchSubdir(tstring const& RootDir
				, tstring const& Name
				, std::list<tstring> &list) const;
		};

		class CSearchFarPolice : public CSearchPolice
		{	//������� ��� ����������, � ����� ��������� ���������� ���� � ��������
			//���������� Far
		public:
			CSearchFarPolice(twhat_to_search_t WhatToSearch) 
				: CSearchPolice(WhatToSearch) {}
			virtual void SearchSubdir(tstring const& RootDir
				, tstring const& Name
				, std::list<tstring> &list) const;
		};
	};

	//����� ���������� ������� ���������� ��������
	//��������� ������� ��������� ������ ����� ������ � �������������� ������������ * � ?
	//���� ��������� ��������� ��������������� ����������� [], ������������ � FAR
	//���������� ������� ����� ��� ����������, ����� ��������� �� ���������� � �������� ���������� FAR
	//� ����� ��� �������� ������ ��� ������ CSearcherPaths �������� � SearchPathsPolices

	class CSearcherPaths
	{	
	public:
		typedef std::list<tstring> tlist;

		CSearcherPaths(tlist& Results
			, SearchPathsPolices::CSearchPolice &sp
			, twhat_to_search_t WhatToSearch = WTS_DIRECTORIES) 
			: m_Results(Results)
			, m_Sp(sp)
			, m_WhatToSearch(WhatToSearch)
		{}
		// ����� ����������, ��������� � �������, �� ������� {[\dir]|[\\dir]|[\\\dir]|..}+
		// ������� ������ ����� � ������� ���������� ������������ ���-��� ������.
		// "\dir"  - ����� dir ������ � ������� ����������
		// "\\dir" - ����� dir �� ���� ��������� ����������� � � ����������� ��������������� � ��� ��������� 
		// "\\\dir" � �.�.
		//? "\*\dir" - ����� ����� dir �� ���� ��������� �����������
		//? ".dir" - ������� dir ������� � ���������� �� ���� ������� ����, "..dir" �� ��� ������ ���� � �.�.
		inline bool SearchByPattern(TCHAR const* PatternOEM, tstring const &RootDirOEM)
		{
				tstring RootDirANSI = Utils::GetInternalToOem(RootDirOEM);
				tstring PatternANSI = Utils::GetInternalToOem(PatternOEM);
				return SearchByPatternOEM(PatternANSI.c_str(), RootDirANSI);
		}

		//����� ��� ����������, ��������������� ��������
		//PathPattern ������ ��������� ������ ��������� ���� ����
		//C:\path1\path2\...\pathN
		//������ C - ����� ���� ����� ��� ���� �� ������������ ?, * ��� [a-z]
		//� ������� pathXXX ����� ����������� ����������� ?, * ��� [a,b-z]
		inline bool SearchMatched(tstring PathPatternOEM)
		{
			tstring PatternANSI = Utils::GetInternalToOem(PathPatternOEM);
			return SearchMatchedOEM(PatternANSI);
		}

	private: //operations
		inline void insert_result(tstring const& dir_oem)
		{
			m_Results.push_back(dir_oem);
		}
		bool search_multisubdir(TCHAR const* RootDir, TCHAR const* Name, int level, tlist &list);
		TCHAR const* extract_name(TCHAR const* Pattern, tstring &Name, int &Level);

		bool SearchByPatternOEM(TCHAR const* Pattern, tstring const &RootDir);
		bool SearchMatchedOEM(tstring PathPattern);

	private: //members
		tlist& m_Results;
		SearchPathsPolices::CSearchPolice const &m_Sp;
		twhat_to_search_t m_WhatToSearch;
	}; //CSearcherPaths

};//nf