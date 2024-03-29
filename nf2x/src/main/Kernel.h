/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#pragma once
#include <list>
#include <vector>
#include "header.h"

extern const TCHAR CHAR_LEADING_VALUE_ENVVAR;
extern const TCHAR CHAR_LEADING_VALUE_REGKEY;


namespace nf
{

namespace Shell 
{
//������� ��� ������ ���� ��������� ���������/�������/����������
//�� ������ ������ ��������� � ������� ������������
//������� ������ ���� ����� ��������� ��������
//������� ���������� ����� ��������� ���������
	size_t SelectShortcuts(tstring shortcut_pattern, tstring catalog, tshortcuts_list& list);	
//	size_t SelectCatalogsAndShortcuts(TCHAR const *shortcut_pattern, TCHAR const* catalog_pattern, tshortcuts_list& list);

//�������� � ������������
	bool InsertShortcut(tshortcut_info const&shortcut, tstring value, bool bOverride);		
	bool ModifyShortcut(tshortcut_info const& from
		, tshortcut_info const&to, tstring * pnew_value = 0);		//�������� �������� ����������
	bool DeleteShortcut(tshortcut_info const&shortcut);		//������� ���������
	bool MoveShortcut(tshortcut_info const&shortcut
		, tstring const& new_shortcut_path
		, tshortcut_info &new_shortcut
	);
	bool CopyShortcut(tshortcut_info const&shortcut
		, tstring const& new_shortcut_path
		, tshortcut_info &new_shortcut
	);

//////////////////////////////////////////////////////////////////////////
//�������� � ����������	
	namespace Private
	{
		bool remove_catalog(tstring const& SourceCatalog, tstring const* pTargetCatalog, bool bDeleteSource);
	}

	bool InsertCatalog(tstring catalog, TCHAR const* Parent = _T(""));		
	inline bool DeleteCatalog(tstring const& SourceCatalog)
	{
		return Private::remove_catalog(SourceCatalog, 0, true);
	}

	inline bool MoveCatalog(tstring const& SourceCatalog, tstring const& TargetCatalog)
	{
		return Private::remove_catalog(SourceCatalog, &TargetCatalog, true);
	}

	inline bool CopyCatalog(tstring const& SourceCatalog, tstring const& TargetCatalog)
	{
		return Private::remove_catalog(SourceCatalog, &TargetCatalog, false);
	}

//��������� ���� ��� ��������� ����������
	bool GetShortcutValue(tshortcut_info const& sh, tstring& DestValue);

} //Shell


}