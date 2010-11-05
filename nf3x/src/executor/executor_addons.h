/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include "Kernel.h"
#include "shortcut_utils.h"
#include <list>
#include "PanelInfoWrap.h"

namespace nf {
namespace Selectors {
//получить полный список директорий для заданной переменной среды/ключа реестра
	void GetAllPathForRegKey(HANDLE hPlugins
		, tstring const &RegKeyName //ключ реестра, содержащий переменные, содержащие требуемые пути
		, tstring const &VarName //название переменной в ключе реестра
		, nf::tlist_strings &DestListPaths);
	void GetAllPathForEnvvar(HANDLE hPlugins
		, tstring const &VarName //название переменной: может быть указано полностью %temp% или частично %t
		, nf::tlist_strings &DestListPaths);

//cd:%
	bool OpenEnvVar(HANDLE hPlugin, tstring const &VarName, tstring const &LocalPath);
	bool GetPathByEnvvarPattern(HANDLE hPlugins
		, tstring const &VarName //название переменной: может быть указано полностью %temp% или частично %t
		, tstring const &LocalPath
		, tstring &path	//выбранная пользователем директория из всех возможных директорий
	);
//value = $reg_key
	bool GetPathByRegKey(HANDLE hPlugins
		, tstring const &regkey_name //ключ реестра, содержащий переменные, содержащие требуемые пути
		, tstring LocalPath
		, tstring &path	//выбранная пользователем директория из всех возможных директорий
	);

//cd:~
	bool GetShortcutByPathPattern(HANDLE hPlugin
		, tstring const &current_path
		, nf::tparsed_command const &cmd
		, nf::tshortcut_info& sh
	);
}

} // nf