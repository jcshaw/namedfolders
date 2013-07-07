/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once
#include "stdafx.h"
#include "stlsoft_def.h"
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include "stlcatalogs.h"
#include "Registry.h"

#include "Kernel.h"
#include "Registry.h"
#include "strings_utils.h"
#include "PanelInfoWrap.h"
#include "PluginSettings.h"

extern struct PluginStartupInfo g_PluginInfo; 

nf::sc::CCatalog::CCatalog() 
: _Key(0)
{

}

nf::sc::CCatalog::~CCatalog() {
	
}

nf::sc::CCatalog::CCatalog(tstring const& subCatalog, CCatalog const *pParent, bool bCreateIfNotExists) {
	_CatalogPath = get_combined_path(subCatalog, pParent);
	_Key = PluginSettings::FarOpenKey(pParent == nullptr ? 0 : pParent->get_key_handle(), subCatalog);
	if (PluginSettings::isInvalidHandle(_Key) && bCreateIfNotExists) {
		_Key = PluginSettings::FarCreateKey(pParent == nullptr ? 0 : pParent->get_key_handle(), subCatalog);
	}
}

//копирующий конструктор
nf::sc::CCatalog::CCatalog(CCatalog const &catalog) 
: _Key(catalog._Key)
, _CatalogPath(catalog._CatalogPath) 
{
}

bool nf::sc::CCatalog::IsSubcatalogExist(tstring const& subCatalog) {
	auto subcatalogs_key = PluginSettings::FarOpenKey(_Key, GetKeyName(REG_SUB_CATALOGS));
	if (PluginSettings::isInvalidHandle(subcatalogs_key)) {
		return false;
	}
	return ! PluginSettings::isInvalidHandle(PluginSettings::FarOpenKey(subcatalogs_key, subCatalog));
}

bool nf::sc::CCatalog::InsertSubcatalog(tstring const& name) {
	auto subcatalogs_key = PluginSettings::FarCreateKey(_Key, GetKeyName(REG_SUB_CATALOGS));
	return ! PluginSettings::isInvalidHandle(PluginSettings::FarCreateKey(subcatalogs_key, name));
}

bool nf::sc::CCatalog::DeleteSubcatalog(tstring const& name) {
	auto subcatalogs_key = PluginSettings::FarCreateKey(_Key, GetKeyName(REG_SUB_CATALOGS));
	return PluginSettings::FarDeleteKey(_Key, name);
}

bool nf::sc::CCatalog::GetShortcutInfo(tstring const& shName, bool bTemporary, tstring &destValue) {	
	if (shName.size() && shName[0] == L'\\') { //!TODO: зачем эта проверка?
		tstring subdirectory(shName, 1, shName.size()-1);
		destValue = Utils::CombinePath(CPanelInfoWrap(INVALID_HANDLE_VALUE).GetPanelCurDir(true), subdirectory, SLASH_DIRS);
		return true;
	} else {
		return get_value(bTemporary ? REG_TEMP_KEYS : REG_STATIC_KEYS, shName, destValue);
	}
}

std::list<tstring> nf::sc::CCatalog::get_combined_path(tstring const& catalog, CCatalog const *parent) {	
	auto path = parent == nullptr ? std::list<tstring>() : parent->_CatalogPath;
	auto addons = path2list(catalog);
	BOOST_FOREACH(auto const& item, addons) {
		path.push_back(item);
	}
	return path;
}

wchar_t const* nf::sc::CCatalog::GetKeyName(tregs_enum Index) {
	static wchar_t const* regs[] = {L"Values", L"TempValues", L"Catalogs", L"Properties"};
	return regs[static_cast<int>(Index)];
};

bool nf::sc::CCatalog::set_value(tregs_enum regKey, tstring const& name, tstring const& keyValue) {
	auto key = PluginSettings::FarOpenKey(_Key, GetKeyName(regKey));
	if (PluginSettings::isInvalidHandle(key)) {
		return false;
	}
	return PluginSettings::FarSet(key, name, keyValue);
}

bool nf::sc::CCatalog::delete_value(tregs_enum regKey, tstring const& srcName) {
	auto key = PluginSettings::FarOpenKey(_Key, GetKeyName(regKey));
	if (PluginSettings::isInvalidHandle(key)) {
		return false;
	}
	return PluginSettings::FarDeleteKey(key, srcName);
}

bool nf::sc::CCatalog::get_value(tregs_enum regKey, tstring const& name, tstring& destValue) {
	auto key = PluginSettings::FarOpenKey(_Key, GetKeyName(regKey));
	if (PluginSettings::isInvalidHandle(key)) {
		return false;
	}
	return PluginSettings::FarGet(key, name, destValue);
}

std::list<tstring> nf::sc::CCatalog::path2list(tstring const& path) {
	std::list<tstring> dest;
	dest.push_back(path);
	return dest; //!TODO
}

tstring nf::sc::CCatalog::getCatalogPath() const {
	if (_CatalogPath.empty()) {
		return tstring();
	}
	if (_CatalogPath.size() == 1) {
		return *_CatalogPath.begin();
	}
	tstring ss; //avoid stringstream because paths are short 
	bool first = true;
	BOOST_FOREACH(tstring const& item, _CatalogPath) {
		if (first) {
			ss += SLASH_CATS;
			first = false;
		}
		ss  += item;
	}

	return ss;
}
