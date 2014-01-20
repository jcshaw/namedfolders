/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

namespace nf {
namespace sc {
	extern wchar_t const* CATALOG_PROPERTY_SHOW_IN_DISK_MENU;
	extern wchar_t const* CATALOG_PROPERTY_SHOW_IN_DISK_MENU_GUID;
	extern wchar_t const* CATALOG_PROPERTY_FLAG_ADD_TO_WIN7_BY_DEFAULT;
	extern wchar_t const* CATALOG_PROPERTY_DEFAULT_WIN7_LIB;
	bool IsCatalogExist(tstring const& pathCatalog);

//routines for extended catalog properties
//these properties are stored in "Catalogs\CatalogName\properties\PropertyName" keys
	void GetDefaultCatalogProperties(tcatalog_properties &destP);
	void LoadCatalogProperties(tstring const& srcCatalogPath, tcatalog_properties &destP);
	void SaveCatalogProperties(tstring const& srcCatalogPath, tcatalog_properties const& srcP);


} //sc
} //nf
