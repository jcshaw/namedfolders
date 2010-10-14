/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "searcherpaths.h"

#include <list>
#include <functional>
#include <boost/bind.hpp>
#include <algorithm>
#include <cmath>
#include "stlsoft_def.h"

#include "strings_utils.h"

#include "Parser.h"

using namespace nf;

namespace
{
	//� header ��������� ��������������� wchar_t* 
	//������ wchar_t �����, ����� ������� case
	wchar_t const DEEP_DIRECT_SEARCH_CH = L'\n';
	wchar_t const DEEP_REVERSE_SEARCH_CH = L'\t';
	wchar_t const DEEP_UP_DIRECTORY_CH = L'\b'; //������, ������������ ������� �� ������� ���� = '..'

	inline bool is_slash(wchar_t ch) 
	{ 
		return SLASH_DIRS_CHAR == ch 
			|| DEEP_UP_DIRECTORY_CH == ch 
			|| DEEP_DIRECT_SEARCH_CH == ch 
			|| DEEP_REVERSE_SEARCH_CH == ch;
	}

	const int UNLIMITED_SEARCH_LEVEL = 9999;
}

bool CheckForEsc(void)
{
	bool EC=false;
	INPUT_RECORD rec;
	static HANDLE hConInp=GetStdHandle(STD_INPUT_HANDLE);
	DWORD ReadCount;
	while (1)
	{
		PeekConsoleInput(hConInp,&rec,1,&ReadCount);
		if (ReadCount==0) break;
		ReadConsoleInput(hConInp,&rec,1,&ReadCount);
		if (rec.EventType==KEY_EVENT)
			if (rec.Event.KeyEvent.wVirtualKeyCode==VK_ESCAPE &&
				rec.Event.KeyEvent.bKeyDown) EC=true;
	}
	return(EC);
}

//////////////////////////////////////////////////////////////////////////
// private
bool CSearcherPaths::SearchByPatternOEM(wchar_t const* Pattern
										, tstring const &RootDir)
{	// ����� ����������, ��������� � �������, �� ������� {[\dir]|[\\dir]|[\\\dir]|..}+
	// ������� ������ ����� � ������� ���������� ������������ ���-��� ������.
	// "\dir"  - ����� ����� dir ������ � ������� ����������
	// "\\dir" - ����� ����� dir �� ���� ��������� ����������� � � ����������� ��������� � ���
	// "\\\dir" � �.�.
	// �������������� ������� ����� \t � \n

	//��� �������� (������������ ���������) �� ������ ���� �������� .*. � slesh*slesh �� �����������

	tstring name; 
	int level = 0;	//�� �������� ������� ����������� ������

	wchar_t const* next_pattern = extract_name(Pattern, name, level);
	if (name.empty())  return false; //����� ��������, ���������� �� ��������...

	name = Parser::ConvertToMask(name);

	tlist variants;
	search_multisubdir(RootDir.c_str(), name.c_str(), level, variants);

	if (! *next_pattern)
	{	// ����� ��������, variants �������� ������� ����������
		// �������� �� � ������ �����������
		std::copy(variants.begin(), variants.end(), 
			std::insert_iterator<tlist>(m_Results, m_Results.begin()));
	} else {
		//� ������ �� ��������� ���������� ���� ��������� ����� 
		std::for_each(variants.begin(), variants.end()
			, boost::bind(&CSearcherPaths::SearchByPatternOEM, this, next_pattern, _1));
	}

	return true;
}

bool CSearcherPaths::SearchMatchedOEM(tstring PathPattern)
{	//����� ��� ����������, ��������������� ��������
	//PathPattern ������ ��������� ������ ��������� ���� ����
	//C:\path1\path2\...\pathN
	//������ C - ����� ���� ����� ��� ���� �� ������������ ?, * ��� [a-z]
	//� ������� pathXXX ����� ����������� ����������� ?, * ��� [a,b-z]

	//���������� ����� �����
	wchar_t const* DISK_PATH_DELIMETER = L":\\";
	if (PathPattern.size() < 2) 
	{
		insert_result(PathPattern);
		return false;	//��� ������� "c:"
	}
	
	size_t column_pos = PathPattern.find(DISK_PATH_DELIMETER);
	if (column_pos == tstring::npos) 
	{
		insert_result(PathPattern);
		return false;
	}

	tstring disk_pattern(PathPattern, 0, column_pos);
	PathPattern.erase(0, disk_pattern.size() + 1); 
		//������� ��� ����� (��� �����) � ":", ��������� ������ ����, ������������ � 1 �����

	if (! disk_pattern.size()) {
		insert_result(PathPattern);
		return false;
	}
	
	if (disk_pattern.size() == 1 &&  disk_pattern[0] != L'*' &&  disk_pattern[0] != L'?') {	//���� ������ ����
		if (! PathPattern.size()) {	//������ ���� ���� "c:"
			insert_result(PathPattern);
			return true;
		}
		tstring root_path = disk_pattern + L":";
		
		return SearchByPatternOEM(PathPattern.c_str(), root_path);
	} else {
		//������ ����� ����� ������ ����������
		//���������� ��� ����� � �������, �������� �����
		//���������� ��� ���, ������� ������������� ��������
		DWORD drives_bitmask = GetLogicalDrives();
		for (int ibit = 2; ibit< 26; ++ibit) {
			if (! (drives_bitmask & (1 << ibit))) continue;

			wchar_t letter[2] = { wchar_t(L'A' + ibit), 0};
			tstring driver_letter = &letter[0];
			tstring drive_name = driver_letter + L":\\";

			if (GetDriveType(drive_name.c_str()) != DRIVE_FIXED)  
				continue;	//�������� ������ � hdd

			tstring drive_name_short = driver_letter + L":";
			if (nf::Parser::IsTokenMatchedToPattern(driver_letter.c_str(), disk_pattern, false)) {
				SearchByPatternOEM(PathPattern.c_str(), drive_name_short);		
			}
		} //for
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
// private

wchar_t const* CSearcherPaths::extract_name(wchar_t const* Pattern, tstring &Name, int &Level)
{	//������� �� Pattern ������ ���, ���������� �� ����� ���-�� ������� �����������
	//������, ������� �������
	//����� ����� ���� ��������� ������� � ������� (���� - ����� ������, ����� - �����)
	//����������� ��������� ��� ������ ��� ����������� ������ �����������
	//.*. � slesh*slesh
	wchar_t const* ps = Pattern;
	int pattern_length = lstrlen(Pattern);
	assert(pattern_length > 0);
	
	bool bSearchDirectionToDeep = true;
	Level = 0;
	while (is_slash(*ps)) 
	{
		switch (*ps)
		{
		case L'\\': ++Level; break;
		case DEEP_UP_DIRECTORY_CH: --Level; break;
		case DEEP_DIRECT_SEARCH_CH: Level = UNLIMITED_SEARCH_LEVEL; break;
		case DEEP_REVERSE_SEARCH_CH: Level = -UNLIMITED_SEARCH_LEVEL; break;
		default: assert(false);
		}
		++ps;
	}

	wchar_t const* psend = ps;
	while ((*psend) && (! is_slash(*psend))) ++psend;

	Name.assign(ps, psend - ps);
	return psend;
}

bool CSearcherPaths::search_multisubdir(wchar_t const* RootDir
										, wchar_t const* Name
										, int level
										, tlist &list)
{	//����� ����� ���������� ������ ������ �����������
	if (level == 0) ++level;	// \ � . �����������, ����� ���� ������ � ������� ����������

	if (CheckForEsc()) return false;	//exit by esc

	if (level >= 0)
	{
		//������� ��� ��������������� ��������� ����������, 
		//��������������� ����� � ������� �� � �������������� ������
		m_Sp.SearchSubdir(RootDir, Name, list);

		level--;

		if (! level) return true;	//������ �� ����, �������� ���������� ������ �����������

		//���������� ���� �� ���� ��������� �����������
		tlist list_subdirs;
		SearchPathsPolices::CSearchSystemPolice sp(m_WhatToSearch); //!tODO
		sp.SearchSubdir(RootDir, L"*", list_subdirs);

		tlist::const_iterator p = list_subdirs.begin();
		while (p != list_subdirs.end())
		{
			if (!search_multisubdir(p->c_str(), Name, level, list)) 
			{
				return false;	//exit by esc
			}
			++p;
		}
	} else 
	{
		//����� � �������� �����������
		//��������� � ���������� ���� � ���������, ���� �� � ��� ����������
		//������� ������������� �����
		
		//� RootDir ������� ��������� ����������
		tstring parent = RootDir;
		while (level)
		{
			parent = Utils::ExtractParentDirectory(parent);
			if (parent.empty()) return true; //���� ������������ ���������� ���, ����� ��������
			Utils::RemoveTrailingCharsOnPlace(parent, SLASH_DIRS_CHAR);	//!TODO: 

			m_Sp.SearchSubdir(parent, Name, list);
		
			level++;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void nf::SearchPathsPolices::
CSearchSystemPolice::SearchSubdir(tstring const& RootDir0
								  , tstring const& Name
								  , std::list<tstring> &list) const
{	//����� ��������������� �������� ������������� ���������� ���������
	tstring smask = Parser::ConvertToMask(Name);

	//���� � �������� RootDir ���������� z: �� ����� �� �����������
	//��������� � ��������� ����
	tstring RootDir = RootDir0;
	if (Utils::IsAliasIsDisk(RootDir)) RootDir += L"\\";

	nf::twhat_to_search_t what_to_search = GetWhatToSearch();
	WinSTL::findfile_sequence_t f(RootDir.c_str()
		, smask.c_str()
		, WTS_DIRECTORIES == what_to_search
			? WinSTL::findfile_sequence_t::directories
			: (WTS_FILES == what_to_search 
				? WinSTL::findfile_sequence_t::files
				: WinSTL::findfile_sequence_t::files | WinSTL::findfile_sequence_t::directories)
		);
	BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
		tstring full_filename = Utils::CombinePath(RootDir0, t.get_filename(), SLASH_DIRS);
		list.push_back(full_filename);
	}
}

void  nf::SearchPathsPolices::
CSearchFarPolice::SearchSubdir(tstring const& RootDir
							   , tstring const& Name
							   , std::list<tstring> &list) const
{	//����� ��������������� �������� ������������� ���������� FAR
	nf::twhat_to_search_t what_to_search = GetWhatToSearch();
	tstring root_dir = RootDir;
	if (root_dir.size() == 2) root_dir += SLASH_DIRS;	
		//!TODO: � ����� ����� ���������� �������� \\
		//����� � ������ findfile_sequence_t ���������� ������������ �������� ����������
		//��� stlsoft?
	WinSTL::findfile_sequence_t f(root_dir.c_str()
		, L"*"
		, WTS_DIRECTORIES == what_to_search
			? WinSTL::findfile_sequence_t::directories
			: (WTS_FILES == what_to_search 
				? WinSTL::findfile_sequence_t::files
				: WinSTL::findfile_sequence_t::files | WinSTL::findfile_sequence_t::directories)
 	);
	BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
		tstring file_name = t.get_filename();
		if (nf::Parser::IsTokenMatchedToPatternOEM(file_name, Name, false)
			|| (file_name == Name)	//������ ��������� �� ������, ���� � ���� ������������ ���������� ������
			)
		{
			tstring full_filename = tstring(RootDir) + tstring(SLASH_DIRS) + file_name;
			list.push_back(full_filename);
		};
	} 

}
