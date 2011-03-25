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

void nf::sc::GetDefaultCatalogProperties(tcatalog_properties &destP) {
	destP.show_in_disk_menu = false;
}

void nf::sc::LoadCatalogProperties(tstring const& srcCatalogPath, tcatalog_properties &destP) {
	CCatalog catalog(srcCatalogPath);
	GetDefaultCatalogProperties(destP);

	tstring dest;
	if (catalog.GetProperty(CATALOG_PROPERTY_SHOW_IN_DISK_MENU, dest)) {
		destP.show_in_disk_menu = dest.size() != 0 && dest[0] != L'0';
	}
}

void nf::sc::SaveCatalogProperties(tstring const& srcCatalogPath, tcatalog_properties const& srcP) {
	CCatalog catalog(srcCatalogPath);
	catalog.SetProperty(CATALOG_PROPERTY_SHOW_IN_DISK_MENU, srcP.show_in_disk_menu ? L"1" : L"0");
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