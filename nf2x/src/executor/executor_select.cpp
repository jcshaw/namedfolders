/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
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
#include "codec_for_values.h"
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
							, tstring const &LocalPath0
							, nf::twhat_to_search_t WhatToSearch
							, std::list<tstring> &DestListPaths)
{
	//p может содержать метасимволы
	//находим все директории, удовлетворяющие panel.value

	//комбинации символов указывающие на поиск неограниченной глубины
	//заменяем спецсимволами
	tstring LocalPath = LocalPath0;
	LocalPath = Utils::ReplaceStringAll(LocalPath, _T(".*."), DEEP_REVERSE_SEARCH);
	LocalPath = Utils::ReplaceStringAll(LocalPath, _T("\\*\\"), DEEP_DIRECT_SEARCH);
	LocalPath = Utils::ReplaceStringAll(LocalPath, LEVEL_UP_TWO_POINTS, DEEP_UP_DIRECTORY);


	if (nf::Parser::IsContainsMetachars(Value))
	{	//при поиске начальной директории всегда ищем и директории 
		nf::SearchPathsPolices::CSearchFarPolice ssp(nf::WTS_DIRECTORIES);
		//		nf::SearchPathsPolices::CSearchSystemPolice ssp(false, false);
		nf::CSearcherPaths sp(DestListPaths, ssp);
		sp.SearchMatched(Value);
	} else {
		DestListPaths.push_back(Value);
	}

	if (! LocalPath.empty())
	{	//учитываем локальный путь относительно каждой найденной директории	
		std::list<tstring> list_paths;
//		LocalPath = Parser::ConvertToMask(LocalPath);

		nf::SearchPathsPolices::CSearchSystemPolice ssp(WhatToSearch);
		nf::CSearcherPaths sp(list_paths, ssp);
		std::for_each(DestListPaths.begin(), DestListPaths.end()
			, boost::bind(&nf::CSearcherPaths::SearchByPattern, &sp, LocalPath.c_str(), _1));
		DestListPaths.swap(list_paths);
	}
}


bool nf::Selectors::GetPath(HANDLE hPlugin
							, tstring const &SrcPath
							, tstring const &LocalPath0
							, tstring &ResultPath
							, nf::twhat_to_search_t WhatToSearch)
{
	//найти все пути, подходящие для Value и LocalPath
	//дать возможность пользователю выбрать требуемый путь 
	//и вернуть его в ResultPath
	std::list<tstring> value_paths;
	GetPath(hPlugin, SrcPath, LocalPath0, WhatToSearch, value_paths);
	value_paths.sort();
	//при игре в \ и .. легко могут появиться множественные варианты одного и того же файла
	value_paths.unique();

	//меню выбора вариантов
	if (! value_paths.empty()) return Menu::SelectPath(value_paths, ResultPath) != 0;

	return false;
};


//выбрать наиболее подходящий псевдоним из списка вариантов
//выбор на основе имени псевдонима
bool nf::Selectors::GetShortcut(HANDLE hPlugin
								, nf::tparsed_command const &cmd
								, nf::tshortcut_info& DestSh)
{
	//псевдоалиас "."
	if (cmd.shortcut == _T("."))
	{
		DestSh.bIsTemporary = false;
		DestSh.catalog = _T("");
		DestSh.shortcut = _T(".");
		return true;
	}

	while (true)
	{
		nf::tshortcuts_list list;
		size_t nexact = Shell::SelectShortcuts(cmd.shortcut.c_str(), cmd.catalog.c_str(), list);

		//если найден один единственный точный вариант
		//но есть и другие варианты
		//то, в зависимости от настроек, 
		//переходим прямо к нему 
		//либо предлагаем выбрать другие варианты

		bool bExpand = (nexact == 1) 
			? CSettings::GetInstance().GetValue(nf::ST_ALWAYS_EXPAND_SHORTCUTS)
			: ! list.empty();	//требуется выбирать из меню..

		if (bExpand)
		{
			int n = Menu::SelectShortcut(list, DestSh);
			if (n == -1)
			{	//удалить выбраный псевдоним..
				nf::Commands::Deleter::DeleteShortcut(DestSh, false);
			} else return n > 0;
		} else {
			if (!list.empty()) 
			{
				DestSh = *list.begin();
				return true;			
			}
			return false;
		}
	} //while
}


bool nf::Selectors::GetAllShortcuts(HANDLE hPlugin
									, nf::tparsed_command const &cmd
									, nf::tshortcuts_list& DestList)
{	//выбрать все подходящие псевдонимы из списка
	size_t nexact = Shell::SelectShortcuts(cmd.shortcut.c_str(), cmd.catalog.c_str(), DestList);
	if (cmd.shortcut == _T("."))
	{	//псевдоалиас "."
		nf::tshortcut_info sh;
		sh.bIsTemporary = false;
		sh.catalog = _T("");
		sh.shortcut = _T(".");
		DestList.push_back(sh);
	}
	return ! DestList.empty();
}

//выбрать наиболее подходящий каталог из списка вариантов
bool nf::Selectors::GetCatalog(HANDLE hPlugin
							   , nf::tparsed_command const &cmd
							   , nf::tcatalog_info &DestCatalog)
{
	nf::tcatalogs_list list;

	if (cmd.catalog.empty()){
		DestCatalog = _T("");
		return true;		//корневой каталог - выбирать нечего...
	}

	tstring catalog_name = cmd.catalog;
	Utils::RemoveTrailingChars(catalog_name, SLASH_CATS_CHAR);
	Shell::SelectCatalogs(catalog_name.c_str(), list);
	Utils::add_leading_char_if_not_exists(catalog_name, SLASH_CATS);

	if (list.empty()) return false;	//нет вариантов

	//если найден один единственный точный вариант
	//то переходим прямо к нему - иначе предлагаем выбрать варианты
	bool bExpand = list.size() > 1;	//требуется выбирать из меню..
	if (bExpand)
	{
		int n = Menu::SelectCatalog(list, DestCatalog);
		return n > 0;
	} else {
		DestCatalog = *list.begin();
		return true;
	}
}


bool nf::Selectors::FindBestDirectory(HANDLE hPlugin
									 , nf::tshortcut_value_parsed const &p
									 , tstring &DestDir)
{	//найти наилучшую директории 
	//если требуемой директории нет - найти ближайшую
	assert(p.ValueType != nf::VAL_TYPE_PLUGIN_DIRECTORY);	//остальные типы должны открывать через эмуляцию нажатия клавиш
	tstring dir_oem = Utils::GetInternalToOem(p.value);

	if (! ::PathFileExists(dir_oem.c_str()))
	{	//находим ближайшую директорию
		nf::tautobuffer_char buf(dir_oem.size()+1);	
		lstrcpy(&buf[0], dir_oem.c_str());

		do {
			if (! ::PathRemoveFileSpec(&buf[0])) return false; //ближайшей директории не оказалось..
		} while (! ::PathFileExists(&buf[0]));

		//!todo предложение удалить
		//!подтвердить переход в ближайшую директорию
		if (! nf::Confirmations::AskToGoToNearest(hPlugin, dir_oem.c_str(), &buf[0])) return false;
		dir_oem = &buf[0];
	}

	DestDir = Utils::GetOemToInternal(dir_oem);
	return true;

} //find_best_dir