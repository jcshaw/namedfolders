/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once
#include "main/header.h"
#include "Kernel.h"
#include "strings_utils.h"
#include "settings.h"
#include "registry_functions.h"
#include <boost/shared_ptr.hpp>
#include "CatalogSequences.h"

namespace nf {
	class FarCatalog;
namespace sc {

/// Catalog of Named Folders
/// NF 4.x stores data using standard FAR settings (sqllite).
/// So, Catalog is wrapper around HANDLE received through FAR3::SettingsControl 
class CCatalog {	
public:
	CCatalog(); //default - root NF-catalog
	CCatalog(CCatalog const &catalog); 
	CCatalog(tstring const& subCatalog, CCatalog const *pParent = 0, bool bCreateIfNotExists = true); //constructor for subcatalog of catalog
	~CCatalog();
public:
	/// @return full path to catalog, i.e. "a/b/c"
	tstring getCatalogPath() const;
	boost::shared_ptr<FarCatalog> getFarCatalog() const;
	size_t GetNumberSubcatalogs() const;
	size_t GetNumberShortcuts() const;
public:	
	//TODO: c11: use unique_ptr instead of shared ptr
	boost::shared_ptr<nf::SequenceValues> GetSequenceShortcuts(bool bTemporary);
	//TODO: c11: use unique_ptr instead of shared ptr
	boost::shared_ptr<nf::SequenceCatalogs> GetSequenceSubcatalogs();
public: 
	bool IsSubcatalogExist(tstring const& subCatalog);
	bool InsertSubcatalog(tstring const& Name); 
	bool DeleteSubcatalog(tstring const& Name);

	bool SetShortcut(tstring const& Name, tstring const& Value, bool bTemporary);
	bool DeleteShortcut(tstring const& Name, bool bTemporary);
	bool GetShortcutInfo(tstring const& Name, bool bTemporary, tstring &Value);

public:
	bool SetProperty(tstring const& propertyName, tstring const& propertyValue);
	bool DeleteProperty(tstring const& propertyName);
	bool GetProperty(tstring const& propertyName, tstring& destValue);
public:
	/// delete key and its children
	static bool eraseKey(tstring const &keyPath);
	/// move content between keys
	static bool moveKey(tstring const &srcPath, tstring const &targetPath);
	/// copy content from key to key
	static bool copyKey(tstring const &srcPath, tstring const &targetPath);

public:
	// a/b/c -> Catalogs/a/Catalogs/b/Catalogs/c
	static std::list<tstring> generateInternalPath(std::list<tstring> const& src);

	/// a/b/c -> "Catalogs/a/Catalogs/b/Catalogs/c"
	static bool copy_or_rename(tstring const &srcPath, tstring const &targetPath, bool renameEnabled);
private: //members
	boost::shared_ptr<FarSettingsItem> _pFSI;
	boost::shared_ptr<FarCatalog> _pFarCatalog; //!TODO: is it possible to use unique_ptr?
};
} //sc
} //nf