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
namespace sc {

/// Catalog of Named Folders
/// NF 4.x stores data using standard FAR settings (sqllite).
/// So, Catalog is wrapper around HANDLE received through FAR3::SettingsControl 
class CCatalog {	
	typedef enum tregs_enum	{REG_STATIC_KEYS
		, REG_TEMP_KEYS
		, REG_SUB_CATALOGS
		, REG_PROPERTIES
	};
public:
	CCatalog(); //default - root NF-catalog
	CCatalog(CCatalog const &catalog); 
	CCatalog(tstring const& subCatalog, CCatalog const *pParent = 0, bool bCreateIfNotExists = true); //constructor for subcatalog of catalog
	~CCatalog();
public:
	/// @return full path to catalog, i.e. "a/b/c"
	tstring getCatalogPath() const;
	inline size_t GetNumberSubcatalogs() const {
		return get_sequence<tstring>(GetKeyName(REG_SUB_CATALOGS))->getItems().size();			
	}
	inline size_t GetNumberShortcuts() const {
		return get_sequence<tstring>(GetKeyName(REG_STATIC_KEYS))->getItems().size(); 
	}
	PluginSettings::tkey_handle get_key_handle() const {
		return _Key;
	}
public:	
	//TODO: c11: use unique_ptr instead of shared ptr
	inline boost::shared_ptr<nf::SequenceShortcuts> GetSequenceShortcuts(bool bTemporary) {
		return get_sequence<nf::shortcuts_sequence_item>(GetKeyName(bTemporary ? REG_TEMP_KEYS : REG_STATIC_KEYS)); 
	}
	//TODO: c11: use unique_ptr instead of shared ptr
	inline boost::shared_ptr<nf::SequenceItems> GetSequenceSubcatalogs() {
		return get_sequence<nf::catalogs_sequence_item>(GetKeyName(REG_SUB_CATALOGS)); 
	}
public: 
	bool IsSubcatalogExist(tstring const& subCatalog);
	bool InsertSubcatalog(tstring const& Name); 
	bool DeleteSubcatalog(tstring const& Name);

	bool SetShortcut(tstring const& Name, tstring const& Value, bool bTemporary) {
		return set_value((bTemporary) ? REG_TEMP_KEYS : REG_STATIC_KEYS, Name, Value); 
	}
	bool DeleteShortcut(tstring const& Name, bool bTemporary) {
		return delete_value((bTemporary) ? REG_TEMP_KEYS : REG_STATIC_KEYS, Name); 
	}
	bool GetShortcutInfo(tstring const& Name, bool bTemporary, tstring &Value);

public:
	bool SetProperty(tstring const& propertyName, tstring const& propertyValue) {
		return set_value(REG_PROPERTIES, propertyName, propertyValue); 
	}
	bool DeleteProperty(tstring const& propertyName) {
		return delete_value(REG_PROPERTIES, propertyName); 
	}
	bool GetProperty(tstring const& propertyName, tstring& destValue) {
		return get_value(REG_PROPERTIES, propertyName, destValue);
	}
private: 
	template<class T>
	inline boost::shared_ptr<nf::SequenceSettings<T>> get_sequence(wchar_t const* subkey) const { 
		auto sk = PluginSettings::FarOpenKey(_Key, subkey);
		if (sk == 0) {
			sk = PluginSettings::FarCreateKey(_Key, subkey);
		} 
		return boost::shared_ptr<nf::SequenceSettings<T>>(new nf::SequenceSettings<T>(sk));
	}
	static wchar_t const* GetKeyName(tregs_enum Index);
	bool set_value(tregs_enum regKey, tstring const& srcName, tstring const& srcValue);
	bool delete_value(tregs_enum regKey, tstring const& srcName);
	bool get_value(tregs_enum regKey, tstring const& srcName, tstring& destValue);

	/// a/b/c -> "Catalogs/a/Catalogs/b/Catalogs/c"
	std::list<tstring> prepare_full_path(std::list<tstring> src);
private: //members
	boost::shared_ptr<FarSettingsItem> _pFSI;

	/// contains "a", "b", "c" for catalog "a/b/c"
	std::list<tstring> _CatalogPath;
	PluginSettings::tsettings_handle _Key;
};
} //sc
} //nf