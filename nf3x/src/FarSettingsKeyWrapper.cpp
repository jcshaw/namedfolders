#include "stdafx.h"
#include "FarSettingsKeyWrapper.h"

#include <boost/foreach.hpp>
#include "stlcatalogs.h"
#include "strings_utils.h"

nf::FarSettingsKeyWrapper::FarSettingsKeyWrapper(nf::tlist_strings const& path) 
: _CatalogPath(path)
{
}

nf::FarSettingsKeyWrapper::FarSettingsKeyWrapper(tstring const& path) {
	Utils::add_to_path(_CatalogPath, path);
}

nf::FarSettingsKeyWrapper::FarSettingsKeyWrapper(FarSettingsKeyWrapper const& fc) 
: _CatalogPath(fc._CatalogPath) 
{

}

nf::FarSettingsKeyWrapper::FarSettingsKeyWrapper(FarSettingsKeyWrapper const* parent, tstring const& path) {
	if (parent != nullptr) {
		_CatalogPath = parent->_CatalogPath;
	}
	Utils::add_to_path(_CatalogPath, path);
}

nf::FarSettingsKeyWrapper::~FarSettingsKeyWrapper(void)
{
}

nf::PluginSettings::tsettings_handle nf::FarSettingsKeyWrapper::openFarHandle(nf::PluginSettings& ps, bool bCreateIfNotExists) const{
	return nf::FarSettingsKeyWrapper::openFarHandle(ps, get_internal_path(), bCreateIfNotExists);
}

nf::PluginSettings::tsettings_handle nf::FarSettingsKeyWrapper::openFarHandle(nf::PluginSettings& ps, std::list<tstring> internalPath, bool bCreateIfNotExists) {
	auto sk = ps.FarOpenKey(0, internalPath);
	if (sk == 0 && bCreateIfNotExists) {
		sk = ps.FarCreateKey(0, internalPath);
	} 
	return sk;
}

bool nf::FarSettingsKeyWrapper::getValue(tstring const& valueName, __int64& destValue) const {
	PluginSettings ps;
	auto key = openFarHandle(ps, false);
	return ps.FarGet(key, valueName, destValue);
}
bool nf::FarSettingsKeyWrapper::getValue(tstring const& valueName, tstring& destValue) const {
	PluginSettings ps;
	auto key = openFarHandle(ps, false);
	return ps.FarGet(key, valueName, destValue);
}

bool nf::FarSettingsKeyWrapper::setValue(tstring const& valueName, __int64 destValue) {
	PluginSettings ps;
	auto key = openFarHandle(ps, true);
	return ps.FarSet(key, valueName, destValue);
}

bool nf::FarSettingsKeyWrapper::setValue(tstring const& valueName, tstring const& destValue) {
	PluginSettings ps;
	auto key = openFarHandle(ps, true);
	return ps.FarSet(key, valueName, destValue);
}


bool nf::FarSettingsKeyWrapper::deleteValue(tstring const& nameValue) {
	PluginSettings ps;
	auto key = openFarHandle(ps, false);
	return ps.FarDeleteValue(key, nameValue);
}


bool nf::FarSettingsKeyWrapper::deleteThisKey() {
	PluginSettings ps;
	auto key = openFarHandle(ps, false);
	if (! PluginSettings::isInvalidHandle(key)) {
		return ps.FarDeleteKey(key);
	} else {
		return false;
	}
}

