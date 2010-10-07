/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "kernel.h"
#include "Parser.h"
#include <cassert>
#include <memory>
#include "stlsoft_def.h"
#include "stlcatalogs.h"
#include "registry_remover.h"
#include "strings_utils.h"
#include "CommandPatterns.h"
#include "select_variants.h"

using namespace nf;

namespace {
	bool expand_catalog_path(tstring const &SrcCatalog
		, tstring TargetCatalog
		, tstring& ResultCatalog)
	{	
		//раскрыть ".." в TargetCatalog
		//"1/2/3/" ".." -> "1/2/../3" = "1/3"
		//"1/2/3/4/" "../../7" -> "1/2/3/../../7"="1/7"
		//"1/2/3/4/" "../../7" -> "1/2/3/../../7/4"="1/7/4"
		//"1/2/3/" "/5/6" -> "/5/6"
		//"1/2/3/" "/5/6/" -> /5/6/3"
		//"1/2/3/" "5/6" -> 1/2/5/6
		//"1/2/3/" "5/6/" -> 1/2/5/6/3
		const tstring ROOT(L"../");
		if (! TargetCatalog.size()) return false;

		bool bRelatedRoot = (*(TargetCatalog.begin()) == SLASH_CATS_CHAR);
		bool bRelatedCurrentCatalog = (*(TargetCatalog.end()-1) == SLASH_CATS_CHAR);
		Utils::AddTrailingCharIfNotExists(TargetCatalog, SLASH_CATS);
		if (TargetCatalog.size() >= ROOT.size()) {
			tstring::const_reverse_iterator p1 = TargetCatalog.rbegin();  //!TODO: итераторы изменены. Разобраться как работает эта функция и проверить правильность работы
			tstring::const_reverse_iterator p2 = ROOT.rbegin();
			while (p2 != ROOT.rend()) {
				if (*p1 == *p2) {
					++p1;
					++p2;
				} else break;
			}
			if (p2 == ROOT.rend()) bRelatedCurrentCatalog = true;
		}
		
		tstring src_path;
		tstring src_catalog_name;
		Utils::DividePathFilename(SrcCatalog, src_path, src_catalog_name, SLASH_CATS_CHAR, true);
		Utils::AddTrailingCharIfNotExists(src_path, SLASH_CATS);

		if (bRelatedRoot) {
			ResultCatalog.swap(TargetCatalog);
		} else {
			ResultCatalog.swap(src_path);
			ResultCatalog.append(TargetCatalog);
		} 
		if (bRelatedCurrentCatalog) {
			Utils::RemoveTrailingChars(ResultCatalog, SLASH_CATS_CHAR);
			ResultCatalog.append(src_catalog_name);
		}

		//сокращаем все ..
		Utils::AddLeadingCharIfNotExists(ResultCatalog, SLASH_CATS);
		tstring::size_type npos;
		while ((npos = ResultCatalog.find(ROOT)) != tstring::npos) {
			tstring::size_type npos2 = 
				ResultCatalog.find_last_of(SLASH_CATS_CHAR, npos-ROOT.size());
			if ((npos2 == tstring::npos)  || npos2 > npos) {
				ResultCatalog = L"";
				return true;	//дальше подниматься некуда
			}
			ResultCatalog.erase(npos2, npos-npos2+ROOT.size()-1);
		}

		Utils::RemoveTrailingChars(ResultCatalog, SLASH_CATS_CHAR);
		return true;
	}
} //namespace



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// --- выборка данных
size_t nf::Shell::SelectShortcuts(tstring shortcut_pattern
								, tstring catalog
								, tshortcuts_list& list)
{	//найти все ярлыки удовлетворяющие переданному шаблону
	if (!shortcut_pattern.size()) shortcut_pattern = L"*";
	Utils::RemoveTrailingChars(catalog, SLASH_CATS_CHAR);
	Shell::SelectShortcuts(catalog, shortcut_pattern, list, true);
	if (list.size()) return Shell::SortByRelevance(list, tstring(catalog), tstring(shortcut_pattern));
	return 0;
}

// --- модификация данных
bool nf::Shell::InsertShortcut(nf::tshortcut_info const&sh
							   , tstring value
							   , bool bOverride)
{	//добавить новый псевдоним в каталог
	tstring catalog_name = sh.catalog;
	Utils::RemoveLeadingChars(catalog_name, SLASH_CATS_CHAR);
	Utils::RemoveTrailingChars(catalog_name, SLASH_CATS_CHAR);

	sc::CCatalog c(catalog_name);
	if (! bOverride) {
		tstring buf;
		if (c.GetShortcutInfo(sh.shortcut, sh.bIsTemporary, buf)) return false; //такой псевдоним уже есть...
	}
	tstring oem_value = value;
	c.SetShortcut(sh.shortcut, oem_value, sh.bIsTemporary);
	return true;
}

bool nf::Shell::DeleteShortcut(tshortcut_info const&sh)
{	//удалить псевдоним
	sc::CCatalog c(sh.catalog);
	c.DeleteShortcut(sh.shortcut, sh.bIsTemporary);

	return true;
}

bool nf::Shell::ModifyShortcut(tshortcut_info const& from, tshortcut_info const& to, tstring* pnew_value)
{	//обновить свойства псевдонима
	tstring value;
	if (! pnew_value) 	{
		Shell::GetShortcutValue(from, value);
	} else {
		value = *pnew_value;
	}

	Shell::DeleteShortcut(from);
	Shell::InsertShortcut(to, value, true);

	return true;
}


bool nf::Shell::InsertCatalog(tstring catalog, wchar_t const* Parent)
{
	Utils::RemoveTrailingChars(catalog, SLASH_CATS_CHAR);

	sc::CCatalog c(Parent);
	return c.InsertSubcatalog(catalog);
}

bool nf::Shell::Private::remove_catalog(tstring const& SourceCatalog, tstring const* pTargetCatalog, bool bDeleteSource)
{
	//переместить каталог и все вложенные в него каталоги и псевдонимы
	//в другой каталог; если последний не указан 
	//удалить каталог и все вложенные в него каталоги и псевдонимы
	nf::registry_remover rr;

	tstring src_key = sc::CCatalog(SourceCatalog).GetCatalogRegkey();
	if (! pTargetCatalog) return rr.Erase(src_key);

	tstring target_catalog;
	if (! expand_catalog_path(SourceCatalog, *pTargetCatalog, target_catalog)) return false;

	tstring target_key = sc::CCatalog(target_catalog).GetCatalogRegkey();

	if (bDeleteSource) {
		return rr.Move(src_key, target_key);
	} else {
		return rr.Copy(src_key, target_key);
	}
}

bool nf::Shell::GetShortcutValue(tshortcut_info const& sh, tstring& value)
{
	sc::CCatalog c(sh.catalog);
	return c.GetShortcutInfo(sh.shortcut, sh.bIsTemporary, value);
}

bool nf::Shell::MoveShortcut(tshortcut_info const& sh, tstring const& new_shortcut_path, tshortcut_info &sh2)
{	//переместить псевдоним по указанному пути.
	//если путь закнчивается на / то весь путь - это каталог
	//если нет - то последнее имя - новое имя псевдонима
	//каталог может содержать ..
	
	sh2 = sh;
	if (*new_shortcut_path.rend() != SLASH_CATS_CHAR) {	
		std::pair<tstring, tstring> catalog_name = Utils::DivideString(new_shortcut_path, SLASH_CATS_CHAR);
		sh2.catalog.swap(catalog_name.first);
		Utils::RemoveLeadingChars(catalog_name.second, SLASH_CATS_CHAR);
		if (! catalog_name.second.empty()) sh2.shortcut.swap(catalog_name.second);
	}

	if (! sh2.catalog.empty() && ! expand_catalog_path(sh.catalog, sh2.catalog, sh2.catalog)) return false;
	return Shell::ModifyShortcut(sh, sh2, 0);
}

bool nf::Shell::CopyShortcut(tshortcut_info const& sh
							 , tstring const& new_shortcut_path
							 , tshortcut_info &sh2)
{	//скопировать псевдоним по указанному пути.
	//если путь закнчивается на / то весь путь - это каталог
	//если нет - то последнее имя - новое имя псевдонима
	//каталог может содержать ..

	sh2 = sh;
	if (*new_shortcut_path.rend() != SLASH_CATS_CHAR)
	{	
		tstring name;
		Utils::DividePathFilename(new_shortcut_path, sh2.catalog, name, SLASH_CATS_CHAR, false);
		Utils::RemoveLeadingChars(name, SLASH_CATS_CHAR);
		if (! name.empty()) sh2.shortcut.swap(name);
	}

	tstring value;
	if (! Shell::GetShortcutValue(sh, value)) return false;

	if (! sh2.catalog.empty() && ! expand_catalog_path(sh.catalog, sh2.catalog, sh2.catalog)) return false;
	return Shell::InsertShortcut(sh2, value, false);
}




