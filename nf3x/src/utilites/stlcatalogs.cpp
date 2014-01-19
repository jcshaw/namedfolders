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
#include "FarCatalog.h"
#include "FarSettingsKeyWrapper.h"

extern struct PluginStartupInfo g_PluginInfo; 

namespace {
	template<class T>
	inline boost::shared_ptr<nf::SequenceSettings<T>> get_sequence(nf::FarCatalog* pfc, nf::tspec_folders specFolder) { 
		nf::PluginSettings ps;
		boost::shared_ptr<nf::FarSettingsKeyWrapper> fs = pfc->openPlainSubKey(nf::FarCatalog::GetSpecFolderName(specFolder));
		return boost::shared_ptr<nf::SequenceSettings<T>>(new nf::SequenceSettings<T>(ps, fs->openFarHandle(ps, true)));
	}
}

nf::sc::CCatalog::CCatalog() 
{

}

nf::sc::CCatalog::~CCatalog() {
	
}

nf::sc::CCatalog::CCatalog(tstring const& subCatalog, CCatalog const *pParent, bool bCreateIfNotExists) 
{
	_pFarCatalog.reset(pParent == nullptr 
		? new FarCatalog(subCatalog, bCreateIfNotExists) 
		: new FarCatalog(pParent->getFarCatalog().get(), subCatalog, bCreateIfNotExists)
	);
}

//копирующий конструктор
nf::sc::CCatalog::CCatalog(CCatalog const &catalog) 
: _pFarCatalog(catalog.getFarCatalog())
{
}

bool nf::sc::CCatalog::IsSubcatalogExist(tstring const& subCatalog) {
	return _pFarCatalog->isSubkeyExists(subCatalog);
}

bool nf::sc::CCatalog::InsertSubcatalog(tstring const& name) {
	if (IsSubcatalogExist(name)) {
		return true;
	}
	return _pFarCatalog->createSubkey(name);
}

bool nf::sc::CCatalog::DeleteSubcatalog(tstring const& name) {
	return _pFarCatalog->deleteSubkey(name);
}

bool nf::sc::CCatalog::GetShortcutInfo(tstring const& shName, bool bTemporary, tstring &destValue) {	
	if (shName.size() && shName[0] == L'\\') { //!TODO: зачем эта проверка?
		tstring subdirectory(shName, 1, shName.size()-1);
		destValue = Utils::CombinePath(CPanelInfoWrap(INVALID_HANDLE_VALUE).GetPanelCurDir(true), subdirectory, SLASH_DIRS);
		return true;
	} else {
		if (_pFarCatalog->getValue(shName, destValue, REG_STATIC_KEYS)) {
			return true;
		}
		return _pFarCatalog->getValue(shName, destValue, REG_TEMP_KEYS);
	} 
}

/// a -> a
/// a,b -> a/b
/// !TODO: make more general, move to string_utils
tstring nf::sc::CCatalog::getCatalogPath() const {
	std::list<tstring> const& catalog_path = _pFarCatalog->getCatalogPath();
	if (catalog_path.empty()) {
		return tstring();
	}
	if (catalog_path.size() == 1) {
		return *catalog_path.begin();
	}
	tstring ss; //avoid stringstream because paths are short 
	bool first = true;
	BOOST_FOREACH(tstring const& item, catalog_path) {
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
	return c.getFarCatalog()->deleteThisKey();
}

bool nf::sc::CCatalog::moveKey(tstring const &srcPath, tstring const &targetPath) {
	if (copy_or_rename(srcPath, targetPath, true)) {
		return eraseKey(srcPath);
	} else {
		return false;
	}
}

bool nf::sc::CCatalog::copyKey(tstring const &srcPath, tstring const &targetPath) {
	return copy_or_rename(srcPath, targetPath, false);
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

bool nf::sc::CCatalog::copy_or_rename(tstring const &srcPath, tstring const &targetPath, bool renameEnabled) {
	nf::tlist_strings src;
	Utils::add_to_path(src, srcPath);

	nf::tlist_strings dest;
	Utils::add_to_path(dest, targetPath);

	if (src.empty()) {
		return false;
	}
	if (targetPath.size() > srcPath.size()) {
		tstring part_target_path(targetPath, 0, srcPath.size());
		part_target_path += L"/"; //to avoid problem with renaming "bb" to "bb2"
		if (0 == lstrcmpi(part_target_path.c_str(), srcPath.c_str())) {
			return false; //it's not possible to copy "aa/bb/cc" to "aa/bb/cc/xx"
		}
	} 

	bool rename = false;
	if (renameEnabled && src.size() == dest.size()) {
		auto ps = src.begin();
		auto pd = dest.begin();
		size_t n = src.size();
		while (ps != src.end() && pd != dest.end() && *ps == *pd) {
			++ps;
			++pd;
			--n;
		}
		rename = n == 1; // renaming: a/b/c -> a/b/c2
	}


	auto src_last_folder = get_last_list_item(src);
	auto dest_last_folder = get_last_list_item(dest);
	bool dest_subcatalog_exists = 
		! src_last_folder.empty() 
		&& ! dest_last_folder.empty()
		&& 0 == lstrcmpi(src_last_folder.c_str(), dest_last_folder.c_str());

	auto src_full = nf::sc::CCatalog::generateInternalPath(src);
	auto dest_full = nf::sc::CCatalog::generateInternalPath(dest);

	if (! dest_subcatalog_exists && ! rename) {
		dest_full.push_back(FarCatalog::GetSpecFolderName(REG_SUB_CATALOGS));
		dest_full.push_back(src_last_folder);
	}

	return nf::PluginSettings::CopyKey(src_full, dest_full);
}


/// a/b/c -> "Catalogs/a/Catalogs/b/Catalogs/c"
std::list<tstring> nf::sc::CCatalog::generateInternalPath(std::list<tstring> const& src) {
	std::list<tstring> dest;
	auto const folder = FarCatalog::GetSpecFolderName(REG_SUB_CATALOGS);
	BOOST_FOREACH(auto const& a, src) {
		dest.push_back(folder);
		dest.push_back(a);
	}
	return dest;
}

size_t nf::sc::CCatalog::GetNumberSubcatalogs() const {
	return get_sequence<tstring>(_pFarCatalog.get(), REG_SUB_CATALOGS)->getItems().size();
}

size_t nf::sc::CCatalog::GetNumberShortcuts() const {
	return get_sequence<tstring>(_pFarCatalog.get(), REG_STATIC_KEYS)->getItems().size();
}

boost::shared_ptr<nf::SequenceValues> nf::sc::CCatalog::GetSequenceShortcuts(bool bTemporary) {
	return get_sequence<nf::titem_sequence_values>(_pFarCatalog.get(), bTemporary ? REG_TEMP_KEYS : REG_STATIC_KEYS);
}

boost::shared_ptr<nf::SequenceCatalogs> nf::sc::CCatalog::GetSequenceSubcatalogs() {
	return get_sequence<nf::titem_sequence_catalogs>(_pFarCatalog.get(), REG_SUB_CATALOGS);
}

bool nf::sc::CCatalog::SetShortcut(tstring const& name, tstring const& shValue, bool bTemporary) {
	return _pFarCatalog->setValue(name, shValue, bTemporary ? REG_TEMP_KEYS : REG_STATIC_KEYS);
}

bool nf::sc::CCatalog::DeleteShortcut(tstring const& Name, bool bTemporary) {
	return _pFarCatalog->deleteValue(Name, bTemporary ? REG_TEMP_KEYS : REG_STATIC_KEYS);
}

bool nf::sc::CCatalog::SetProperty(tstring const& propertyName, tstring const& propertyValue) {
	return _pFarCatalog->setValue(propertyName, propertyValue, REG_PROPERTIES);
}

bool nf::sc::CCatalog::DeleteProperty(tstring const& propertyName) {
	return _pFarCatalog->deleteValue(propertyName, REG_PROPERTIES);
}

bool nf::sc::CCatalog::GetProperty(tstring const& propertyName, tstring& destValue) {
	return _pFarCatalog->getValue(propertyName, destValue, REG_PROPERTIES);
}

boost::shared_ptr<nf::FarCatalog> nf::sc::CCatalog::getFarCatalog() const {
	return _pFarCatalog;
}
