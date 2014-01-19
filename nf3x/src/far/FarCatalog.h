#pragma once

#include <boost/shared_ptr.hpp>
#include "PluginSettings.h"
#include "FarSettingsKeyWrapper.h"

namespace nf {

	/** 
	
	*/
	class FarCatalog : public FarSettingsKeyWrapper {
	public:		
		FarCatalog(nf::tlist_strings const& path, bool bCreateIfNotExists);
		FarCatalog(tstring const& path, bool bCreateIfNotExists);
		FarCatalog(FarCatalog const* parent, tstring const& path, bool bCreateIfNotExists);
		//FarCatalog(FarCatalog const& fc);
		virtual ~FarCatalog(void);
	public:
		static wchar_t const* GetSpecFolderName(tspec_folders Index);

		bool isSubkeyExists(tstring const& subKeyName) const;
		bool createSubkey(tstring const& subKeyName);
		bool deleteSubkey(tstring const& subKeyName);

		boost::shared_ptr<FarCatalog> openSubKey(tstring const& subKeyName, bool createIfNotExists);
		boost::shared_ptr<FarSettingsKeyWrapper> openPlainSubKey(tstring const& subKeyName);

		bool getValue(tstring const& valueName, tstring& destValue, tspec_folders specFolder) const;
		bool setValue(tstring const& valueName, tstring const& requiredValue, tspec_folders specFolder);
		bool deleteValue(tstring const& valueName, tspec_folders specFolder);


	protected:
		virtual std::list<tstring> get_internal_path() const override {
			return _InternalPath;
		}
	private:
		nf::PluginSettings::tsettings_handle openFarHandleForKey(nf::PluginSettings& ps, bool bCreateIfNotExists, tspec_folders specFolder) const;
		/// _InternalPath + REG_SUB_CATALOGS
		nf::PluginSettings::tsettings_handle openFarHandleForFolder(nf::PluginSettings& ps, bool bCreateIfNotExists, wchar_t const* subCatalog) const;
		void ensure_exists(bool createIfNotExists);
	private:
		///for "a/b/c": 	"Catalogs/a/Catalogs/b/Catalogs/c"
		std::list<tstring> _InternalPath;
	};

}