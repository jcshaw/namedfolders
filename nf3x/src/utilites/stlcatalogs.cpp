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

nf::sc::CCatalog::CCatalog() {

}

nf::sc::CCatalog::~CCatalog() {
	
}

nf::sc::CCatalog::CCatalog(tstring subCatalog, CCatalog const *pParent, bool bCreateIfNotExists) {
	m_CatalogPath = get_combined_path(subCatalog.c_str(), pParent);
	tstring regkey = GetCatalogRegkey();
	if (bCreateIfNotExists) {
		nf::CRegistry(HKEY_CURRENT_USER, regkey.c_str());
	}
	m_key = basic_class(HKEY_CURRENT_USER, regkey, KEY_ALL_ACCESS);		
}

//копирующий конструктор
nf::sc::CCatalog::CCatalog(CCatalog const &catalog) 
: m_key(catalog.m_key)
, m_CatalogPath(catalog.m_CatalogPath) 
{
}

//конструктор для создания обертки вокруг итератора последовательности каталогов
nf::sc::CCatalog::CCatalog(sc::catalogs_sequence_item &c, CCatalog const *pParent) {
	m_CatalogPath = get_combined_path(c.GetName().c_str(), pParent);
	m_key = basic_class(HKEY_CURRENT_USER, GetCatalogRegkey(), KEY_ALL_ACCESS);		
}

tstring nf::sc::CCatalog::get_catalog_regkey(tstring key) const {
	//получить ключ реестра соответствующий каталогу
	//алгоритм: если каталог равер "/" или пустой то в качестве ключа принимаем ключ реестра плагина
	//иначе: (добавляем (если его нет) первым символом "/"), заменяем все "/" на "catalogs\", затем добавляем к ключу реестра плагина
	if ((key != SLASH_CATS) && (! key.empty())) {
		if (key[0] != SLASH_CATS_CHAR) key = SLASH_CATS_CHAR + key;

		//заменяем "/" на "catalogs\"
		key = Utils::ReplaceStringAll(key, SLASH_CATS, GetKeyName(REG_B_SUB_CATALOGS_B));

		if (key.size() > 1 && *key.begin() == SLASH_DIRS_CHAR) key.erase(key.begin());	//удаляем первый /
		return Utils::CombinePath(get_far_reg_key(), key, SLASH_DIRS);
	}
	return get_far_reg_key();
}

bool nf::sc::CCatalog::IsSubcatalogExist(tstring const& subCatalog) {
	tstring catalog_path = get_combined_path(subCatalog.c_str(), this);
	tstring regkey = get_catalog_regkey(catalog_path);

	HKEY hkey;
	return ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER
		, regkey.c_str()
		, 0
		, KEY_READ
		, &hkey);
}

bool nf::sc::CCatalog::InsertSubcatalog(tstring const& name) {
	nf::CRegistry newkey(m_key.get_key_handle(), GetKeyName(REG_SUB_CATALOGS));
	nf::CRegistry hkey(newkey, name.c_str());
	return true;
}

bool nf::sc::CCatalog::DeleteSubcatalog(tstring const& Name) {
	tstring name = Name;
	if (*name.begin() == SLASH_CATS_CHAR) name.erase(name.begin());

	nf::CRegistry r(m_key.get_key_handle(), GetKeyName(REG_SUB_CATALOGS), true, false);
	return ERROR_SUCCESS == ::RegDeleteKey(r, name.c_str());
}

bool nf::sc::CCatalog::GetShortcutInfo(tstring const& shName, bool bTemporary, tstring &destValue) {	
	if (shName.size() && shName[0] == L'\\') { //!TODO: зачем эта проверка?
		tstring subdirectory(shName, 1, shName.size()-1);
		destValue = Utils::CombinePath(CPanelInfoWrap(INVALID_HANDLE_VALUE).GetPanelCurDir(true), subdirectory, SLASH_DIRS);
		return true;
	} else {
		return get_key_value(bTemporary ? REG_TEMP_KEYS : REG_STATIC_KEYS, shName, destValue);
	}
}

tstring nf::sc::CCatalog::get_combined_path(wchar_t const* catalog, CCatalog const *parent)
{	//получить полный путь к каталогу относительно корневого пути
	tstring name = (parent) ? parent->CatalogPath() : L"";
	if (! name.empty()) name += tstring(SLASH_CATS);
	name += catalog;
	return name;
}

wchar_t const* nf::sc::CCatalog::GetKeyName(tregs_enum Index) {
	static wchar_t const* regs[] = {L"keys", L"tempkeys", L"Catalogs", L"\\Catalogs\\", L"Properties"};
	return regs[static_cast<int>(Index)];
};

bool nf::sc::CCatalog::set_key(tregs_enum regKey, tstring const& srcName, tstring const& srcValue) {
	nf::CRegistry r(m_key.get_key_handle(), GetKeyName(regKey));
	r.SetValue(srcName.c_str(), srcValue.c_str());
	return true;
}

bool nf::sc::CCatalog::delete_key(tregs_enum regKey, tstring const& srcName) {
	basic_class key(m_key.get_key_handle(), GetKeyName(regKey), KEY_ALL_ACCESS);
	return ERROR_SUCCESS == ::RegDeleteValue(key.get_key_handle(), srcName.c_str());
}

bool nf::sc::CCatalog::get_key_value(tregs_enum regKey, tstring const& srcName, tstring& destValue) {
	tstring subkey = GetKeyName(regKey);
	if (! nf::Registry::IsSubkeyExist(m_key.get_key_handle(), subkey.c_str())) return false;

	basic_class key(m_key.get_key_handle(), subkey.c_str(), KEY_ALL_ACCESS);
	DWORD size = 256;
	DWORD type;
	DWORD ErrorCode;
	nf::tautobuffer_byte buf(size * sizeof(wchar_t));
	do {
		buf.resize(size);
		ErrorCode = ::RegQueryValueEx(key.get_key_handle(), srcName.c_str(), 0, &type, reinterpret_cast<BYTE*>(&buf[0]), &size);
	} while (ErrorCode == ERROR_MORE_DATA);

	if (ErrorCode != ERROR_SUCCESS) return false;
	if (type != REG_SZ) return false;
	if (size) {
		destValue.assign(reinterpret_cast<wchar_t*>(&buf[0]), static_cast<int>(size / sizeof(wchar_t)) - 1);
	}
	return true;
}
