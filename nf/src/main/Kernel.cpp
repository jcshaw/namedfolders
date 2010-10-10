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
#include "catalog_names.h"

using namespace nf;

size_t nf::Shell::SelectShortcuts(tstring shortcut_pattern
								, tstring catalog
								, tshortcuts_list& list)
{	//найти все ярлыки удовлетворяющие переданному шаблону
	if (!shortcut_pattern.size()) shortcut_pattern = L"*";
	Utils::RemoveTrailingCharsOnPlace(catalog, SLASH_CATS_CHAR);
	Shell::SelectShortcuts(catalog, shortcut_pattern, list, true);
	if (list.size()) return Shell::SortByRelevance(list, tstring(catalog), tstring(shortcut_pattern));
	return 0;
}

bool nf::Shell::InsertShortcut(nf::tshortcut_info const&sh
							   , tstring value
							   , bool bOverride)
{	//добавить новый псевдоним в каталог
	tstring catalog_name = sh.catalog;
	Utils::RemoveLeadingCharsOnPlace(catalog_name, SLASH_CATS_CHAR);
	Utils::RemoveTrailingCharsOnPlace(catalog_name, SLASH_CATS_CHAR);

	sc::CCatalog c(catalog_name);
	if (! bOverride) {
		tstring buf;
		if (c.GetShortcutInfo(sh.shortcut, sh.bIsTemporary, buf)) return false; //такой псевдоним уже есть...
	}
	tstring oem_value = value;
	c.SetShortcut(sh.shortcut, oem_value, sh.bIsTemporary);
	return true;
}

bool nf::Shell::DeleteShortcut(tshortcut_info const&sh)
{	//удалить псевдоним
	sc::CCatalog c(sh.catalog);
	c.DeleteShortcut(sh.shortcut, sh.bIsTemporary);

	return true;
}

bool nf::Shell::ModifyShortcut(tshortcut_info const& from, tshortcut_info const& to, tstring* pnew_value)
{	//обновить свойства псевдонима
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
	Utils::RemoveTrailingCharsOnPlace(catalog, SLASH_CATS_CHAR);

	sc::CCatalog c(Parent);
	return c.InsertSubcatalog(catalog);
}

bool nf::Shell::Private::remove_catalog(tstring const& SourceCatalog, tstring const* pTargetCatalog, bool bDeleteSource)
{
	//переместить каталог и все вложенные в него каталоги и псевдонимы
	//в другой каталог; если последний не указан 
	//удалить каталог и все вложенные в него каталоги и псевдонимы
	nf::registry_remover rr;

	tstring src_key = sc::CCatalog(SourceCatalog).GetCatalogRegkey();
	if (! pTargetCatalog) return rr.Erase(src_key);

	tstring target_catalog;
	if (! Utils::ExpandCatalogPath(SourceCatalog, *pTargetCatalog, target_catalog, false)) return false;

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
{	
	if (Utils::PrepareMovingShortcut(sh, new_shortcut_path, sh2)) {
		return Shell::ModifyShortcut(sh, sh2, 0);
	} else return false;
}

bool nf::Shell::CopyShortcut(tshortcut_info const& sh 
							 , tstring const& new_shortcut_path
							 , tshortcut_info &sh2)
{	
	if (Utils::PrepareMovingShortcut(sh, new_shortcut_path, sh2)) {
		tstring value;
		if (! Shell::GetShortcutValue(sh, value)) return false;
		return Shell::InsertShortcut(sh2, value, false);
	} else return false;
}



