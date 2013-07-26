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

#include "Kernel.h"
#include "strings_utils.h"
#include "PanelInfoWrap.h"
#include "PluginSettings.h"
#include "strings_utils.h"

extern struct PluginStartupInfo g_PluginInfo; 

namespace {
	void add_to_path(std::list<tstring> &path, tstring const& pathToAdd) {
		nf::tlist_strings addons;
		Utils::SplitStringByRegex(Utils::TrimChar(pathToAdd, SLASH_CATS_CHAR).c_str(), addons, SLASH_CATS);
		BOOST_FOREACH(auto const& a, addons) {
			path.push_back(a);
		}
	}
	
}

nf::sc::CCatalog::CCatalog() 
: _Key(0)
{

}

nf::sc::CCatalog::~CCatalog() {
	
}

nf::sc::CCatalog::CCatalog(tstring const& subCatalog, CCatalog const *pParent, bool bCreateIfNotExists) 
{
	nf::tlist_strings addons;
	add_to_path(addons, subCatalog);

	_CatalogPath = pParent == nullptr ? std::list<tstring>() : pParent->_CatalogPath;
	BOOST_FOREACH(auto const& a, addons) {
		_CatalogPath.push_back(a);
	}	

	auto full_path = prepare_full_path(addons);
	_Key = nf::PluginSettings::FarOpenKey(pParent == nullptr ? 0 : pParent->get_key_handle(), full_path);
	if (nf::PluginSettings::isInvalidHandle(_Key) && bCreateIfNotExists) {
		_Key = nf::PluginSettings::FarCreateKey(pParent == nullptr ? 0 : pParent->get_key_handle(), full_path);
	}
}

/// a/b/c -> "Catalogs/a/Catalogs/b/Catalogs/c"
std::list<tstring> nf::sc::CCatalog::prepare_full_path(std::list<tstring> const& src) {
	std::list<tstring> dest;
	auto const folder = GetSpecFolderName(REG_SUB_CATALOGS);
	BOOST_FOREACH(auto const& a, src) {
		dest.push_back(folder);
		dest.push_back(a);
	}
	return dest;
}

//копирующий конструктор
nf::sc::CCatalog::CCatalog(CCatalog const &catalog) 
: _Key(catalog._Key)
, _CatalogPath(catalog._CatalogPath) 
{
}

bool nf::sc::CCatalog::IsSubcatalogExist(tstring const& subCatalog) {
	auto subcatalogs_key = nf::PluginSettings::FarOpenKey(_Key, GetSpecFolderName(REG_SUB_CATALOGS));
	if (nf::PluginSettings::isInvalidHandle(subcatalogs_key)) {
		return false;
	}
	return ! nf::PluginSettings::isInvalidHandle(nf::PluginSettings::FarOpenKey(subcatalogs_key, subCatalog));
}

bool nf::sc::CCatalog::InsertSubcatalog(tstring const& name) {
	if (IsSubcatalogExist(name)) {
		return true;
	}
	auto subcatalogs_key = nf::PluginSettings::FarCreateKey(_Key, GetSpecFolderName(REG_SUB_CATALOGS));
	return ! nf::PluginSettings::isInvalidHandle(nf::PluginSettings::FarCreateKey(subcatalogs_key, name));
}

bool nf::sc::CCatalog::DeleteSubcatalog(tstring const& name) {
	auto subcatalogs_key = nf::PluginSettings::FarOpenKey(_Key, GetSpecFolderName(REG_SUB_CATALOGS));
	return nf::PluginSettings::FarDeleteKey(subcatalogs_key, name);
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

wchar_t const* nf::sc::CCatalog::GetSpecFolderName(tspec_folders Index) {
	static wchar_t const* regs[] = {L"Values", L"TempValues", L"Catalogs", L"Properties"};
	return regs[static_cast<int>(Index)];
};

bool nf::sc::CCatalog::set_value(tspec_folders specFolder, tstring const& name, tstring const& keyValue) {
	auto folder = GetSpecFolderName(specFolder);
	auto handle_folder = nf::PluginSettings::FarOpenKey(_Key, folder);
	if (nf::PluginSettings::isInvalidHandle(handle_folder)) {
		handle_folder = nf::PluginSettings::FarCreateKey(_Key, folder);
	}
	return nf::PluginSettings::FarSet(handle_folder, name, keyValue);
}

bool nf::sc::CCatalog::delete_value(tspec_folders specFolder, tstring const& srcName) {
	auto key = nf::PluginSettings::FarOpenKey(_Key, GetSpecFolderName(specFolder));
	if (nf::PluginSettings::isInvalidHandle(key)) {
		return false;
	}	
	return nf::PluginSettings::FarDeleteValue(key, srcName);
}

bool nf::sc::CCatalog::get_value(tspec_folders specFolder, tstring const& name, tstring& destValue) {
	auto key = nf::PluginSettings::FarOpenKey(_Key, GetSpecFolderName(specFolder));
	if (nf::PluginSettings::isInvalidHandle(key)) {
		return false;
	}
	return nf::PluginSettings::FarGet(key, name, destValue);
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
		if (! first) {
			ss += SLASH_CATS;
		} else {
			first = false;
		}
		ss  += item;
	}

	return ss;
}

bool nf::sc::CCatalog::eraseKey(tstring const &keyPath) {
	CCatalog c(keyPath);
	return c.deleteThisCatalog();
}

bool nf::sc::CCatalog::moveKey(tstring const &srcPath, tstring const &targetPath) {
	if (copyKey(srcPath, targetPath)) {
		return eraseKey(srcPath);
	} else {
		return false;
	}
}

namespace {
	tstring get_last_list_item(nf::tlist_strings const& list) {
		if (list.empty()) {
			return tstring();
		}
		auto last = list.end();
		--last;
		return *last;
	}
}

bool nf::sc::CCatalog::copyKey(tstring const &srcPath, tstring const &targetPath) {
	nf::tlist_strings src;
	add_to_path(src, srcPath);

	nf::tlist_strings dest;
	add_to_path(dest, targetPath);

	if (src.empty()) {
		return false;
	}
	if (targetPath.size() > srcPath.size()) {
		tstring part_target_path(targetPath, 0, srcPath.size());
		if (0 == lstrcmpi(part_target_path.c_str(), srcPath.c_str())) {
			return false; //it's not possible to copy "aa/bb/cc" to "aa/bb/cc/xx"
		}
	}

	auto src_last_folder = get_last_list_item(src);
	auto dest_last_folder = get_last_list_item(dest);
	bool dest_subcatalog_exists = 
		! src_last_folder.empty() 
		&& ! dest_last_folder.empty()
		&& 0 == lstrcmpi(src_last_folder.c_str(), dest_last_folder.c_str());

	auto src_full = prepare_full_path(src);
	auto dest_full = prepare_full_path(dest);

	if (! dest_subcatalog_exists) {
		dest_full.push_back(GetSpecFolderName(REG_SUB_CATALOGS));
		dest_full.push_back(src_last_folder);
	}
	
	return nf::PluginSettings::CopyKey(src_full, dest_full);
}

bool nf::sc::CCatalog::deleteThisCatalog() {
	return nf::PluginSettings::FarDeleteKey(_Key);
}
