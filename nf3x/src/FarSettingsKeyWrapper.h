#pragma once

#include <boost/shared_ptr.hpp>
#include "PluginSettings.h"

namespace nf {
class FarSettingsKeyWrapper
{
public:		
	FarSettingsKeyWrapper(nf::tlist_strings const& path, bool bCreateIfNotExists);
	FarSettingsKeyWrapper(tstring const& path, bool bCreateIfNotExists);
	FarSettingsKeyWrapper(FarSettingsKeyWrapper const* parent, tstring const& path, bool bCreateIfNotExists);
	FarSettingsKeyWrapper(FarSettingsKeyWrapper const& fc);
	virtual ~FarSettingsKeyWrapper(void);

	nf::PluginSettings::tsettings_handle openFarHandle(nf::PluginSettings& ps, bool bCreateIfNotExists) const;
public:
	bool getValue(tstring const& valueName, __int64& destValue) const;
	bool getValue(tstring const& valueName, tstring& destValue) const;

	bool setValue(tstring const& valueName, __int64 destValue);
	bool setValue(tstring const& valueName, tstring const& requiredValue);

	bool deleteValue(tstring const& valueName);

	bool deleteThisKey();
	std::list<tstring> const& getCatalogPath() const {
		return _CatalogPath;
	}
protected:
	virtual std::list<tstring> get_internal_path() const {
		return _CatalogPath;
	}
	static nf::PluginSettings::tsettings_handle openFarHandle(nf::PluginSettings& ps, std::list<tstring> internalPath, bool bCreateIfNotExists);
private:
	/// contains "a", "b", "c" for catalog "a/b/c"
	std::list<tstring> _CatalogPath;	
};

}