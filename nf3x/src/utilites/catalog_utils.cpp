/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "stdafx.h"
#include "catalog_utils.h"
#include "stlcatalogs.h"

wchar_t const* nf::sc::CATALOG_PROPERTY_SHOW_IN_DISK_MENU = L"show_in_disk_menu";
wchar_t const* nf::sc::CATALOG_PROPERTY_FLAG_ADD_TO_WIN7_BY_DEFAULT = L"flag_add_to_win7_lib_by_default";
wchar_t const* nf::sc::CATALOG_PROPERTY_DEFAULT_WIN7_LIB = L"default_win7_lib";

void nf::sc::GetDefaultCatalogProperties(tcatalog_properties &destP) {
	destP.show_in_disk_menu = false;
	destP.flag_add_to_win7_lib_by_default = false;
	destP.default_win7_lib.clear();
}

void nf::sc::LoadCatalogProperties(tstring const& srcCatalogPath, tcatalog_properties &destP) {
	CCatalog catalog(srcCatalogPath);
	GetDefaultCatalogProperties(destP);

	tstring dest;
	if (catalog.GetProperty(CATALOG_PROPERTY_SHOW_IN_DISK_MENU, dest)) {
		destP.show_in_disk_menu = dest.size() != 0 && dest[0] != L'0';
	}
	if (catalog.GetProperty(CATALOG_PROPERTY_FLAG_ADD_TO_WIN7_BY_DEFAULT, dest)) {
		destP.flag_add_to_win7_lib_by_default = dest.size() != 0 && dest[0] != L'0';
	}
	if (catalog.GetProperty(CATALOG_PROPERTY_DEFAULT_WIN7_LIB, dest)) {
		destP.default_win7_lib.swap(dest);
	}
}

void nf::sc::SaveCatalogProperties(tstring const& srcCatalogPath, tcatalog_properties const& srcP) {
	CCatalog catalog(srcCatalogPath);
	catalog.SetProperty(CATALOG_PROPERTY_SHOW_IN_DISK_MENU, srcP.show_in_disk_menu ? L"1" : L"0");
	catalog.SetProperty(CATALOG_PROPERTY_FLAG_ADD_TO_WIN7_BY_DEFAULT, srcP.flag_add_to_win7_lib_by_default ? L"1" : L"0");
	catalog.SetProperty(CATALOG_PROPERTY_DEFAULT_WIN7_LIB, srcP.default_win7_lib);
}

namespace {
	bool check_if_catalog_exists(nf::sc::CCatalog &parent, std::list<tstring>& paths) {
		if (paths.empty()) return false;
		tstring root = paths.front();
		paths.pop_front();
		if (! parent.IsSubcatalogExist(root)) {
			return false;
		}
		if (paths.empty()) return true;
		return check_if_catalog_exists(nf::sc::CCatalog(root, &parent), paths);
	}
}

bool nf::sc::IsCatalogExist(tstring const& pathCatalog) {
	tstring s = Utils::RemoveTrailingChars(pathCatalog, SLASH_CATS_CHAR);
	Utils::RemoveLeadingCharsOnPlace(s, SLASH_CATS_CHAR);

	std::list<tstring> list;
	Utils::SplitStringByRegex(s, list, SLASH_CATS);

	return check_if_catalog_exists(nf::sc::CCatalog(), list);
}