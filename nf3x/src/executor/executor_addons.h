/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include "Kernel.h"
#include "shortcut_utils.h"
#include <list>
#include "PanelInfoWrap.h"

namespace nf {
namespace Selectors {
//�������� ������ ������ ���������� ��� �������� ���������� �����/����� �������
	void GetAllPathForRegKey(HANDLE hPlugins
		, tstring const &RegKeyName //���� �������, ���������� ����������, ���������� ��������� ����
		, tstring const &VarName //�������� ���������� � ����� �������
		, nf::tlist_strings &DestListPaths);
	void GetAllPathForEnvvar(HANDLE hPlugins
		, tstring const &VarName //�������� ����������: ����� ���� ������� ��������� %temp% ��� �������� %t
		, nf::tlist_strings &DestListPaths);

//cd:%
	bool OpenEnvVar(HANDLE hPlugin, tstring const &VarName, tstring const &LocalPath);
	bool GetPathByEnvvarPattern(HANDLE hPlugins
		, tstring const &VarName //�������� ����������: ����� ���� ������� ��������� %temp% ��� �������� %t
		, tstring const &LocalPath
		, tstring &path	//��������� ������������� ���������� �� ���� ��������� ����������
	);
//value = $reg_key
	bool GetPathByRegKey(HANDLE hPlugins
		, tstring const &regkey_name //���� �������, ���������� ����������, ���������� ��������� ����
		, tstring LocalPath
		, tstring &path	//��������� ������������� ���������� �� ���� ��������� ����������
	);

//cd:~
	bool GetShortcutByPathPattern(HANDLE hPlugin
		, tstring const &current_path
		, nf::tparsed_command const &cmd
		, nf::tshortcut_info& sh
	);
}

} // nf