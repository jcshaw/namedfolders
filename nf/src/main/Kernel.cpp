/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "kernel.h"
#include "Parser.h"
#include <cassert>
#include <memory>
#include "stlsoft_def.h"
#include "stlcatalogs.h"
#include "registry_remover.h"
#include "strings_utils.h"
#include "CommandPatterns.h"
#include "select_variants.h"

using namespace nf;

size_t nf::Shell::SelectShortcuts(tstring shortcut_pattern
								, tstring catalog
								, tshortcuts_list& list)
{	//����� ��� ������ ��������������� ����������� �������
	if (!shortcut_pattern.size()) shortcut_pattern = L"*";
	Utils::RemoveTrailingChars(catalog, SLASH_CATS_CHAR);
	Shell::SelectShortcuts(catalog, shortcut_pattern, list, true);
	if (list.size()) return Shell::SortByRelevance(list, tstring(catalog), tstring(shortcut_pattern));
	return 0;
}

bool nf::Shell::InsertShortcut(nf::tshortcut_info const&sh
							   , tstring value
							   , bool bOverride)
{	//�������� ����� ��������� � �������
	tstring catalog_name = sh.catalog;
	Utils::RemoveLeadingChars(catalog_name, SLASH_CATS_CHAR);
	Utils::RemoveTrailingChars(catalog_name, SLASH_CATS_CHAR);

	sc::CCatalog c(catalog_name);
	if (! bOverride) {
		tstring buf;
		if (c.GetShortcutInfo(sh.shortcut, sh.bIsTemporary, buf)) return false; //����� ��������� ��� ����...
	}
	tstring oem_value = value;
	c.SetShortcut(sh.shortcut, oem_value, sh.bIsTemporary);
	return true;
}

bool nf::Shell::DeleteShortcut(tshortcut_info const&sh)
{	//������� ���������
	sc::CCatalog c(sh.catalog);
	c.DeleteShortcut(sh.shortcut, sh.bIsTemporary);

	return true;
}

bool nf::Shell::ModifyShortcut(tshortcut_info const& from, tshortcut_info const& to, tstring* pnew_value)
{	//�������� �������� ����������
	tstring value;
	if (! pnew_value) 	{
		Shell::GetShortcutValue(from, value);
	} else {
		value = *pnew_value;
	}

	Shell::DeleteShortcut(from);
	Shell::InsertShortcut(to, value, true);

	return true;
}


bool nf::Shell::InsertCatalog(tstring catalog, wchar_t const* Parent)
{
	Utils::RemoveTrailingChars(catalog, SLASH_CATS_CHAR);

	sc::CCatalog c(Parent);
	return c.InsertSubcatalog(catalog);
}

bool nf::Shell::Private::remove_catalog(tstring const& SourceCatalog, tstring const* pTargetCatalog, bool bDeleteSource)
{
	//����������� ������� � ��� ��������� � ���� �������� � ����������
	//� ������ �������; ���� ��������� �� ������ 
	//������� ������� � ��� ��������� � ���� �������� � ����������
	nf::registry_remover rr;

	tstring src_key = sc::CCatalog(SourceCatalog).GetCatalogRegkey();
	if (! pTargetCatalog) return rr.Erase(src_key);

	tstring target_catalog;
	if (! Utils::ExpandCatalogPath(SourceCatalog, *pTargetCatalog, target_catalog)) return false;

	tstring target_key = sc::CCatalog(target_catalog).GetCatalogRegkey();

	if (bDeleteSource) {
		return rr.Move(src_key, target_key);
	} else {
		return rr.Copy(src_key, target_key);
	}
}

bool nf::Shell::GetShortcutValue(tshortcut_info const& sh, tstring& value)
{
	sc::CCatalog c(sh.catalog);
	return c.GetShortcutInfo(sh.shortcut, sh.bIsTemporary, value);
}

bool nf::Shell::MoveShortcut(tshortcut_info const& sh, tstring const& new_shortcut_path, tshortcut_info &sh2)
{	//����������� ��������� �� ���������� ����.
	//���� ���� ������������ �� / �� ���� ���� - ��� �������
	//���� ��� - �� ��������� ��� - ����� ��� ����������
	//������� ����� ��������� ..
	
	sh2 = sh;
	if (*new_shortcut_path.rend() != SLASH_CATS_CHAR) {	
		std::pair<tstring, tstring> catalog_name = Utils::DivideString(new_shortcut_path, SLASH_CATS_CHAR);
		sh2.catalog.swap(catalog_name.first);
		Utils::RemoveLeadingChars(catalog_name.second, SLASH_CATS_CHAR);
		if (! catalog_name.second.empty()) sh2.shortcut.swap(catalog_name.second);
	}

	if (! sh2.catalog.empty() && ! Utils::ExpandCatalogPath(sh.catalog, sh2.catalog, sh2.catalog)) return false;
	return Shell::ModifyShortcut(sh, sh2, 0);
}

bool nf::Shell::CopyShortcut(tshortcut_info const& sh
							 , tstring const& new_shortcut_path
							 , tshortcut_info &sh2)
{	//����������� ��������� �� ���������� ����.
	//���� ���� ������������ �� / �� ���� ���� - ��� �������
	//���� ��� - �� ��������� ��� - ����� ��� ����������
	//������� ����� ��������� ..

	sh2 = sh;
	if (*new_shortcut_path.rend() != SLASH_CATS_CHAR)
	{	
		tstring name;
		Utils::DividePathFilename(new_shortcut_path, sh2.catalog, name, SLASH_CATS_CHAR, false);
		Utils::RemoveLeadingChars(name, SLASH_CATS_CHAR);
		if (! name.empty()) sh2.shortcut.swap(name);
	}

	tstring value;
	if (! Shell::GetShortcutValue(sh, value)) return false;

	if (! sh2.catalog.empty() && ! Utils::ExpandCatalogPath(sh.catalog, sh2.catalog, sh2.catalog)) return false;
	return Shell::InsertShortcut(sh2, value, false);
}




