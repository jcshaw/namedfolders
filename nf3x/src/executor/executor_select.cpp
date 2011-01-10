/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/


#include "StdAfx.h"
#include "executor_select.h"

#include "StdAfx.h"
#include <cassert>
#include <vector>
#include <list>
#include <shlwapi.h>

#include "executor.h"
#include "Kernel.h"
#include "stlcatalogs.h"
#include "menu2.h"
#include "menus_impl.h"
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

//����� ��� ����, ���������� ��� Value � LocalPath
//� ������� �� ������ ������ � DestListPaths
void nf::Selectors::GetPath(HANDLE hPlugin, tstring const &srcValue, tstring const &localPath0
							, nf::twhat_to_search_t whatToSearch
							, nf::tlist_strings &destListPaths) {
	//p ����� ��������� �����������
	//������� ��� ����������, ��������������� panel.value

	//���������� �������� ����������� �� ����� �������������� ������� �������� �������������
	bool ballow_short_syntax = (nf::CSettings::GetInstance().GetValue(nf::ST_ALLOW_ABBREVIATED_SYNTAX_FOR_DEEP_SEARCH) != 0);
	tstring local_path = Utils::SubstituteSearchMetachars(localPath0, ballow_short_syntax); 
	tstring svalue = Utils::SubstituteSearchMetachars(srcValue, ballow_short_syntax);

	nf::Search::CSearchEngine ssp(nf::WTS_DIRECTORIES, false);
	nf::Search::SearchMatched(svalue, ssp, nf::ASTERIX_MODE_AS_IS, destListPaths);

	if (! local_path.empty()) {	//��������� ��������� ���� ������������ ������ ��������� ����������	
		nf::tlist_strings list_paths;
		nf::Search::CSearchEngine ssp(whatToSearch, true);
		BOOST_FOREACH(tstring const& path, destListPaths) {
			nf::Search::SearchByPattern(local_path.c_str(), path, ssp
				, static_cast<tasterix_mode>(Utils::atoi(nf::CSettings::GetInstance().GetValue(nf::ST_ASTERIX_MODE)))
				, list_paths);
		}
		destListPaths.swap(list_paths);
	}
}

tpath_selection_result nf::Selectors::GetPath(HANDLE hPlugin, tstring const &srcPath, tstring const &localPath0
											  , tstring &destPath, nf::twhat_to_search_t whatToSearch) {
	//����� ��� ����, ���������� ��� Value � LocalPath
	//���� ����������� ������������ ������� ��������� ���� 
	//� ������� ��� � ResultPath
	nf::tlist_strings value_paths;
	GetPath(hPlugin, srcPath, localPath0, whatToSearch, value_paths);
	value_paths.sort();
	//��� ���� � \ � .. ����� ����� ��������� ������������� �������� ������ � ���� �� �����
	value_paths.unique();

	//���� ������ ���������
	if (value_paths.empty()) return nf::ID_PATH_NOT_FOUND;

	if (Menu::SelectPath(value_paths, destPath) == 0) {
		return nf::ID_MENU_CANCELED;
	} else {
		return nf::ID_PATH_SELECTED;
	}
};

//������� �������� ���������� ��������� �� ������ ���������
//����� �� ������ ����� ����������
bool nf::Selectors::GetShortcut(HANDLE hPlugin, nf::tparsed_command const &cmd, nf::tshortcut_info& DestSh) {
	//����������� "."
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
			: !list.empty();	//variant should be selected from menu

		if (bExpand) {
			int n = Menu::SelectShortcut(list, DestSh);
			if (n == -Menu::CMenuShortcuts::MS_COMMAND_DELETE) {	//������� �������� ���������..
				nf::Commands::DeleteShortcut(DestSh, false);
			} else if (n == -Menu::CMenuShortcuts::MS_COMMAND_EDIT) {
				nf::Commands::EditShortcut(hPlugin, DestSh);
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
	//������� ��� ���������� ���������� �� ������
	size_t nexact = Shell::SelectShortcuts(cmd.shortcut.c_str(), cmd.catalog.c_str(), destList);
	if (cmd.shortcut == L".") {	//����������� "."
		destList.push_back(nf::MakeShortcut(L"",  L".", false));
	}
	return !destList.empty();
}

//������� �������� ���������� ������� �� ������ ���������
bool nf::Selectors::GetCatalog(HANDLE hPlugin, nf::tparsed_command const &cmd, nf::tcatalog_info &destCatalog) {
	nf::tvector_strings list;

	if (cmd.catalog.empty()) {
		destCatalog = L"";
		return true;		//�������� ������� - �������� ������...
	}

	tstring catalog_name = cmd.catalog;
	Utils::RemoveTrailingCharsOnPlace(catalog_name, SLASH_CATS_CHAR);
	Shell::SelectCatalogs(catalog_name.c_str(), list);
	Utils::AddLeadingCharIfNotExistsOnPlace(catalog_name, SLASH_CATS);

	if (list.empty()) return false;	//��� ���������

	//���� ������ ���� ������������ ������ �������
	//�� ��������� ����� � ���� - ����� ���������� ������� ��������
	bool bExpand = list.size() > 1;	//��������� �������� �� ����..
	if (bExpand) {
		int n = Menu::SelectCatalog(list, destCatalog);
		return n > 0;
	} else {
		destCatalog = *list.begin();
		return true;
	}
}

bool nf::Selectors::FindBestDirectory(HANDLE hPlugin, nf::tshortcut_value_parsed const &p, tstring const& localPath, tstring &destDir) {
	//����� ��������� ����������; ���� ��������� ���������� ��� - ����� ���������
	//������� ������������ � � ��� ������, ���� ��������� ���� �� ������, � ���������� ��������� � ����������� - ����������
	//����� localPath �� ������
	if (::PathFileExists(p.value.c_str()) && localPath.empty())  {
		destDir = p.value;
		return true;
	}
	bool blocal_path_not_found = ::PathFileExists(p.value.c_str()) && !localPath.empty();

	assert(p.ValueType != nf::VAL_TYPE_PLUGIN_DIRECTORY);	//��������� ���� ������ ��������� ����� �������� ������� ������
	tstring title = p.value + (blocal_path_not_found  //���������� ���������� ���������� + ��������� ����
		? localPath							//�� ��������� ���� ����� �������� (�� �� ������)
		: L"");
	tstring dir = p.value;

	nf::tautobuffer_char buf(dir.size()+1);	
	lstrcpy(&buf[0], dir.c_str());

	while (! ::PathFileExists(&buf[0])) {
		if (! ::PathRemoveFileSpec(&buf[0])) return false; //��������� ���������� �� ���������..
	};

	if (! nf::Confirmations::AskToGoToNearest(hPlugin, title, &buf[0])) return false;
	dir = &buf[0];

	destDir.swap(dir);
	return true;
} 