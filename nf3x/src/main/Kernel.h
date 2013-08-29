/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once
#include <list>
#include <vector>
#include "header.h"

extern const wchar_t CHAR_LEADING_VALUE_ENVVAR;
extern const wchar_t CHAR_LEADING_VALUE_REGKEY;


namespace nf {
namespace Shell {

	/// search all variants of catalogs/shortcuts/directories
	/// exactly matched results must be returned first
	/// returns count of exact matches
	size_t SelectShortcuts(tstring shortcut_pattern, tstring catalog, tshortcuts_list& list);	

//shortcuts operations
	bool InsertShortcut(tshortcut_info const&shortcut, tstring const& srcValue, bool bOverride);		
	bool ModifyShortcut(tshortcut_info const& from
		, tshortcut_info const &to, tstring * pnew_value = 0);		
	bool DeleteShortcut(tshortcut_info const&shortcut);		
	bool MoveShortcut(tshortcut_info const&shortcut
		, tstring const& new_shortcut_path
		, tshortcut_info &new_shortcut
	);

	bool PrepareMovingShortcut(tshortcut_info const &srcSh, tstring const &targetPath, tshortcut_info &destSh);
	bool CopyShortcut(tshortcut_info const&shortcut
		, tstring const& newShortcutPath
		, tshortcut_info &newShortcut
	);

	bool GetShortcutValue(tshortcut_info const& sh, tstring& DestValue);

//NF-catalogs operations
	namespace Private {
		bool remove_catalog(tstring const& catalog, tstring const* pTargetCatalog, bool bDeleteSource);
	}
	
	/// InsertCatalog supports recreation whole catalog hierarchy, i.e. "a/b/c/d/"
	bool InsertCatalog(tstring const& catalog, tstring const& subCatalogName);
	inline bool InsertCatalog(tstring const& catalog) { 
		return InsertCatalog(catalog, L"");
	}
	inline bool DeleteCatalog(tstring const& catalog) {
		return Private::remove_catalog(catalog, 0, true);
	}

	inline bool MoveCatalog(tstring const& catalog, tstring const& targetCatalog) {
		return Private::remove_catalog(catalog, &targetCatalog, true);
	}

	inline bool CopyCatalog(tstring const& catalog, tstring const& targetCatalog) {
		return Private::remove_catalog(catalog, &targetCatalog, false);
	}
} 
}