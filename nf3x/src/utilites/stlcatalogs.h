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
		, REG_B_SUB_CATALOGS_B
		, REG_PROPERTIES
	};
public:
	CCatalog(); //default - root NF-catalog
	CCatalog(CCatalog const &catalog); 
	CCatalog(tstring const& subCatalog, CCatalog const *pParent = 0, bool bCreateIfNotExists = true); //constructor for subcatalog of catalog
	~CCatalog();
public:
	inline tstring const& CatalogPath() const {//путь к каталогу относительно корневого каталога	
		return m_CatalogPath; 
	}
	inline tstring GetCatalogRegkey() const {
		return get_catalog_regkey(m_CatalogPath); 
	}
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
		return set_key((bTemporary) ? REG_TEMP_KEYS : REG_STATIC_KEYS, Name, Value); 
	}
	bool DeleteShortcut(tstring const& Name, bool bTemporary) {
		return delete_key((bTemporary) ? REG_TEMP_KEYS : REG_STATIC_KEYS, Name); 
	}
	bool GetShortcutInfo(tstring const& Name, bool bTemporary, tstring &Value);

public:
	bool SetProperty(tstring const& propertyName, tstring const& propertyValue) {
		return set_key(REG_PROPERTIES, propertyName, propertyValue); 
	}
	bool DeleteProperty(tstring const& propertyName) {
		return delete_key(REG_PROPERTIES, propertyName); 
	}
	bool GetProperty(tstring const& propertyName, tstring& destValue) {
		return get_key_value(REG_PROPERTIES, propertyName, destValue);
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
	tstring get_combined_path(wchar_t const* catalog, CCatalog const *parent = 0); //получить полный путь к каталогу относительно корневого пути
	tstring get_catalog_regkey(tstring catalogName) const;
	inline static tstring const& get_far_reg_key() { 
		return nf::CSettings::GetInstance().get_NamedFolders_reg_key();
	}
	static wchar_t const* GetKeyName(tregs_enum Index);
	bool set_key(tregs_enum regKey, tstring const& srcName, tstring const& srcValue);
	bool delete_key(tregs_enum regKey, tstring const& srcName);
	bool get_key_value(tregs_enum regKey, tstring const& srcName, tstring& destValue);
private: //members
	boost::shared_ptr<FarSettingsItem> _pFSI;
	tstring m_CatalogPath;
	PluginSettings::tsettings_handle _Key;
};
} //sc
} //nf