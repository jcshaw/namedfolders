/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/


#include "StdAfx.h"
#include "executor_select.h"

#include "StdAfx.h"
#include <cassert>
#include <vector>
#include <shlwapi.h>
#include <boost/bind.hpp>

#include "executor.h"
#include "Kernel.h"
#include "stlcatalogs.h"
#include "menu2.h"
#include "executor_addons.h"
#include "lang.h"
#include "shortcut_utils.h"
#include "header.h"
#include "confirmations.h"
#include "strings_utils.h"
#include "Parser.h"
#include "searcherpaths.h"
#include "select_variants.h"

extern struct PluginStartupInfo g_PluginInfo; 
extern struct FarStandardFunctions g_FSF;

using namespace nf;

//найти все пути, подходящие для Value и LocalPath
//и вернуть их полный список в DestListPaths
void nf::Selectors::GetPath(HANDLE hPlugin
							, tstring const &Value
							, tstring const &localPath0
							, nf::twhat_to_search_t whatToSearch
							, std::list<tstring> &destListPaths) {
	//p может содержать метасимволы
	//находим все директории, удовлетворяющие panel.value

	//комбинации символов указывающие на поиск неограниченной глубины заменяем спецсимволами
	tstring local_path = localPath0;
	local_path = Utils::ReplaceStringAll(local_path, L".*.", DEEP_REVERSE_SEARCH);
	local_path = Utils::ReplaceStringAll(local_path, L"\\*\\", DEEP_DIRECT_SEARCH);
	local_path = Utils::ReplaceStringAll(local_path, LEVEL_UP_TWO_POINTS, DEEP_UP_DIRECTORY);

	if (nf::Parser::ContainsMetachars(Value)) {	//при поиске начальной директории всегда ищем и директории 
		nf::Search::CSearchFarPolice ssp(nf::WTS_DIRECTORIES); //nf::Search::CSearchSystemPolice ssp(false, false);
		nf::Search::SearchMatched(Value, ssp, destListPaths);
	} else {
		destListPaths.push_back(Value);
	}
	if (! local_path.empty()) {	//учитываем локальный путь относительно каждой найденной директории	
		std::list<tstring> list_paths;
		nf::Search::CSearchSystemPolice ssp(whatToSearch);
		BOOST_FOREACH(tstring const& path, destListPaths) {
			nf::Search::SearchByPattern(local_path.c_str(), path, ssp, list_paths);
		}
		destListPaths.swap(list_paths);
	}
}

bool nf::Selectors::GetPath(HANDLE hPlugin, tstring const &srcPath, tstring const &localPath0, tstring &destPath
							, nf::twhat_to_search_t whatToSearch)
{
	//найти все пути, подходящие для Value и LocalPath
	//дать возможность пользователю выбрать требуемый путь 
	//и вернуть его в ResultPath
	std::list<tstring> value_paths;
	GetPath(hPlugin, srcPath, localPath0, whatToSearch, value_paths);
	value_paths.sort();
	//при игре в \ и .. легко могут появиться множественные варианты одного и того же файла
	value_paths.unique();

	//меню выбора вариантов
	if (! value_paths.empty()) return Menu::SelectPath(value_paths, destPath) != 0;
	return false;
};

//выбрать наиболее подходящий псевдоним из списка вариантов
//выбор на основе имени псевдонима
bool nf::Selectors::GetShortcut(HANDLE hPlugin, nf::tparsed_command const &cmd, nf::tshortcut_info& DestSh) {
	//псевдоалиас "."
	if (cmd.shortcut == L".") {
		DestSh = nf::MakeShortcut(L"", L".", false);
		return true;
	}
	while (true) {
		nf::tshortcuts_list list;
		size_t nexact = Shell::SelectShortcuts(cmd.shortcut.c_str(), cmd.catalog.c_str(), list);

		//if we have single exactly matched variant and there are some other (not exact) variants
		//then there are two possible scenario depending on settings: return exact variant or show menu
		bool bExpand = (nexact == 1) 
			? CSettings::GetInstance().GetValue(nf::ST_ALWAYS_EXPAND_SHORTCUTS) != 0
			: ! list.empty();	//variant should be selected from menu

		if (bExpand) {
			int n = Menu::SelectShortcut(list, DestSh);
			if (n == -1) {	//удалить выбраный псевдоним..
				nf::Commands::DeleteShortcut(DestSh, false);
			} else return n > 0;
		} else {
			if (!list.empty()) {
				DestSh = *list.begin();
				return true;			
			}
			return false;
		}
	} 
}

bool nf::Selectors::GetAllShortcuts(HANDLE hPlugin, nf::tparsed_command const &cmd, nf::tshortcuts_list& destList) {	
	//выбрать все подходящие псевдонимы из списка
	size_t nexact = Shell::SelectShortcuts(cmd.shortcut.c_str(), cmd.catalog.c_str(), destList);
	if (cmd.shortcut == L".") {	//псевдоалиас "."
		destList.push_back(nf::MakeShortcut(L"",  L".", false));
	}
	return ! destList.empty();
}

//выбрать наиболее подходящий каталог из списка вариантов
bool nf::Selectors::GetCatalog(HANDLE hPlugin, nf::tparsed_command const &cmd, nf::tcatalog_info &destCatalog) {
	nf::tcatalogs_list list;

	if (cmd.catalog.empty()) {
		destCatalog = L"";
		return true;		//корневой каталог - выбирать нечего...
	}

	tstring catalog_name = cmd.catalog;
	Utils::RemoveTrailingCharsOnPlace(catalog_name, SLASH_CATS_CHAR);
	Shell::SelectCatalogs(catalog_name.c_str(), list);
	Utils::AddLeadingCharIfNotExistsOnPlace(catalog_name, SLASH_CATS);

	if (list.empty()) return false;	//нет вариантов

	//если найден один единственный точный вариант
	//то переходим прямо к нему - иначе предлагаем выбрать варианты
	bool bExpand = list.size() > 1;	//требуется выбирать из меню..
	if (bExpand) {
		int n = Menu::SelectCatalog(list, destCatalog);
		return n > 0;
	} else {
		destCatalog = *list.begin();
		return true;
	}
}

bool nf::Selectors::FindBestDirectory(HANDLE hPlugin, nf::tshortcut_value_parsed const &p, tstring &destDir)
{	//найти наилучшую директории 
	//если требуемой директории нет - найти ближайшую
	assert(p.ValueType != nf::VAL_TYPE_PLUGIN_DIRECTORY);	//остальные типы должны открывать через эмуляцию нажатия клавиш
	tstring dir = p.value;

	if (! ::PathFileExists(dir.c_str())) {	//find nearest directory
		nf::tautobuffer_char buf(dir.size()+1);	
		lstrcpy(&buf[0], dir.c_str());

		do {
			if (! ::PathRemoveFileSpec(&buf[0])) return false; //ближайшей директории не оказалось..
		} while (! ::PathFileExists(&buf[0]));

		if (! nf::Confirmations::AskToGoToNearest(hPlugin, dir, &buf[0])) return false;
		dir = &buf[0];
	}
	destDir.swap(dir);
	return true;
} 