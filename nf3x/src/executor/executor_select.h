/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

#include "Kernel.h"
#include "shortcut_utils.h"
#include "PanelInfoWrap.h"


namespace nf {
//выбрать единственный путь из возможных вариантов
//варианты могу появиться, если указан 
//путь относительный именованной директории (LocalPath) 
//либо Value содержит метасимволы
namespace Selectors {
	//найти все пути, подходящие для Value и LocalPath
	//и вернуть их полный список в DestListPaths
	void GetPath(HANDLE hPlugin
		, tstring const &Value
		, tstring const &LocalPath
		, nf::twhat_to_search_t WhatToSearch
		, nf::tlist_strings &DestListPaths);

	//найти все пути, подходящие для Value и LocalPath
	//дать возможность пользователю выбрать требуемый путь 
	//и вернуть его в ResultPath
	tpath_selection_result GetPath(HANDLE hPlugin
		, tstring const &SrcPath
		, tstring const &LocalPath
		, tstring &ResultPath
		, nf::twhat_to_search_t WhatToSearch);

	//выбрать наиболее подходящий псевдоним из списка
	bool GetShortcut(HANDLE hPlugin
		, nf::tparsed_command const &cmd
		, nf::tshortcut_info& sh);
	//выбрать все подходящие псевдонимы из списка
	bool GetAllShortcuts(HANDLE hPlugin
		, nf::tparsed_command const &cmd
		, nf::tshortcuts_list& DestList);

	//выбрать наиболее подходящий каталог из списка
	bool GetCatalog(HANDLE hPlugin
		, nf::tparsed_command const &cmd
		, nf::tcatalog_info& c);

	//найти наилучшую директории 
	//если требуемой директории нет - найти ближайшую
	bool FindBestDirectory(HANDLE hPlugin
		, nf::tshortcut_value_parsed const &p
		, tstring const& localPath
		, tstring &dir);
} //Selectors
} //nf