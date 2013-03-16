/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once
#include "Kernel.h"
#include "strings_utils.h"
#include "settings.h"
#include "registry_functions.h"
#include <boost/shared_ptr.hpp>

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
	CCatalog(tstring subCatalog, CCatalog const *pParent = 0, bool bCreateIfNotExists = true); //constructor for subcatalog of catalog
	~CCatalog();
public:
	inline tstring const& CatalogPath() const {//путь к каталогу относительно корневого каталога	
		return m_CatalogPath; 
	}
	inline tstring GetCatalogRegkey() const {
		return get_catalog_regkey(m_CatalogPath); 
	}
	inline size_t GetNumberSubcatalogs() const {
		return get_number_keys<WinSTL::reg_key_sequence_t>(GetKeyName(REG_SUB_CATALOGS)); 
	}
	inline size_t GetNumberShortcuts() const {
		return get_number_keys<WinSTL::reg_value_sequence_t>(GetKeyName(REG_STATIC_KEYS)); 
	}
public:	
	inline basic_class GetSequenceShortcuts(bool bTemporary) {
		return get_sequence(GetKeyName(bTemporary ? REG_TEMP_KEYS : REG_STATIC_KEYS)); 
	}
	inline basic_class GetSequenceSubcatalogs() {
		return get_sequence(GetKeyName(REG_SUB_CATALOGS)); 
	}
	inline basic_class GetSequenceProperties() {
		return get_sequence(GetKeyName(REG_PROPERTIES)); 
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
	inline basic_class get_sequence(wchar_t const* subkey) { 
		return m_key.has_sub_key(subkey)
			? basic_class(m_key.get_key_handle(), subkey)
			: m_key.create_sub_key(subkey);
	}
	template<class T>
	inline size_t get_number_keys(wchar_t const* skey) const {
		if (! nf::Registry::IsSubkeyExist(m_key.get_key_handle(), skey)) return 0;
		WinSTL::reg_key_t c(m_key.get_key_handle(), skey);
		return T(c).size();
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
};
} //sc
} //nf