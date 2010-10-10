/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include <cassert>
#include <vector>
#include <shlwapi.h>

#include "PanelInfoWrap.h"
#include "executor.h"
#include "Kernel.h"
#include "stlcatalogs.h"
#include "executor_addons.h"
#include "executor_select.h"
#include "lang.h"
#include "codec_for_values.h"
#include "confirmations.h"
#include "Parser.h"
#include "StartSoftShortcut.h"
#include "far_impl.h"

extern struct PluginStartupInfo g_PluginInfo; 

bool nf::ExecuteCommand(nf::tparsed_command &cmd)
{
	CPanelInfoWrap plugin(INVALID_HANDLE_VALUE);
	nf::tshortcut_info sh;
	nf::tcatalog_info cat;

	const bool bSingleMenuMode_OpenLocalDirectory = CSettings::GetInstance().GetValue(nf::ST_USE_SINGLE_MENU_MODE) != 0;

	switch(cmd.kind) 
	{
	case nf::QK_OPEN_SHORTCUT: //cd:
	case nf::QK_SEARCH_DIRECTORIES_AND_FILES:
	case nf::QK_SEARCH_FILE:
		{
			//что ищем?
			nf::twhat_to_search_t what_to_search = nf::WTS_DIRECTORIES;
			switch (cmd.kind)
			{
			case nf::QK_SEARCH_FILE: what_to_search = nf::WTS_FILES; break;
			case nf::QK_SEARCH_DIRECTORIES_AND_FILES: what_to_search = nf::WTS_DIRECTORIES_AND_FILES; break;
			};

			if (! cmd.local_directory.empty() && bSingleMenuMode_OpenLocalDirectory) {
				//найти все подходящие псевдонимы, определить для них все пути 
				//потом дать пользователю выбрать путь (не псевдоним!) и открыть его
				nf::tshortcuts_list list_all_shortcuts;
				if (Selectors::GetAllShortcuts(plugin, cmd, list_all_shortcuts)) {
					return nf::Commands::OpenShortcut(plugin, list_all_shortcuts, cmd.local_directory, true, what_to_search);
				}
			} else {
				//найти один, наиболее подходящий псевдоним - дать пользователю выбрать среди псевдонимов нужный
				//затем для только выбранного псевдонима определить все подходящие пути, дать пользователю выбрать путь и т.д.
				if (nf::Selectors::GetShortcut(plugin, cmd, sh)) {	
				//открыть путь соответствующий псевдониму
					return nf::Commands::OpenShortcut(plugin, sh, cmd.local_directory, true, what_to_search);
				} else return false;
			} 
		} break;
	case nf::QK_OPEN_BY_PATH:	//cd:~ 
		//найти наиболее подходящий псевдоним
		if (! nf::Selectors::GetShortcutByPathPattern(plugin
			, plugin.GetPanelCurDir(true)
			, cmd
			, sh)) return false;
		//открыть путь соответствующий псевдониму
		return nf::Commands::OpenShortcut(plugin, sh, cmd.local_directory);
		break;
	case nf::QK_OPEN_ENVIRONMENT_VARIABLE: //cd:%
		return nf::Selectors::OpenEnvVar(plugin, cmd.shortcut, cmd.param);
	case nf::QK_OPEN_NETWORK:	/*cd:\\*/
		break;
	case nf::QK_INSERT_SHORTCUT:	//cd::
	case nf::QK_INSERT_BOTH:				//cd:::
	case nf::QK_INSERT_SHORTCUT_IMPLICIT: //cd::
	case nf::QK_INSERT_BOTH_TEMPORARY:	//cd:+:
	case nf::QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT: //cd:+
	case nf::QK_INSERT_SHORTCUT_TEMPORARY: //cd:+
		{
			bool bTemporary = ((nf::QK_INSERT_SHORTCUT_TEMPORARY == cmd.kind) 
							|| (nf::QK_INSERT_BOTH_TEMPORARY == cmd.kind)
							|| (nf::QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT == cmd.kind));
			bool bImplicit =  ((nf::QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT == cmd.kind) 
							|| (nf::QK_INSERT_SHORTCUT_IMPLICIT == cmd.kind));

			if (bImplicit) 
				cmd.shortcut = nf::Commands::get_implicit_name_and_value().first;

			if (cmd.shortcut.empty())
			{	//создать каталог
				Shell::InsertCatalog(cmd.catalog);

			} else {
				//найти наиболее подходящий каталог
				if (! nf::Selectors::GetCatalog(plugin, cmd, cat)) 
				{
					return false;
				}

				//добавить новый псевдоним в выбранный каталог
				if ((nf::QK_INSERT_BOTH == cmd.kind) || (nf::QK_INSERT_BOTH_TEMPORARY == cmd.kind))
					return nf::Commands::AddShortcutForBothPanels(plugin, cat, cmd.shortcut, bTemporary, bImplicit);
				else 
					return nf::Commands::AddShortcut(plugin, cat, cmd.shortcut, bTemporary, bImplicit);
			} //if
		} break;

	case nf::QK_DELETE_SHORTCUT_IMPLICIT:
	case nf::QK_DELETE_SHORTCUT:	//cD:-
		{
			bool bImplicit = nf::QK_DELETE_SHORTCUT_IMPLICIT == cmd.kind;
			
			if (bImplicit)
			{
				//найти псевдонимы для текущего каталога
				//пользователь должен выбрать наилучший..
				//cmd.shortcut = get_implicit_name();	
				//!TODO: проверить 
				if (! nf::Selectors::GetShortcutByPathPattern(plugin
					, plugin.GetPanelCurDir(true)
					, cmd
					, sh)) return false;
			
			} else 
			{
				//найти наиболее подходящий псевдоним
				if (! nf::Selectors::GetShortcut(plugin, cmd, sh)) 
					return false;
			}

			//удалить псевдоним
			return DR_DELETE == nf::Commands::Deleter::DeleteShortcut(sh, nf::QK_DELETE_SHORTCUT_IMPLICIT == cmd.kind);	
		} break;
	case nf::QK_DELETE_CATALOG:	//cD:-
		//найти наиболее подходящий каталог
		if (! nf::Selectors::GetCatalog(plugin, cmd, cat)) return false;
		//удалить каталог
		return DR_DELETE == nf::Commands::Deleter::DeleteCatalog(cat);
		break;
	case nf::QK_START_SOFT_SHORTCUT:
		return Start::OpenSoftShortcut(plugin, cmd);
		break;
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


//////////////////////////////////////////////////////////////////////////
// Open shortcut
namespace 
{
	tstring get_shortcut_value_ex(CPanelInfoWrap &Plugin, nf::tshortcut_info const& SrcSh)
	{
		tstring dest_value;
		if (nf::Shell::GetShortcutValue(SrcSh, dest_value)) return dest_value;
		
		//возможно это псевдоалиасы - cd:path (полный путь или точка)		
		if (SrcSh.shortcut == L".") return Plugin.GetPanelCurDir(true);
		else if (::PathFileExists(SrcSh.shortcut.c_str())) return SrcSh.shortcut;

		return L"";
	}

	void append_paths(HANDLE hPlugin
		, nf::tshortcut_info const& SrcSh
		, nf::tshortcut_value_parsed SrcParsed
		, tstring const &LocalPath0
		, nf::twhat_to_search_t WhatToSearch
		, std::list<std::pair<tstring, tstring> > &DestList)
	{
		tstring LocalPath = LocalPath0;
//!TODO: рефакторинг
//удаляем ТОЛЬКО ОДИН лидирующий слеш
		if (LocalPath.size() > 1 && LocalPath[0] == L'\\') LocalPath.erase(LocalPath.begin());
//		Utils::RemoveLeadingCharsOnPlace(LocalPath, _T('\\'));
		if (SrcParsed.bValueEnabled)
		{
			std::list<tstring> list;
			switch (SrcParsed.ValueType)
			{
			case nf::VAL_ENVIRONMENT_VARIABLE: nf::Selectors::GetAllPathForEnvvar(hPlugin, SrcParsed.value, list); break;
			case nf::VAL_REGISTRY_KEY: nf::Selectors::GetAllPathForRegKey(hPlugin, SrcParsed.value
										, LocalPath //передать сюда только первый токен LocalPath, для этих директорий LocalPath учитывать без первого токена
										, list); break;
			case nf::VAL_DIRECT_PATH: list.push_back(SrcParsed.value); break;
			case nf::VAL_TYPE_NET_DIRECTORY:;	//! TODO: пока не поддерживается
			default: //определяем всевозможные директории для локального пути
				//!TODO: LocalPath нужно учесть потом, для всех сразу
				nf::Selectors::GetPath(hPlugin, SrcParsed.value
					, LocalPath
					, WhatToSearch, list);
			} //switch
			//!TODO: учесть LocalPath
			for (std::list<tstring>::const_iterator p = list.begin(); p != list.end(); ++p)
			{
				DestList.push_back(std::make_pair(SrcSh.shortcut, *p));
			}
		}
	}
}

bool nf::Commands::OpenShortcut(HANDLE hPlugin
								, nf::tshortcut_info const& sh
								, tstring const &LocalPath
								, bool bOpenOnActivePanel
								, nf::twhat_to_search_t WhatToSearch)
{	//открыть один псевдоним
	CPanelInfoWrap plugin(hPlugin);
	tstring value = get_shortcut_value_ex(plugin, sh);

	nf::tshortcut_value_parsed_pair vp = nf::DecodeValues(value);
	select_panel(plugin, vp, bOpenOnActivePanel);

	bool bOpenBoth = vp.first.bValueEnabled && vp.second.bValueEnabled;
	if (vp.second.bValueEnabled) ::OpenShortcutOnPanel(hPlugin, vp.second, LocalPath, false, bOpenBoth, false, WhatToSearch);
	if (vp.first.bValueEnabled) ::OpenShortcutOnPanel(hPlugin, vp.first, LocalPath, true, bOpenBoth
		, (hPlugin != INVALID_HANDLE_VALUE), WhatToSearch);

	return true;
}

bool nf::Commands::OpenShortcut(HANDLE hPlugin
								, nf::tshortcuts_list const& SrcList
								, tstring const &LocalPath
								, bool bOpenOnActivePanel
								, nf::twhat_to_search_t WhatToSearch)
{	//открыть один из возможных псевдонимов
	//учесть локальные пути, сразу показать список всех возможных директорий (а не псевдонимов, затем директорий)

	CPanelInfoWrap plugin(hPlugin);
	//получаем значения для каждого из алиасов
	std::list<std::pair<tstring, tstring> > list_alias_path;
	for (nf::tshortcuts_list::const_iterator p = SrcList.begin(); p != SrcList.end(); ++p)
	{
		tstring value = get_shortcut_value_ex(plugin, *p);
		if (! value.empty())
		{
			nf::tshortcut_value_parsed_pair vp = nf::DecodeValues(value);	
		//если две директории - помещаем в список обе
			::append_paths(hPlugin, *p, vp.first, LocalPath, WhatToSearch, list_alias_path);
			::append_paths(hPlugin, *p, vp.second, LocalPath, WhatToSearch, list_alias_path);
		}
	}

	//предлагаем пользователю выбрать требуемый путь
	return ::SelectAndOpenPathOnPanel(hPlugin, list_alias_path, WhatToSearch);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

bool nf::Commands::OpenPath(HANDLE hPlugin, tstring const& path)
{	//открыть заданную директорию на активной панели
	nf::tshortcut_value_parsed ap;		//active panel
	ap.bValueEnabled = true;
	ap.value  = path;
	ap.ValueType = nf::VAL_DIRECT_PATH;

	::OpenShortcutOnPanel(hPlugin, ap, L"", true, false, true, nf::WTS_DIRECTORIES);
	return true;
}

bool nf::Commands::OpenShortcutInExplorer(HANDLE hPlugin
										  , nf::tshortcut_info const& sh
										  , tstring const &path)
{
	tstring value;
	Shell::GetShortcutValue(sh, value);
	nf::tshortcut_value_parsed_pair vp = nf::DecodeValues(value);
	if (vp.first.bValueEnabled) nf::Commands::OpenPathInExplorer(vp.first.value);
	if (vp.second.bValueEnabled) nf::Commands::OpenPathInExplorer(vp.second.value);

	return TRUE;
}

void nf::Commands::OpenPathInExplorer(tstring const& s)
{
	tstring path = tstring(L"explorer \"") + s + L"\"";

	nf::tautobuffer_char sv(path.size() + 1);
	::lstrcpy(&sv[0], path.c_str());

	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION pi;
	::CreateProcess(0, &sv[0], NULL, NULL, 0, 0, NULL, 0, &si, &pi);
}

//////////////////////////////////////////////////////////////////////////
// add
bool nf::Commands::AddCatalog(HANDLE hPlugin, nf::tcatalog_info const &cat)
{
	tstring parent;
	tstring c;
	Utils::DividePathFilename(cat, parent, c, SLASH_CATS_CHAR, false);
	Utils::RemoveLeadingCharsOnPlace(c, SLASH_CATS_CHAR);
	return Shell::InsertCatalog(c.c_str(), parent.c_str());
}

bool nf::Commands::AddShortcut(HANDLE hPlugin
							   , nf::tshortcut_info const &sh
							   ,tstring const& Value
							   , bool bImplicit)
{
	if (sh.shortcut.empty()) return false;	//name of shortcut is absent

	//проверить не существует ли уже точно такой псевдоним
	sc::CCatalog c(sh.catalog);
	tstring v;
	if (bImplicit)
		if (! nf::Confirmations::AskForImplicitInsert(hPlugin, sh, Value)) return false;
	if (c.GetShortcutInfo(sh.shortcut, sh.bIsTemporary, v))
	{
		if (! nf::Confirmations::AskForOverride(hPlugin, sh, v)) return false;
	}

	return Shell::InsertShortcut(sh, Value, true) != 0;
}

bool nf::Commands::AddShortcut(HANDLE hPlugin
							   , nf::tcatalog_info const &cat
							   , tstring const& sh
							   , bool bTemporary
							   , bool bImplicit)
{
	if (sh.empty()) return false;	//name of shortcut is absent

	nf::tshortcut_info info;
	info.catalog = cat;
	info.shortcut = sh;
	info.bIsTemporary = bTemporary;

	CPanelInfoWrap plugin(hPlugin);
	tstring cur_dir = plugin.GetPanelCurDir(true);

	return nf::Commands::AddShortcut(hPlugin, info, cur_dir.c_str(), bImplicit);
}

bool nf::Commands::AddShortcutForBothPanels(HANDLE hPlugin
											, nf::tcatalog_info const &cat
											, tstring const& sh
											, bool bTemporary
											, bool bImplicit)
{
	if (sh.empty()) return false;	//name of shortcut is absent

	CPanelInfoWrap plugin(hPlugin);
	tstring cur_dir1 = plugin.GetPanelCurDir(true);
	tstring cur_dir2 = plugin.GetPanelCurDir(false);

	tstring value = nf::EncodeValues(cur_dir1, cur_dir2);

	nf::tshortcut_info info;
	info.catalog = cat;
	info.shortcut = sh;
	info.bIsTemporary = bTemporary;

	return nf::Commands::AddShortcut(hPlugin, info, value.c_str(), bImplicit);
}

//////////////////////////////////////////////////////////////////////////
// misc
bool nf::Commands::IsCatalogIsEmpty(nf::tcatalog_info const& cat)
{
	sc::CCatalog c(cat);
	return (0 == c.GetNumberSubcatalogs()) && (0 == c.GetNumberShortcuts());
}

std::pair<tstring, tstring>
nf::Commands::get_implicit_name_and_value(HANDLE hPlugin, bool bGetDataFromActivePanel)
{
	std::pair<tstring, tstring> result;
	CPanelInfoWrap plugin(hPlugin);
	result.second = plugin.GetPanelCurDir(bGetDataFromActivePanel);
	result.first = ::PathFindFileName(result.second.c_str());
	tstring &s = result.first;

	tstring::iterator ps = s.begin();
	while (ps != s.end())
	{ //first char shouldn't be command characther - skip if it is one.
		switch(*ps) {
		case L'%':
//		case L'&':
		case L'+':
		case L'|':
		case L'-':
		case L'!':
			s.erase(ps);
			ps = s.begin();
			continue;
		};
		break;
	}

	//other characters should't be & (FAR) and %, | (NF) 
	while (ps != s.end()) {
		switch(*ps) {
		case L'%':
//		case L'&':
		case L'|':
			*ps = L'_';
		};
		++ps;
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////
// Deleter

nf::Commands::Deleter::Deleter(std::list<nf::tshortcut_info> const& Shortcuts
							   , std::list<nf::tcatalog_info> const& Catalogs
							   , bool bImplicit)
							   : m_Shortcuts(Shortcuts)
							   , m_Catalogs(Catalogs)
							   , m_bImplicit(bImplicit)
{

}

int nf::Commands::Deleter::DeleteShortcut(nf::tshortcut_info const&Shortcut, bool bImplicit)
{
	std::list<nf::tshortcut_info>  Shortcuts;
	std::list<nf::tcatalog_info>  Catalogs;
	Shortcuts.push_back(Shortcut);
	return Deleter(Shortcuts, Catalogs, bImplicit).Del();
}

int nf::Commands::Deleter::DeleteCatalog(nf::tcatalog_info const&Catalog, bool bImplicit)
{
	std::list<nf::tshortcut_info>  Shortcuts;
	std::list<nf::tcatalog_info>  Catalogs;
	Catalogs.push_back(Catalog);
	return Deleter(Shortcuts, Catalogs, bImplicit).Del();
}

int nf::Commands::Deleter::Del()
{	//удалить выбранные каталоги и псевдонимы
	//запросить необходимые подтверждения у пользователя (если они включены в настройках)
	//возвращаем количество удаленных каталогов и псевдонимов

	bool bConfirmDelete = CSettings::GetInstance().GetValue(nf::ST_CONFIRM_DELETE) != 0;
	bool bConfirmImplicit = CSettings::GetInstance().GetValue(nf::ST_CONFIRM_IMPLICIT_DELETION) != 0;	
	bool bConfirmNotEmpty = CSettings::GetInstance().GetValue(nf::ST_CONFIRM_DELETE_NOT_EMPTY_CATALOGS) != 0;

	bConfirmDelete = bConfirmDelete || (bConfirmImplicit && m_bImplicit); 

	bool bSeveral = (m_Shortcuts.size() + m_Catalogs.size()) > 1;

	using namespace nf::Confirmations;

	int count_deleted = 0;	//сколько всего удалено каталогов и псевдонимов

	//удаляем каталоги
	std::list<nf::tcatalog_info>::const_iterator pc = m_Catalogs.begin();
	while (pc != m_Catalogs.end())
	{
		bool bDelete = true;
		if (bConfirmDelete)
		{
			switch (nf::Confirmations::AskForDelete(*pc, false, bSeveral))
			{
			case R_DELETEALL: bConfirmDelete = false; break;
			case R_CANCEL: return count_deleted;
			case R_SKIP: bDelete = false;
			}
		}
		if (bConfirmNotEmpty)
		{
			if (! nf::Commands::IsCatalogIsEmpty(*pc))
			{	//каталог пуст
				switch (nf::Confirmations::AskForDelete(*pc, true, bSeveral))
				{
				case R_DELETEALL: bConfirmNotEmpty = false; break;
				case R_CANCEL: return count_deleted;
				case R_SKIP: bDelete = false;
				}
			}
		}
		if (bDelete)
		{
			Shell::DeleteCatalog(*pc);
			++count_deleted;
		}

		++pc;
	}

	std::list<nf::tshortcut_info>::const_iterator psh = m_Shortcuts.begin();
	while (psh != m_Shortcuts.end())
	{
		bool bDelete = true;
		if (bConfirmDelete)
		{
			switch (nf::Confirmations::AskForDelete(*psh, bSeveral))
			{
			case R_DELETEALL: bConfirmDelete = false; break;
			case R_CANCEL: return count_deleted;
			case R_SKIP: bDelete = false;
			}
		}
		if (bDelete)
		{
			Shell::DeleteShortcut(*psh);
			++count_deleted;
		}
		++psh;
	}
	
	return count_deleted;
}
