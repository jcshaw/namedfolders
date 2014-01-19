#include "stdafx.h"
#include "FarCatalog.h"

#include <boost/foreach.hpp>
#include "stlcatalogs.h"
#include "strings_utils.h"

nf::FarCatalog::FarCatalog(nf::tlist_strings const& path, bool bCreateIfNotExists) 
: FarSettingsKeyWrapper(path)
, _InternalPath(nf::sc::CCatalog::generateInternalPath(getCatalogPath()))
{
	ensure_exists(bCreateIfNotExists);
}

nf::FarCatalog::FarCatalog(tstring const& path, bool bCreateIfNotExists) 
: FarSettingsKeyWrapper(path) 
, _InternalPath(nf::sc::CCatalog::generateInternalPath(getCatalogPath()))
{
	ensure_exists(bCreateIfNotExists);
}

nf::FarCatalog::FarCatalog(FarCatalog const* parent, tstring const& path, bool bCreateIfNotExists) 
: FarSettingsKeyWrapper(parent, path)
, _InternalPath(nf::sc::CCatalog::generateInternalPath(getCatalogPath()))
{
	ensure_exists(bCreateIfNotExists);
}

nf::FarCatalog::~FarCatalog(void)
{
}


wchar_t const* nf::FarCatalog::GetSpecFolderName(tspec_folders Index) {
	static wchar_t const* regs[] = {L"Values", L"TempValues", L"Catalogs", L"Properties"};
	return regs[static_cast<int>(Index)];
};

bool nf::FarCatalog::getValue(tstring const& valueName, tstring& destValue, tspec_folders specFolder) const {
	PluginSettings ps;
	auto key = openFarHandleForKey(ps, false, specFolder);
	return ps.FarGet(key, valueName, destValue);
}

bool nf::FarCatalog::setValue(tstring const& valueName, tstring const& destValue, tspec_folders specFolder) {
	PluginSettings ps;
	auto key = openFarHandleForKey(ps, true, specFolder);
	return ps.FarSet(key, valueName, destValue);
}

bool nf::FarCatalog::deleteValue(tstring const& nameValue, tspec_folders specFolder) {
	PluginSettings ps;
	auto key = openFarHandleForKey(ps, false, specFolder);
	return ps.FarDeleteValue(key, nameValue);
}


nf::PluginSettings::tsettings_handle nf::FarCatalog::openFarHandleForFolder(nf::PluginSettings& ps, bool bCreateIfNotExists, wchar_t const* subCatalog) const {
	std::list<tstring> ipath = get_internal_path();
	ipath.push_back(GetSpecFolderName(REG_SUB_CATALOGS));
	if (subCatalog != nullptr) {
		ipath.push_back(subCatalog);
	}
	return nf::FarCatalog::openFarHandle(ps, ipath, true);
}

nf::PluginSettings::tsettings_handle nf::FarCatalog::openFarHandleForKey(nf::PluginSettings& ps, bool bCreateIfNotExists, tspec_folders specFolder) const {
	std::list<tstring> ipath = get_internal_path();
	ipath.push_back(GetSpecFolderName(specFolder));
	return nf::FarCatalog::openFarHandle(ps, ipath, bCreateIfNotExists);
}

bool nf::FarCatalog::isSubkeyExists(tstring const& subKeyName) const {
	return ! PluginSettings::isInvalidHandle(openFarHandleForFolder(PluginSettings(), false, subKeyName.c_str()));
}

bool nf::FarCatalog::createSubkey(tstring const& subKeyName) {
	return ! PluginSettings::isInvalidHandle(openFarHandleForFolder(PluginSettings(), true, subKeyName.c_str()));
}

bool nf::FarCatalog::deleteSubkey(tstring const& subKeyName) {
	PluginSettings ps;
	auto key = openFarHandleForFolder(ps, false, nullptr);
	return ps.FarDeleteKey(key, subKeyName);
}

boost::shared_ptr<nf::FarCatalog> nf::FarCatalog::openSubKey(tstring const& subKeyName, bool createIfNotExists) {
	return boost::shared_ptr<FarCatalog>(new FarCatalog(this, subKeyName, createIfNotExists) );
}

boost::shared_ptr<nf::FarSettingsKeyWrapper> nf::FarCatalog::openPlainSubKey(tstring const& subKeyName) {
	auto ipath = this->get_internal_path();
	ipath.push_back(subKeyName);

	return boost::shared_ptr<FarSettingsKeyWrapper>(new FarSettingsKeyWrapper(ipath) );
}

void nf::FarCatalog::ensure_exists(bool bCreateIfNotExists) {
	if (bCreateIfNotExists) {
		nf::PluginSettings ps;
		openFarHandle(ps, bCreateIfNotExists);
	}
}
