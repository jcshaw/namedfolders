/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

#include "Kernel.h"
#include "codec_for_values.h"
#include "PanelInfoWrap.h"


namespace nf {

	//������� ������������ ���� �� ��������� ���������
	//�������� ���� ���������, ���� ������ 
	//���� ������������� ����������� ���������� (LocalPath) 
	//���� Value �������� �����������
	namespace Selectors 
	{

		//����� ��� ����, ���������� ��� Value � LocalPath
		//� ������� �� ������ ������ � DestListPaths
		void GetPath(HANDLE hPlugin
			, tstring const &Value
			, tstring const &LocalPath
			, nf::twhat_to_search_t WhatToSearch
			, std::list<tstring> &DestListPaths);

		//����� ��� ����, ���������� ��� Value � LocalPath
		//���� ����������� ������������ ������� ��������� ���� 
		//� ������� ��� � ResultPath
		bool GetPath(HANDLE hPlugin
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
			, tstring &dir);
	} //Selectors
} //nf