/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include "Kernel.h"
#include "codec_for_values.h"
#include "PanelInfoWrap.h"

namespace nf 
{
//выполнить команду (все команды кроме открытия панели)
	bool ExecuteCommand(nf::tparsed_command &cmd);

	namespace Commands {
	//открыть в FAR
		bool OpenShortcut(HANDLE hPlugin
			, nf::tshortcut_info const& sh
			, tstring const &path
			, bool bOpenOnActivePanel = true
			, nf::twhat_to_search_t WhatToSearch = nf::WTS_DIRECTORIES);	//!TODO: remove

		bool OpenShortcut(HANDLE hPlugin
			, nf::tshortcuts_list const& SrcList
			, tstring const &path
			, bool bOpenOnActivePanel = true
			, nf::twhat_to_search_t WhatToSearch = nf::WTS_DIRECTORIES);

		bool OpenShortcutInExplorer(HANDLE hPlugin
			, nf::tshortcut_info const& sh
			, tstring const &path);
		inline bool OpenFile(HANDLE hPlugin
			, nf::tshortcut_info const& sh
			, tstring const &path
			, bool bOpenOnActivePanel = true)
		{
			//при открытии директории проверяем - не файл ли это
			return OpenShortcut(hPlugin, sh, path, bOpenOnActivePanel, nf::WTS_FILES);
		}

	//открыть в Explorer
		bool OpenPath(HANDLE hPlugin, tstring const& path);
		void OpenPathInExplorer(tstring const& s);

	//создать новый псевдоним / каталог
		bool AddShortcut(HANDLE hPlugin, nf::tshortcut_info const &sh, tstring const& Value, bool bImplicit);
		bool AddShortcut(HANDLE hPlugin, nf::tcatalog_info const &cat, tstring const& sh, bool bTemporary, bool bImplicit);
		bool AddShortcutForBothPanels(HANDLE hPlugin, nf::tcatalog_info const &cat, tstring const& sh, bool bTemporary, bool bImplicit);
		bool AddCatalog(HANDLE hPlugin, nf::tcatalog_info const &cat);

		tpair_strings get_implicit_name_and_value(HANDLE hPlugin = INVALID_HANDLE_VALUE
			, bool bGetDataFromActivePanel = true	//с активной или неактивной панели брать данные
		);
		bool IsCatalogIsEmpty(nf::tcatalog_info const& cat);

	//класс для удаления одного/нескольких каталогов и/или псевдонимов
		int DeleteShortcut(nf::tshortcut_info const& Shortcut, bool bImplicit = false);
		int DeleteCatalog(nf::tcatalog_info const& Catalog, bool bImplicit = false);
		int DeleteCatalogsAndShortcuts(std::list<nf::tshortcut_info> const& listSh
			, std::list<nf::tcatalog_info> const& listCatalogs, bool bImplicit);
	} 
};