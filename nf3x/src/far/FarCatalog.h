#pragma once

#include <boost/shared_ptr.hpp>
#include "PluginSettings.h"

namespace nf {

	/** 
	
	*/
	class FarCatalog {
	public:
		FarCatalog(nf::tlist_strings const& path, bool bCreateIfNotExists);
		FarCatalog(tstring const& path, bool bCreateIfNotExists);
		FarCatalog(FarCatalog const* parent, tstring const& path, bool bCreateIfNotExists);
		FarCatalog(FarCatalog const& fc);
		~FarCatalog(void);

		nf::PluginSettings::tsettings_handle openFarHandle(nf::PluginSettings& ps, bool bCreateIfNotExists) const;
	public:

		bool isSubkeyExists(tstring const& subKeyName) const;
		bool createSubkey(tstring const& subKeyName);
		bool deleteSubkey(tstring const& subKeyName);

		boost::shared_ptr<FarCatalog> openSubKey(tstring const& subKeyName, bool createIfNotExists);

		bool getValue(tstring const& valueName, tstring& destValue, tspec_folders specFolder) const;
		bool getValue(tstring const& valueName, __int64& destValue) const;
		bool getValue(tstring const& valueName, tstring& destValue) const;

		bool setValue(tstring const& valueName, tstring const& requiredValue, tspec_folders specFolder);
		bool setValue(tstring const& valueName, __int64 destValue);
		bool setValue(tstring const& valueName, tstring const& requiredValue);

		bool deleteValue(tstring const& valueName);
		bool deleteValue(tstring const& valueName, tspec_folders specFolder);

		bool deleteThisKey();
		std::list<tstring> const& getCatalogPath() const {
			return _CatalogPath;
		}
	public:
		static wchar_t const* GetSpecFolderName(tspec_folders Index);

	private:
		/// _InternalPath + REG_SUB_CATALOGS
		nf::PluginSettings::tsettings_handle openFarHandleForSubcatalog(nf::PluginSettings& ps, bool bCreateIfNotExists, wchar_t const* subCatalog) const;
		nf::PluginSettings::tsettings_handle openFarHandleForShortcut(nf::PluginSettings& ps, bool bCreateIfNotExists, tspec_folders specFolder) const;
	private:
		/// contains "a", "b", "c" for catalog "a/b/c"
		std::list<tstring> _CatalogPath;
		/// Catalogs/a/Catalogs/b/Catalogs/c
		std::list<tstring> _InternalPath;
	};

}