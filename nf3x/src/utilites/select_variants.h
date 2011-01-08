/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

namespace nf {
namespace Shell {
	//����� ��� �������� ��������� ��������������� �������
	int SelectCatalogs(tstring const& SrcPattern		//������
						, nf::tvector_strings& DestList//������ ���� ��������� ����������					
						);	

	//����� � �������� �������� ��� ������, ��������������� �������
	int SelectShortcuts(tstring const &catalog				//� ��������
						, tstring const &shortcut_pattern	//������ 
						, nf::tshortcuts_list& DestList			//������ ���� ��������� ��������
						, bool bSearchInSubcatalogs	//������ �� ��������� ��������� ����
						);

	//����� � �������� �������� ��� ����������
	//��� ������� ����������� ����������, �� ������� ��� ���������
	//������������� ����������� �������
	int SelectShortcutsByPath(tstring catalog_pattern
							  , tstring const &value_pattern
							  , nf::tshortcuts_list& DestList
							  , bool bExactCoincidence
							  , bool bSearchInSubcatalogs = true);


	//����������� ������������������ ��������� ��������� �� �������������
	//������ ���������� ��������� ������� � ������� �� ������ ����������
	int SortByRelevance(nf::tshortcuts_list& SrcDestList, 
						tstring const& catalog, 
						tstring const& shortcut_pattern);


} //Shell
} //nf