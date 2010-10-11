/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
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

using namespace nf;
using namespace sc;

CCatalog::CCatalog() 
: m_key(HKEY_CURRENT_USER, get_far_reg_key()) 
{
}

//конструктор для каталога вложенного в текущий
CCatalog::CCatalog(tstring SubCatalog, CCatalog const *pParent, bool bCreateIfNotExists)
{
	m_CatalogPath = get_combined_path(SubCatalog.c_str(), pParent);
	tstring regkey = GetCatalogRegkey();
	if (bCreateIfNotExists) nf::CRegistry(HKEY_CURRENT_USER, regkey.c_str());
	m_key = basic_class(HKEY_CURRENT_USER, regkey, KEY_ALL_ACCESS);		
}

//копирующий конструктор
CCatalog::CCatalog(CCatalog const &catalog) 
: m_key(catalog.m_key)
, m_CatalogPath(catalog.m_CatalogPath) 
{
}

//конструктор для создания обертки вокруг итератора последовательности каталогов
CCatalog::CCatalog(sc::catalogs_sequence_item &c, CCatalog const *pParent) 
{
	m_CatalogPath = get_combined_path(c.GetName().c_str(), pParent);
	m_key = basic_class(HKEY_CURRENT_USER, GetCatalogRegkey(), KEY_ALL_ACCESS);		
}

tstring CCatalog::GetCatalogRegkey() const
{	//получить ключ реестра соответствующий каталогу
	//алгоритм: если каталог равер "/" или пустой то в качестве ключа принимаем ключ реестра плагина
	//иначе: (добавляем (если его нет) первым символом "/"), заменяем все "/" на "catalogs\", затем добавляем к ключу реестра плагина
	tstring key = m_CatalogPath;
	if ((key != SLASH_CATS) && (! key.empty()))
	{
		if (key[0] != SLASH_CATS_CHAR) key = SLASH_CATS_CHAR + key;

		//заменяем "/" на "catalogs\"
		key = Utils::ReplaceStringAll(key, SLASH_CATS, GetKeyName(REG_B_SUB_CATALOGS_B));
		
		if (key.size() > 1 && *key.begin() == SLASH_DIRS_CHAR) key.erase(key.begin());	//удаляем первый /
		return Utils::CombinePath(get_far_reg_key(), key, SLASH_DIRS);
	}
	return get_far_reg_key();
}

bool CCatalog::SetShortcut(tstring const& Name, tstring const& Value, bool bTemporary) {	
	nf::CRegistry r(m_key.get_key_handle(), GetKeyName(bTemporary ? REG_TEMP_KEYS : REG_STATIC_KEYS));
	r.SetValue(Name, Value);
	return true;
}
bool CCatalog::InsertSubcatalog(tstring const& name) {
	nf::CRegistry newkey(m_key.get_key_handle(), GetKeyName(REG_SUB_CATALOGS));
	nf::CRegistry hkey(newkey, name.c_str());
	return true;
}

bool CCatalog::DeleteShortcut(tstring const& name, bool bTemporary) {
	basic_class key(m_key.get_key_handle(), GetKeyName(bTemporary ? REG_TEMP_KEYS : REG_STATIC_KEYS), KEY_ALL_ACCESS);
	return ERROR_SUCCESS == ::RegDeleteValue(key.get_key_handle(), name.c_str());
}

bool CCatalog::DeleteSubcatalog(tstring const& Name) {
	tstring name = Name;
	if (*name.begin() == SLASH_CATS_CHAR) name.erase(name.begin());

	nf::CRegistry r(m_key.get_key_handle(), GetKeyName(REG_SUB_CATALOGS), true, false);
	return ERROR_SUCCESS == ::RegDeleteKey(r, name.c_str());
}

bool CCatalog::GetShortcutInfo(tstring const& name_ansi, bool bTemporary, tstring &value) {	
	//Если псевдоним начинается с префикса "." - значит это субдиректория в текущем каталоге
	if (name_ansi.size() && name_ansi[0] == '\\') {
		tstring subdirectory(name_ansi, 1, name_ansi.size()-1);
		value = Utils::CombinePath(CPanelInfoWrap(INVALID_HANDLE_VALUE).GetPanelCurDir(true), subdirectory, SLASH_DIRS);
	} else {
		tstring name_reg_format = name_ansi;
		tstring subkey = GetKeyName(bTemporary ? REG_TEMP_KEYS : REG_STATIC_KEYS);
		if (! nf::Registry::IsSubkeyExist(m_key.get_key_handle(), subkey.c_str())) return false;

		basic_class key(m_key.get_key_handle(), subkey.c_str(), KEY_ALL_ACCESS);
		DWORD size = 256;
		DWORD type;
		DWORD ErrorCode;
		nf::tautobuffer_byte buf(size*sizeof(wchar_t));
		do {
			buf.resize(size);
			ErrorCode = ::RegQueryValueEx(key.get_key_handle(), name_reg_format.c_str(), 0, &type, reinterpret_cast<BYTE*>(&buf[0]), &size);
		} while (ErrorCode == ERROR_MORE_DATA);

		if (ErrorCode != ERROR_SUCCESS) return false;
		if (type != REG_SZ) return false;
		if (size) value.assign(reinterpret_cast<wchar_t*>(&buf[0]), size-1);
	}
	return true;
}

tstring CCatalog::get_combined_path(wchar_t const* catalog, CCatalog const *parent)
{	//получить полный путь к каталогу относительно корневого пути
	tstring name = (parent) ? parent->CatalogPath() : L"";
	if (! name.empty()) name += tstring(SLASH_CATS);
	name += catalog;
	return name;
}