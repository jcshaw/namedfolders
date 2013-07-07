/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
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

/// find all shortcuts that match to specified pattern
size_t nf::Shell::SelectShortcuts(tstring shPattern, tstring srcCatalog, tshortcuts_list& destList) {
	if (!shPattern.size()) shPattern = L"*";
	Utils::RemoveTrailingCharsOnPlace(srcCatalog, SLASH_CATS_CHAR);
	Shell::SelectShortcuts(srcCatalog, shPattern, destList, true);
	if (destList.size()) return Shell::SortByRelevance(destList, tstring(srcCatalog), tstring(shPattern));
	return 0;
}

bool nf::Shell::InsertShortcut(nf::tshortcut_info const& sh, tstring const& srcValue, bool bOverride) {
	tstring catalog_name = sh.catalog;
	Utils::RemoveLeadingCharsOnPlace(catalog_name, SLASH_CATS_CHAR);
	Utils::RemoveTrailingCharsOnPlace(catalog_name, SLASH_CATS_CHAR);

	sc::CCatalog c(catalog_name);
	if (! bOverride) {
		tstring buf;
		if (c.GetShortcutInfo(sh.shortcut, sh.bIsTemporary, buf)) {
			return false; //the shortcut already exists
		}
	}

	c.SetShortcut(sh.shortcut, srcValue, sh.bIsTemporary);
	return true;
}

bool nf::Shell::DeleteShortcut(tshortcut_info const&sh) {	
	sc::CCatalog c(sh.catalog);
	c.DeleteShortcut(sh.shortcut, sh.bIsTemporary);

	return true;
}

bool nf::Shell::ModifyShortcut(tshortcut_info const& from, tshortcut_info const& to, tstring* pNewValue) {	
	tstring value;
	if (! pNewValue) 	{
		Shell::GetShortcutValue(from, value);
	} else {
		value = *pNewValue;
	}

	Shell::DeleteShortcut(from);
	Shell::InsertShortcut(to, value, true);

	return true;
}

bool nf::Shell::InsertCatalog(tstring const& srcCatalog, tstring const& subCatalogName) {
	sc::CCatalog c(srcCatalog); //CCatalog is able to restore catalog hierarchy 
	if (! subCatalogName.empty()) {
		return c.InsertSubcatalog(Utils::RemoveTrailingChars(subCatalogName, SLASH_CATS_CHAR));
	} else {
		return true; 
	}
}

/// переместить каталог и все вложенные в него каталоги и псевдонимы
/// в другой каталог; если последний не указан 
/// удалить каталог и все вложенные в него каталоги и псевдонимы
bool nf::Shell::Private::remove_catalog(tstring const& srcCatalog, tstring const* pTargetCatalog, bool bDeleteSource) {
	nf::registry_remover rr;
	assert(false); //!TODO: перейти от реестра к настройкам
	return false;

// 	tstring src_key = sc::CCatalog(srcCatalog).GetCatalogRegkey();
// 	if (! pTargetCatalog) {
// 		return rr.Erase(src_key);
// 	}
// 
// 	tstring target_catalog;
// 	if (! Utils::ExpandCatalogPath(srcCatalog, *pTargetCatalog, target_catalog, false)) {
// 		return false;
// 	}
// 
// 	tstring target_key = sc::CCatalog(target_catalog).GetCatalogRegkey();
// 	if (target_key == srcCatalog) {
// 		return true; //nothing to do
// 	}
// 
// 	return bDeleteSource
// 		? rr.Move(src_key, target_key)
// 		: rr.Copy(src_key, target_key);
}

bool nf::Shell::GetShortcutValue(tshortcut_info const& sh, tstring& value) {
	return sc::CCatalog(sh.catalog).GetShortcutInfo(sh.shortcut, sh.bIsTemporary, value);
}

bool nf::Shell::MoveShortcut(tshortcut_info const& sh, tstring const& new_shortcut_path, tshortcut_info &sh2) {	
	if (Utils::PrepareMovingShortcut(sh, new_shortcut_path, sh2)) {
		return Shell::ModifyShortcut(sh, sh2, 0);
	} else {
		return false;
	}
}

bool nf::Shell::CopyShortcut(tshortcut_info const& srcSh, tstring const& targetPath, tshortcut_info &destSh) {	
	if (Utils::PrepareMovingShortcut(srcSh, targetPath, destSh)) {
		tstring value;
		if (! Shell::GetShortcutValue(srcSh, value)) return false;
		return Shell::InsertShortcut(destSh, value, false);
	} else {
		return false;
	}
}



