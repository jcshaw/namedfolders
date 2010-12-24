/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

#include "Kernel.h"
#include "shortcut_utils.h"
#include "PanelInfoWrap.h"


namespace nf {
//������� ������������ ���� �� ��������� ���������
//�������� ���� ���������, ���� ������ 
//���� ������������� ����������� ���������� (LocalPath) 
//���� Value �������� �����������
namespace Selectors {
	//����� ��� ����, ���������� ��� Value � LocalPath
	//� ������� �� ������ ������ � DestListPaths
	void GetPath(HANDLE hPlugin
		, tstring const &Value
		, tstring const &LocalPath
		, nf::twhat_to_search_t WhatToSearch
		, nf::tlist_strings &DestListPaths);

	//����� ��� ����, ���������� ��� Value � LocalPath
	//���� ����������� ������������ ������� ��������� ���� 
	//� ������� ��� � ResultPath
	tpath_selection_result GetPath(HANDLE hPlugin
		, tstring const &SrcPath
		, tstring const &LocalPath
		, tstring &ResultPath
		, nf::twhat_to_search_t WhatToSearch);

	//������� �������� ���������� ��������� �� ������
	bool GetShortcut(HANDLE hPlugin
		, nf::tparsed_command const &cmd
		, nf::tshortcut_info& sh);
	//������� ��� ���������� ���������� �� ������
	bool GetAllShortcuts(HANDLE hPlugin
		, nf::tparsed_command const &cmd
		, nf::tshortcuts_list& DestList);

	//������� �������� ���������� ������� �� ������
	bool GetCatalog(HANDLE hPlugin
		, nf::tparsed_command const &cmd
		, nf::tcatalog_info& c);

	//����� ��������� ���������� 
	//���� ��������� ���������� ��� - ����� ���������
	bool FindBestDirectory(HANDLE hPlugin
		, nf::tshortcut_value_parsed const &p
		, tstring const& localPath
		, tstring &dir);
} //Selectors
} //nf