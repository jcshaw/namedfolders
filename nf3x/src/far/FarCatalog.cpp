#include "stdafx.h"
#include "FarCatalog.h"

#include <boost/foreach.hpp>
#include "stlcatalogs.h"
#include "strings_utils.h"

nf::FarCatalog::FarCatalog(nf::tlist_strings const& path, bool bCreateIfNotExists) 
: _InternalPath(nf::sc::CCatalog::generateInternalPath(path))
, _CatalogPath(path)
{
	nf::PluginSettings ps;
	if (bCreateIfNotExists) {
		openFarHandle(ps, bCreateIfNotExists);
	}
}

nf::FarCatalog::FarCatalog(tstring const& path, bool bCreateIfNotExists) {
	Utils::add_to_path(_CatalogPath, path);
	_InternalPath = nf::sc::CCatalog::generateInternalPath(_CatalogPath);

	nf::PluginSettings ps;
	if (bCreateIfNotExists) {
		openFarHandle(ps, bCreateIfNotExists);
	}

}

nf::FarCatalog::FarCatalog(FarCatalog const& fc) 
: _CatalogPath(fc._CatalogPath) 
, _InternalPath(fc._InternalPath)
{

}

nf::FarCatalog::FarCatalog(FarCatalog const* parent, tstring const& path, bool bCreateIfNotExists) {
	nf::tlist_strings addons;
	Utils::add_to_path(_CatalogPath, path);

	if (parent != nullptr) {
		_CatalogPath = parent->_CatalogPath;
	}
	BOOST_FOREACH(auto const& a, addons) {
		_CatalogPath.push_back(a);
	}	

	_InternalPath = nf::sc::CCatalog::generateInternalPath(_CatalogPath);
	
	nf::PluginSettings ps;
	if (bCreateIfNotExists) {
		openFarHandle(ps, bCreateIfNotExists);
	}
}

nf::FarCatalog::~FarCatalog(void)
{
}

nf::PluginSettings::tsettings_handle nf::FarCatalog::openFarHandle(nf::PluginSettings& ps, bool bCreateIfNotExists) const{
	auto sk = ps.FarOpenKey(0, _InternalPath);
	if (sk == 0) {
		sk = ps.FarCreateKey(0, _InternalPath);
	} 
	return sk;
}

bool nf::FarCatalog::isSubkeyExists(tstring const& subKeyName) const {
	return ! PluginSettings::isInvalidHandle(openFarHandleForSubcatalog(PluginSettings(), false, subKeyName.c_str()));
}

bool nf::FarCatalog::createSubkey(tstring const& subKeyName) {
	return ! PluginSettings::isInvalidHandle(openFarHandleForSubcatalog(PluginSettings(), true, subKeyName.c_str()));
}

bool nf::FarCatalog::deleteSubkey(tstring const& subKeyName) {
	PluginSettings ps;
	auto key = openFarHandleForSubcatalog(ps, false, nullptr);
	return ps.FarDeleteKey(key, subKeyName);
}

boost::shared_ptr<nf::FarCatalog> nf::FarCatalog::openSubKey(tstring const& subKeyName, bool createIfNotExists) {
	return boost::shared_ptr<FarCatalog>(new FarCatalog(this, subKeyName, createIfNotExists) );
}

bool nf::FarCatalog::getValue(tstring const& valueName, tstring& destValue, tspec_folders specFolder) const {
	PluginSettings ps;
	auto key = openFarHandleForShortcut(ps, false, specFolder);
	return ps.FarGet(key, valueName, destValue);
}

bool nf::FarCatalog::getValue(tstring const& valueName, __int64& destValue) const {
	PluginSettings ps;
	auto key = openFarHandle(ps, false);
	return ps.FarGet(key, valueName, destValue);
}
bool nf::FarCatalog::getValue(tstring const& valueName, tstring& destValue) const {
	PluginSettings ps;
	auto key = openFarHandle(ps, false);
	return ps.FarGet(key, valueName, destValue);
}


bool nf::FarCatalog::setValue(tstring const& valueName, tstring const& destValue, tspec_folders specFolder) {
	PluginSettings ps;
	auto key = openFarHandleForShortcut(ps, true, specFolder);
	return ps.FarSet(key, valueName, destValue);
}

bool nf::FarCatalog::setValue(tstring const& valueName, __int64 destValue) {
	PluginSettings ps;
	auto key = openFarHandle(ps, true);
	return ps.FarSet(key, valueName, destValue);
}

bool nf::FarCatalog::setValue(tstring const& valueName, tstring const& destValue) {
	PluginSettings ps;
	auto key = openFarHandle(ps, true);
	return ps.FarSet(key, valueName, destValue);
}


bool nf::FarCatalog::deleteValue(tstring const& nameValue, tspec_folders specFolder) {
	PluginSettings ps;
	auto key = openFarHandleForShortcut(ps, false, specFolder);
	return ps.FarDeleteValue(key, nameValue);
}

bool nf::FarCatalog::deleteValue(tstring const& nameValue) {
	PluginSettings ps;
	auto key = openFarHandle(ps, false);
	return ps.FarDeleteValue(key, nameValue);
}


bool nf::FarCatalog::deleteThisKey() {
	PluginSettings ps;
	auto key = openFarHandle(ps, false);
	if (! PluginSettings::isInvalidHandle(key)) {
		return ps.FarDeleteKey(key);
	} else {
		return false;
	}
}

nf::PluginSettings::tsettings_handle nf::FarCatalog::openFarHandleForSubcatalog(nf::PluginSettings& ps, bool bCreateIfNotExists, wchar_t const* subCatalog) const {
	std::list<tstring> ipath = _InternalPath;
	ipath.push_back(GetSpecFolderName(REG_SUB_CATALOGS));
	if (subCatalog != nullptr) {
		ipath.push_back(subCatalog);
	}
	return openFarHandle(ps, true);
}

wchar_t const* nf::FarCatalog::GetSpecFolderName(tspec_folders Index) {
	static wchar_t const* regs[] = {L"Values", L"TempValues", L"Catalogs", L"Properties"};
	return regs[static_cast<int>(Index)];
};

nf::PluginSettings::tsettings_handle nf::FarCatalog::openFarHandleForShortcut(nf::PluginSettings& ps, bool bCreateIfNotExists, tspec_folders specFolder) const {
	std::list<tstring> ipath = _InternalPath;
	ipath.push_back(GetSpecFolderName(specFolder));
	return openFarHandle(ps, true);
}
