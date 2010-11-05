/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "executor_addons.h"
#include "stlsoft_def.h"

#include <cassert>
#include <vector>
#include <list>
#include <functional>
#include <shlwapi.h>
#include <boost/foreach.hpp>

#include "header.h"
#include "strings_utils.h"

#include "executor.h"
#include "Kernel.h"
#include "stlcatalogs.h"
#include "menu2.h"
#include "lang.h"
#include "shortcut_utils.h"
#include "confirmations.h"
#include "Parser.h"
#include "select_variants.h"

using namespace nf;

namespace {
	void gen_list_envvars(tstring const& srcMask, tlist_pairs_strings &listPairs, bool bIncompleteName) {
		//получаем значения всех переменных среды, которые удовлетворяют шаблону varPattern
		//генерируем список всех директорий на которые ссылаются выбранные переменные среды
		if (_wenviron == NULL) {
			//see msdn: In a program that uses main, _wenviron is initially NULL because the environment is composed of multibyte-character strings. 
			//On the first call to _wgetenv or _wputenv, a corresponding wide-character string environment is created and is pointed to by _wenviron.
			_wgetenv(L"");
		}
		tstring full_mask = L"*" + srcMask;
		wchar_t **penv = _wenviron;
		while (*penv != NULL) {
			tstring senv = *penv;
			std::size_t pos = senv.find(L"=");
			if (pos != tstring::npos) {
				assert(pos+1 <= senv.size());
				tstring env_name(senv.c_str(), pos);
				tstring pe(senv.c_str(), pos+1, senv.size() - pos);

				bool bmask_in_name = nf::Parser::IsTokenMatchedToPattern(env_name, srcMask, false);
				if (bmask_in_name || bIncompleteName) {
					nf::tlist_strings list_tokens;
					Utils::SplitStringByRegex(pe, list_tokens, L";");
					BOOST_FOREACH(tstring const& token, list_tokens) {
						if (bmask_in_name || (bIncompleteName && nf::Parser::IsTokenMatchedToPattern(token, full_mask, false))) 
						{ //srcMask matched to name OR (if srcMask is incomplete) srcMask matched to directory
							if (::PathFileExists(token.c_str()) && ::PathIsDirectory(token.c_str())) {
								listPairs.push_back(std::make_pair(env_name, token));	
							}
						}
					}
				} 
			}
			++penv;
		}
	}
	void get_list_pairs_for_envvar(tstring const &varPattern, tlist_pairs_strings &listPairs, tstring &destLocalPath) {
		//получить список <имя переменная среды, путь>, удовлетворяющих шаблону varPattern
		//Если varPattern задана в виде %varPattern% или %varPattern то 
		//расширяем varPattern в переменную среды средствами NF

		tstring variable_name;

		//определяем имя переменной, локальный путь
		//а так же, задано ли имя переменной полностью, или это маска
		//если имя переменной задано без концевого % то это маска (это определяет функция ParseEnvVarPath)
		//если концевой % есть, но в имене есть метасимволы - это тоже маска, причем концевые звездочки к ней добавлять не надо
		//иначе это полное имя переменной
		bool bIncompleteVarPatternPattern  = ! nf::Parser::ParseEnvVarPath(varPattern, variable_name, destLocalPath);
		bool bMetachars = nf::Parser::ContainsMetachars(varPattern); //в имени есть метасимволы

		tstring mask = (bIncompleteVarPatternPattern && (! bMetachars))
			? variable_name + tstring(L"*") //"mask*" is more convenient in practice then "*mask*"
			: variable_name;

		gen_list_envvars(mask, listPairs, bIncompleteVarPatternPattern);
	};

	bool decode_reg_key(tstring const& path, HKEY &hkey, tstring &SubKeyPath) {
		//$HKEY_CURRENT_USER\Software ->  hkey=HKEY_CURRENT_USER, SubKeyPath=Software
		hkey = 0;
		size_t npos = min(static_cast<unsigned int>(path.find_first_of(SLASH_DIRS)), 
			static_cast<unsigned int>(path.find_first_of(SLASH_CATS)));
		if (npos == tstring::npos) return false;
		SubKeyPath.assign(path, npos+1, path.size() - npos);

		tstring hkey_path(path, 0, npos);
		assert(hkey_path.size());
		if (*hkey_path.begin() == CHAR_LEADING_VALUE_REGKEY) hkey_path.erase(hkey_path.begin());
		if (! lstrcmpi(hkey_path.c_str(), L"HKEY_CURRENT_USER")) hkey = HKEY_CURRENT_USER;
		else if (! lstrcmpi(hkey_path.c_str(), L"HKEY_LOCAL_MACHINE")) hkey = HKEY_LOCAL_MACHINE;

		return 0 != hkey;
	}

	void GetListPairsForRegKey(tstring const &RegKeyName
		, tstring const& subkeyPatternName
		, tlist_pairs_strings &DestListPaths) {	
	//получить список <ключ реестра, путь>, удовлетворяющих шаблону KeyPattern
		tstring subkey;
		HKEY hkey;
		if (! decode_reg_key(RegKeyName, hkey, subkey)) return;		//что-то пользователь не то передал...

		WinSTL::reg_key_t c(hkey, subkey.c_str());
		WinSTL::reg_value_sequence_t s(c);
		BOOST_FOREACH(WinSTL::reg_value_sequence_t::value_type const& value, s) {
			//путь заносим в виде: имя ключа, путь
			//если указан subkeyPatternName, оставляем только те переменные, пути которых 
			//удовлетворяют шаблону subkeyPatternName*
			tstring subkey_pattern_name = subkeyPatternName;
			if (! subkeyPatternName.empty()) subkey_pattern_name += L"*";
			if (subkeyPatternName.empty() || nf::Parser::IsTokenMatchedToPattern(value.name(), subkey_pattern_name, true)) {
				tstring path = value.value_expand_sz();
				if (::PathFileExists(path.c_str())) { //#18: only exist paths
					DestListPaths.push_back(std::make_pair(value.name(), path)); //CHAR_LEADING_VALUE_ENVVAR
				}
			}
		}
	}
}	//namespace

void nf::Selectors::GetAllPathForEnvvar(HANDLE hPlugins, tstring const &varName, nf::tlist_strings &destListPaths)
{	//получить полный список вариантов именованных директорий 
	tlist_pairs_strings list_var_paths;
	tstring additional_local_path; //!TODO: что делать с этим значением?
	::get_list_pairs_for_envvar(varName, list_var_paths, additional_local_path);

	//we don't need names of environment variables, we need only their paths
	BOOST_FOREACH(tpair_strings const& kvp, list_var_paths) {
		destListPaths.push_back(kvp.second);
	}

	//remove duplicates from the list
	destListPaths.sort(Utils::CmpStringEqualCI());
	destListPaths.unique(Utils::CmpStringEqualCI());
}

void nf::Selectors::GetAllPathForRegKey(HANDLE hPlugins, tstring const &regKeyName, tstring const &varName
										, nf::tlist_strings &destListPaths)
{	//получить полный список вариантов именованных директорий 	
	tlist_pairs_strings list_var_paths;
	::GetListPairsForRegKey(regKeyName, varName, list_var_paths);
	BOOST_FOREACH(tpair_strings const& kvp, list_var_paths) {
		destListPaths.push_back(kvp.second);
	}
}

bool nf::Selectors::OpenEnvVar(HANDLE hPlugin, tstring const &VarName, tstring const &LocalPath) {
	tstring result_path;
	if (! nf::Selectors::GetPathByEnvvarPattern(hPlugin, VarName, LocalPath, result_path)) return false;
	return nf::Commands::OpenPath(hPlugin, result_path, L"");
};

bool nf::Selectors::GetPathByEnvvarPattern(HANDLE hPlugin
										   , tstring const &varName
										   , tstring const &localPath
										   , tstring &destPath)
{
	tlist_pairs_strings list_var_paths;
	tstring additional_local_path;
	::get_list_pairs_for_envvar(varName, list_var_paths, additional_local_path);
	if (! localPath.empty()) {
		additional_local_path = additional_local_path.empty()
			? localPath
			: Utils::CombinePath(additional_local_path, localPath, L"\\");
		}

	if (list_var_paths.size() == 1) {
		destPath = (*list_var_paths.begin()).second;
	} else {
		tpair_strings result;
		if (! Menu::SelectEnvVar(list_var_paths, result)) return false;
		destPath = result.second;
	}
	if (! additional_local_path.empty()) destPath = Utils::CombinePath(destPath, additional_local_path, L"\\");
	return true;
};

bool nf::Selectors::GetPathByRegKey(HANDLE hPlugins
									, tstring const &regKey //ключ реестра, содержащий переменные, содержащие требуемые пути
									, tstring localPath
									, tstring &destPath	//выбранная пользователем директория из всех возможных директорий
									) {
	tlist_pairs_strings list_var_paths;
	::GetListPairsForRegKey(regKey
		, localPath	//!TODO: здесь только первое имя из localpath
		, list_var_paths);

	tpair_strings result;
	if (! Menu::SelectPathByRegKey(list_var_paths, result)) return false;
	destPath = result.second;
	return true;
}


//выбрать наиболее подходящий псевдоним из списка вариантов
//выбор на основе директории, на которую ссылается псевдоним...
bool nf::Selectors::GetShortcutByPathPattern(HANDLE hPlugin
											 , tstring const &curPath
											 , nf::tparsed_command const &cmd
											 , nf::tshortcut_info& sh)
{
	while (true) {
		nf::tshortcuts_list list;

		switch(cmd.kind) {
		case nf::QK_DELETE_SHORTCUT_IMPLICIT:
			Shell::SelectShortcutsByPath(L"", curPath, list, true);
			break;
		case nf::QK_OPEN_BY_PATH:
			Shell::SelectShortcutsByPath(cmd.catalog, cmd.shortcut, list, false);
			break;
		default:
			return false;
		} 
		int nCodeRet = Menu::SelectShortcut(list, sh);
		if (nCodeRet == -1) { //delete selected shortcut
			nf::Commands::DeleteShortcut(sh, false);
			continue;
		} else return nCodeRet != 0;
	}; 
}

