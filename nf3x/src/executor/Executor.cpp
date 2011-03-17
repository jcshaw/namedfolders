/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include <cassert>
#include <vector>
#include <list>
#include <shlwapi.h>
#include <boost/foreach.hpp>

#include "PanelInfoWrap.h"
#include "executor.h"
#include "Kernel.h"
#include "stlcatalogs.h"
#include "executor_addons.h"
#include "executor_select.h"
#include "lang.h"
#include "shortcut_utils.h"
#include "confirmations.h"
#include "Parser.h"
#include "StartSoftShortcut.h"
#include "far_impl.h"
#include "DialogEditShortcut.h"

extern struct PluginStartupInfo g_PluginInfo; 
using namespace nf;

namespace {
	bool delete_shortcut(nf::tparsed_command &cmd, CPanelInfoWrap &plugin, bool bImplicit) {
		nf::tshortcut_info sh;
		if (bImplicit) { //search shortcuts for current NF-catalog, user should select required shortcut
			if (! nf::Selectors::GetShortcutByPathPattern(plugin
				, plugin.GetPanelCurDir(true)
				, cmd
				, sh)) return false;
		} else { //ask user to select shortcut
			if (! nf::Selectors::GetShortcut(plugin, cmd, sh)) return false;
		}
		//remove selected shortcut
		return DR_DELETE == nf::Commands::DeleteShortcut(sh, bImplicit);	
	}

	bool insert_shortcut(nf::tparsed_command &cmd, CPanelInfoWrap &plugin, bool bImplicit, bool bTemporary, bool bInsertBoth) {
		if (bImplicit) {
			cmd.shortcut = nf::Commands::get_implicit_name_and_value(bInsertBoth).first;
		}
		if (cmd.shortcut.empty()) return false;
		//select best matchec catalog

//!TODO: select exist catalog OR create new one
		nf::tcatalog_info cat;
		if (! nf::Selectors::GetCatalog(plugin, cmd, cat)) {
			if (Confirmations::AskForCreateCatalog(plugin, cmd.catalog)) {
				nf::Commands::AddCatalog(plugin, cmd.catalog);
				if (! nf::Selectors::GetCatalog(plugin, cmd, cat)) { //каталог не существует и создать его не удалось.. 
					return false; 
				}
			} else return false;
			//return false;
		}

		//add new shortcut to selected catalog
		if (bInsertBoth) {
 			return nf::Commands::AddShortcutForBothPanels(plugin, cat, cmd.shortcut, bTemporary, bImplicit);
		} else {
			return nf::Commands::AddShortcut(plugin, cat, cmd.shortcut, bTemporary, bImplicit);
		}
		return false;
	}

	nf::twhat_to_search_t get_what_to_search(tcommands_kinds srcKind) {
		switch (srcKind) {
		case nf::QK_SEARCH_FILE: return nf::WTS_FILES; 
		case nf::QK_SEARCH_DIRECTORIES_AND_FILES: return nf::WTS_DIRECTORIES_AND_FILES; 
		default: return nf::WTS_DIRECTORIES;
		};
	}

	bool open_shortcut( nf::tparsed_command &cmd, CPanelInfoWrap& plugin, nf::twhat_to_search_t whatToSearch) {
		nf::tshortcut_info sh;
		if (! cmd.local_directory.empty() && CSettings::GetInstance().GetValue(nf::ST_USE_SINGLE_MENU_MODE) != 0) {
			//find all matched shortcuts, find all paths form them 
			//give possibility to user select path (not shortcut) and open it
			nf::tshortcuts_list list_all_shortcuts;
			if (Selectors::GetAllShortcuts(plugin, cmd, list_all_shortcuts)) {
				return nf::Commands::OpenShortcut(plugin, list_all_shortcuts, cmd.local_directory, true, whatToSearch);
			}
		} else {
			//find single best fitted shortcut (user selects required shortcut from the list)
			//then find all paths for selected shortcut, give user possibility to select path, etc
			if (nf::Selectors::GetShortcut(plugin, cmd, sh)) {
				return nf::Commands::OpenShortcut(plugin, sh, cmd.local_directory, true, whatToSearch);
			} 
		} 
		return false;
	}

	bool open_by_path(CPanelInfoWrap &plugin, nf::tparsed_command &cmd) {
		nf::tshortcut_info sh; //find most fitted shortcut 
		if (! nf::Selectors::GetShortcutByPathPattern(plugin
			, plugin.GetPanelCurDir(true)
			, cmd
			, sh)) return false;
		//open directory that is corresponded to selected shortcut
		return nf::Commands::OpenShortcut(plugin, sh, cmd.local_directory);
	}

	bool delete_catalog(CPanelInfoWrap plugin, nf::tparsed_command & cmd) {
		nf::tcatalog_info cat;		
		if (! nf::Selectors::GetCatalog(plugin, cmd, cat)) return false;
		return DR_DELETE == nf::Commands::DeleteCatalog(cat);
	}

	inline bool insert_catalog(CPanelInfoWrap plugin, nf::tparsed_command & cmd) {
		return Commands::AddCatalog(plugin, cmd.catalog);
	}

	inline bool open_directory_directly(CPanelInfoWrap &plugin, nf::tparsed_command &cmd, nf::twhat_to_search_t whatToSearch) {	
		if (Utils::IsLastCharEqualTo(cmd.local_directory, L':')) { 
			cmd.local_directory += L"\\*"; //cd:z: must be automatically converted to cd:z:\*
		}
		tpair_strings kvp = Utils::DividePathFilename(cmd.local_directory, SLASH_DIRS_CHAR, true);
		return nf::Commands::OpenPath(plugin, kvp.first, kvp.second, whatToSearch);
	}

	inline bool open_network(CPanelInfoWrap &plugin, tstring const& networkPath) {
		tshortcut_value_parsed vp;
		vp.bValueEnabled = true;
		vp.ValueType = VAL_TYPE_NET_DIRECTORY;
		vp.value = networkPath;
		return ::OpenShortcutOnPanel(plugin, vp, L"", false, false, true
			, WTS_DIRECTORIES //!TODO: а если это файл?
			);
	}
}

bool nf::ExecuteCommand(nf::tparsed_command &cmd, bool bReadDataForDialogMode) {
	CPanelInfoWrap plugin(INVALID_HANDLE_VALUE);

	if (0 == lstrcmpi((cmd.prefix + L":").c_str(), PREFIX_ST)) {
		if (bReadDataForDialogMode) return false; //not supported in dialogs
		switch(cmd.kind) {
		case QK_OPEN_SHORTCUT:
		case QK_START_SOFT_SHORTCUT:
			return Start::OpenSoftShortcut(plugin, cmd);
		}
	}

	switch(cmd.kind) {
	case nf::QK_OPEN_DIRECTORY_DIRECTLY:
		return open_directory_directly(plugin, cmd, nf::WTS_DIRECTORIES);
	case nf::QK_OPEN_SHORTCUT: //cd:
		return open_shortcut(cmd, plugin, get_what_to_search(cmd.kind));
	case nf::QK_SEARCH_DIRECTORIES_AND_FILES:
	case nf::QK_SEARCH_FILE: 
		if (cmd.catalog.empty()) {
			if ((0 != Parser::GetNetworkPathPrefixLength(cmd.shortcut)) || ::PathFileExists(cmd.shortcut.c_str())) {
				cmd.local_directory = Utils::CombinePath(cmd.shortcut, cmd.local_directory, SLASH_DIRS);
				cmd.shortcut.clear();
				return open_directory_directly(plugin, cmd, get_what_to_search(cmd.kind));
			}
		}
		return open_shortcut(cmd, plugin, get_what_to_search(cmd.kind));
	case nf::QK_OPEN_BY_PATH:	//cd:~ 
		return open_by_path(plugin, cmd);
	case nf::QK_OPEN_ENVIRONMENT_VARIABLE: //cd:%
		return nf::Selectors::OpenEnvVar(plugin, cmd.shortcut, cmd.local_directory);
	case nf::QK_OPEN_NETWORK: 	/*cd:\\*/
		return open_network(plugin, L"\\" + cmd.local_directory);
	}

	if (bReadDataForDialogMode) return false; //it's not possible to use other commands from dialogs
	switch(cmd.kind) {
	case nf::QK_INSERT_SHORTCUT:	//cd::
	case nf::QK_INSERT_SHORTCUT_IMPLICIT: //cd::
	case nf::QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT: //cd:+
	case nf::QK_INSERT_SHORTCUT_TEMPORARY: //cd:+
	case nf::QK_INSERT_BOTH:				//cd:::
	case nf::QK_INSERT_BOTH_TEMPORARY:	//cd:+:
	case nf::QK_INSERT_BOTH_IMPLICIT:	//cd:::
	case nf::QK_INSERT_BOTH_TEMPORARY_IMPLICIT:	//cd:+:
		return insert_shortcut(cmd, plugin 
			, ((nf::QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT == cmd.kind)  //implicit
				|| (nf::QK_INSERT_SHORTCUT_IMPLICIT == cmd.kind)
				|| (nf::QK_INSERT_BOTH_IMPLICIT == cmd.kind) 
				|| (nf::QK_INSERT_BOTH_TEMPORARY_IMPLICIT == cmd.kind)
			)
			, ((nf::QK_INSERT_SHORTCUT_TEMPORARY == cmd.kind)   //temporary
				|| (nf::QK_INSERT_BOTH_TEMPORARY == cmd.kind)
				|| (nf::QK_INSERT_BOTH_TEMPORARY_IMPLICIT == cmd.kind)
				|| (nf::QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT == cmd.kind))
			, ((nf::QK_INSERT_BOTH == cmd.kind) 
				|| (nf::QK_INSERT_BOTH_IMPLICIT == cmd.kind) 
				|| (nf::QK_INSERT_BOTH_TEMPORARY == cmd.kind)
				|| (nf::QK_INSERT_BOTH_TEMPORARY_IMPLICIT == cmd.kind))
		);
	case nf::QK_DELETE_SHORTCUT_IMPLICIT: //cd:-
		return delete_shortcut(cmd, plugin, true);
	case nf::QK_DELETE_SHORTCUT:	//cd:-alias
		return delete_shortcut(cmd, plugin, false);
	case nf::QK_DELETE_CATALOG:	//cd:-
		return delete_catalog(plugin, cmd);
	case nf::QK_INSERT_CATALOG: //cd::catalog/ 
		//The command has double meaning:
		//1) create new catalog if catalog doesn't exist
		//2) create shortcut implicitly if catalog exists
		if (! nf::sc::CCatalog::IsCatalogExist(cmd.catalog)) {
			return insert_catalog(plugin, cmd);
		} else {
			return insert_shortcut(cmd, plugin, true, false, false);
		}
	case nf::QK_START_SOFT_SHORTCUT:
		return Start::OpenSoftShortcut(plugin, cmd);
	default:;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// open
inline void select_panel(CPanelInfoWrap &plugin, nf::tshortcut_value_parsed_pair& Values, bool bOpenOnActivePanel)
{
	//отображаем панели "наоборот", если пользователь это явно требует (вызвал псевдоним удерживая Ctrl)
	bool bReverseValues = ! bOpenOnActivePanel;
	if (Values.first.bValueEnabled && Values.second.bValueEnabled)
	{
		//если открываем два значения, то поведение должно быть таким:
		//директория указанная первой должна всегда открываться на левой панели
		//другая - на правой.
		//если мы находимся на правой панели, нужно открывать 
		//директории "наоборот"

		//на какой панели находимся: на левой или на правой?
		bool bIsPanelRight = (plugin.GetPanelInfo(true).PanelRect.left != 0); //панель правая?

		if (plugin.GetPanelInfo(true).PanelRect.left == plugin.GetPanelInfo(false).PanelRect.left)
		{	//если правая панель распахнута на весь экран, то коодинаты left у панелей совпадают

			//если распахнута только правая панель, а левая - нормальная, тогда
			//если у активной панели правая координата больше, то активна правая панель иначе левая 
			bIsPanelRight = (plugin.GetPanelInfo(true).PanelRect.right > plugin.GetPanelInfo(false).PanelRect.right);
			//если обе панели распахнуты, определить на какой панели мы находимся невозможно
			//так что полагаем, что это левая панель
		};

		if (bIsPanelRight) bReverseValues = ! bReverseValues;
	}
	if (bReverseValues) std::swap(Values.first, Values.second);
}

namespace {
	tstring get_shortcut_value_ex(CPanelInfoWrap &Plugin, nf::tshortcut_info const& SrcSh) {
		tstring dest_value;
		if (nf::Shell::GetShortcutValue(SrcSh, dest_value)) return dest_value;
		if (SrcSh.shortcut == L".") return Plugin.GetPanelCurDir(true); //pseudo-alias "."
		if (::PathFileExists(SrcSh.shortcut.c_str())) return SrcSh.shortcut; //pseudo-alias "full path"
		return L"";
	}

	void append_paths(HANDLE hPlugin
		, nf::tshortcut_info const& SrcSh
		, nf::tshortcut_value_parsed SrcParsed
		, tstring const &LocalPath0
		, nf::twhat_to_search_t WhatToSearch
		, tlist_pairs_strings &DestList) {
		tstring local_path = LocalPath0;		
		//::Utils::RemoveSingleLeadingCharOnPlace(local_path, SLASH_DIRS_CHAR);
		if (SrcParsed.bValueEnabled) {
			nf::tlist_strings list;
			switch (SrcParsed.ValueType) {
			case nf::VAL_ENVIRONMENT_VARIABLE: nf::Selectors::GetAllPathForEnvvar(hPlugin, SrcParsed.value, list); break;
			case nf::VAL_REGISTRY_KEY: 
				nf::Selectors::GetAllPathForRegKey(hPlugin, SrcParsed.value
					, local_path //!TODO: передать сюда только первый токен LocalPath, для этих директорий LocalPath учитывать без первого токена
					, list); break;
			case nf::VAL_DIRECT_PATH: list.push_back(SrcParsed.value); break;
			case nf::VAL_TYPE_NET_DIRECTORY:;	//! TODO: пока не поддерживается
			default: //find all possible directories for local path
				nf::Selectors::GetPath(hPlugin, SrcParsed.value
					, local_path
					, WhatToSearch, list);
			} 			
			BOOST_FOREACH(tstring const& path, list) {
				DestList.push_back(std::make_pair(SrcSh.shortcut, path));
			}
		}
	}
}

bool nf::Commands::OpenShortcut(HANDLE hPlugin
								, nf::tshortcut_info const& sh
								, tstring const &LocalPath
								, bool bOpenOnActivePanel
								, nf::twhat_to_search_t WhatToSearch)
{	//open shortcut directory on panel, or both shortcut directories on both panels
	CPanelInfoWrap plugin(hPlugin);
	tstring value = get_shortcut_value_ex(plugin, sh);

	nf::tshortcut_value_parsed_pair vp = nf::DecodeValues(value);
	select_panel(plugin, vp, bOpenOnActivePanel);

	bool bOpenBoth = vp.first.bValueEnabled && vp.second.bValueEnabled;
	if (vp.second.bValueEnabled) {
		::OpenShortcutOnPanel(hPlugin, vp.second, LocalPath, false, bOpenBoth, false, WhatToSearch);
	}
	if (vp.first.bValueEnabled) {
		::OpenShortcutOnPanel(hPlugin, vp.first, LocalPath, true, bOpenBoth, (hPlugin != INVALID_HANDLE_VALUE), WhatToSearch);
	}

	return true;
}

bool nf::Commands::OpenShortcut(HANDLE hPlugin
								, nf::tshortcuts_list const& SrcList
								, tstring const &LocalPath
								, bool bOpenOnActivePanel
								, nf::twhat_to_search_t WhatToSearch) {
//open one of possible shortcuts
//show list of possible paths with taking into account available local paths
	CPanelInfoWrap plugin(hPlugin);
	tlist_pairs_strings list_sh_paths; //values for each shortcut
	BOOST_FOREACH(nf::tshortcut_info const& kvp, SrcList) { 
		tstring value = get_shortcut_value_ex(plugin, kvp);
		if (! value.empty()) {
			nf::tshortcut_value_parsed_pair vp = nf::DecodeValues(value);	
		//если две директории - помещаем в список обе
			::append_paths(hPlugin, kvp, vp.first, LocalPath, WhatToSearch, list_sh_paths);
			::append_paths(hPlugin, kvp, vp.second, LocalPath, WhatToSearch, list_sh_paths);
		}
	}
//suggest user to select path
//!TODO: special handling for other plugin directories (i.e. FTP).
	return ::SelectAndOpenPathOnPanel(hPlugin, list_sh_paths, WhatToSearch, bOpenOnActivePanel);
}

bool nf::Commands::OpenPath(HANDLE hPlugin, tstring const& srcPath, tstring const& localPath, nf::twhat_to_search_t whatToSearch) {	//open specified directory on the active panel
	nf::tshortcut_value_parsed ap;		//active panel
	ap.bValueEnabled = true;
	ap.value = srcPath;
	ap.ValueType = nf::VAL_DIRECT_PATH;

	::OpenShortcutOnPanel(hPlugin, ap, localPath, true, false, true, whatToSearch);
	return true;
}

bool nf::Commands::OpenShortcutInExplorer(HANDLE hPlugin, nf::tshortcut_info const& sh, tstring const &path) {
	tstring value;
	Shell::GetShortcutValue(sh, value);
	nf::tshortcut_value_parsed_pair vp = nf::DecodeValues(value);
	if (vp.first.bValueEnabled) {
		nf::Commands::OpenPathInExplorer(vp.first.value);
	}
	if (vp.second.bValueEnabled) {
		nf::Commands::OpenPathInExplorer(vp.second.value);
	}
	return TRUE;
}

void nf::Commands::OpenPathInExplorer(tstring const& srcPath) {
	tstring path = tstring(L"explorer \"") + srcPath + L"\"";

	nf::tautobuffer_char sv(path.size() + 1);
	::lstrcpyW(&sv[0], path.c_str());

	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION pi;
	::CreateProcess(0, &sv[0], NULL, NULL, 0, 0, NULL, 0, &si, &pi);
}

bool nf::Commands::AddShortcut(HANDLE hPlugin, nf::tshortcut_info const &sh, tstring const& srcValue, bool bImplicit) {
	if (sh.shortcut.empty()) return false;

	//check if exactly same shortcut already exists 
	sc::CCatalog c(sh.catalog);
	if (bImplicit) if (! nf::Confirmations::AskForImplicitInsert(hPlugin, sh, srcValue)) return false;
	tstring v;
	if (c.GetShortcutInfo(sh.shortcut, sh.bIsTemporary, v)) {
		if (! nf::Confirmations::AskForOverride(hPlugin, sh, v)) return false;
	}
	return Shell::InsertShortcut(sh, srcValue, true) != 0;
}

bool nf::Commands::AddShortcut(HANDLE hPlugin, nf::tcatalog_info const &cat, tstring const& shName, bool bTemp, bool bImplicit) {
	if (shName.empty()) return false;	//name of shortcut is absent

	return nf::Commands::AddShortcut(hPlugin, nf::MakeShortcut(cat, shName, bTemp)
		, CPanelInfoWrap(hPlugin).GetPanelCurDir(true)
		, bImplicit);
}

bool nf::Commands::AddShortcutForBothPanels(HANDLE hPlugin, nf::tcatalog_info const &cat, tstring const& shName, bool bTemp, bool bImplicit) {
	if (shName.empty()) return false;	
	CPanelInfoWrap plugin(hPlugin);
	nf::tshortcut_info sh;
	return nf::Commands::AddShortcut(hPlugin, nf::MakeShortcut(cat, shName, bTemp)
		, nf::EncodeValues(plugin.GetPanelCurDir(true), plugin.GetPanelCurDir(false))
		, bImplicit);
}

bool nf::Commands::CheckIfCatalogIsEmpty(nf::tcatalog_info const& cat) {
	sc::CCatalog c(cat);
	return (0 == c.GetNumberSubcatalogs()) && (0 == c.GetNumberShortcuts());
}

tpair_strings nf::Commands::get_implicit_name_and_value(bool bBothPanels, HANDLE hPlugin, bool bGetDataFromActivePanel) {
	tpair_strings result;
	CPanelInfoWrap plugin(hPlugin);
	if (! bBothPanels) {
		result.second = plugin.GetPanelCurDir(bGetDataFromActivePanel);
	} else {
		result.second = nf::EncodeValues(result.second = plugin.GetPanelCurDir(bGetDataFromActivePanel)
			, result.second = plugin.GetPanelCurDir(! bGetDataFromActivePanel));
	}
	result.first = ::PathFindFileName(result.second.c_str());
	tstring &s = result.first;

	tstring dest;
	dest.reserve(s.size());
	tstring::const_iterator ps = s.begin();
	while (ps != s.end()) { //first char mustn't be command character - skip if it is one.
		switch(*ps) {
		case L'%':
//		case L'&':
		case L'+':
		case L'|':
		case L'-':
		case L'!':
			//nothing to do
			break;
		default:
			dest.push_back(*ps);
		};
		++ps;
		break;
	}

	//other characters shouldn't be & (FAR) and %, | (NF) 
	while (ps != s.end()) {
		switch(*ps) {
		case L'%':
//		case L'&':
		case L'|':
			dest.push_back(L'_');
			break;
		case L'\\':
		case L':':
		case L'/':
			//just skip
			break;
		default:
			dest.push_back(*ps);
		};
		++ps;
	}
	result.first.swap(dest);
	return result;
}

int nf::Commands::DeleteCatalogsAndShortcuts(nf::tshortcuts_list const& listSh
											 , nf::tcatalogs_list const& listCatalogs
											 , bool bImplicit)
{	//delete selected catalogs and shortcuts; ask confirmations from user (if they are turned on in settings)
	//return total count of deleted shortcuts and catalogs
	using namespace nf::Confirmations;
	int count_deleted = 0;	//total count of deleted catalogs and shortcuts

	bool bConfirmImplicit = CSettings::GetInstance().GetValue(nf::ST_CONFIRM_IMPLICIT_DELETION) != 0;	
	bool bConfirmNotEmpty = CSettings::GetInstance().GetValue(nf::ST_CONFIRM_DELETE_NOT_EMPTY_CATALOGS) != 0;
	bool bConfirmDelete = CSettings::GetInstance().GetValue(nf::ST_CONFIRM_DELETE) != 0 || (bConfirmImplicit && bImplicit); 
	bool bSeveral = (listSh.size() + listCatalogs.size()) > 1;

	//delete catalogs
	BOOST_FOREACH(nf::tcatalog_info const& t, listCatalogs) {
		if (bConfirmDelete) {
			switch (nf::Confirmations::AskForDelete(t, false, bSeveral)) {
			case R_DELETEALL: bConfirmDelete = false; break;
			case R_CANCEL: return count_deleted;
			case R_SKIP: continue;
			}
		}
		if (bConfirmNotEmpty && (! nf::Commands::CheckIfCatalogIsEmpty(t))) {	
			switch (nf::Confirmations::AskForDelete(t, true, bSeveral)) {
			case R_DELETEALL: bConfirmNotEmpty = false; break;
			case R_CANCEL: return count_deleted;
			case R_SKIP: continue;
			}
		}
		Shell::DeleteCatalog(t);
		++count_deleted;
	}

	//delete shortcuts
	BOOST_FOREACH(nf::tshortcut_info const& sh, listSh) {
		if (bConfirmDelete) {
			switch (nf::Confirmations::AskForDelete(sh, bSeveral)) {
			case R_DELETEALL: bConfirmDelete = false; break;
			case R_CANCEL: return count_deleted;
			case R_SKIP: continue;
			}
		}
		Shell::DeleteShortcut(sh);
		++count_deleted;
	}	
	return count_deleted;
}

bool nf::Commands::EditShortcut(HANDLE hPlugin, nf::tshortcut_info const &sh) {
	tstring value;
	Shell::GetShortcutValue(sh, value);
	nf::tshortcut_value_parsed_pair vp = nf::DecodeValues(value);

	nf::CDialogEditShortcut dlg(sh, vp.first.value, vp.second.value, false);
	if (dlg.ShowModal()) {
		nf::tshortcut_info const& sh2 = dlg.GetShortcutInfo();
		tstring value2 = nf::EncodeValues(dlg.GetValueForActivePanel(), dlg.GetValueForInActivePanel());
		return Shell::ModifyShortcut(sh, sh2, &value2);
	}
	return false;
}