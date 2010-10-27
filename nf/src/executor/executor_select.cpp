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

namespace {
	tstring substitute_metachars(tstring const& srcPath) {
		tstring s = srcPath;
		s = Utils::ReplaceStringAll(s, L".*.", DEEP_REVERSE_SEARCH);
		s = Utils::ReplaceStringAll(s, L"\\*\\", DEEP_DIRECT_SEARCH);
		s = Utils::ReplaceStringAll(s, tstring(L"\\") + LEVEL_UP_TWO_POINTS, DEEP_UP_DIRECTORY);
		return Utils::ReplaceStringAll(s, LEVEL_UP_TWO_POINTS, DEEP_UP_DIRECTORY);
	}
}

//����� ��� ����, ���������� ��� Value � LocalPath
//� ������� �� ������ ������ � DestListPaths
void nf::Selectors::GetPath(HANDLE hPlugin
							, tstring const &srcValue
							, tstring const &localPath0
							, nf::twhat_to_search_t whatToSearch
							, std::list<tstring> &destListPaths) {
	//p ����� ��������� �����������
	//������� ��� ����������, ��������������� panel.value

	//���������� �������� ����������� �� ����� �������������� ������� �������� �������������
	tstring local_path = substitute_metachars(localPath0);
	tstring svalue = substitute_metachars(srcValue);

	//since b242 it's possible to use NF-metacharacters in shortucts values
	//try to detect if any metacharacters (NF's or FAR's) are used 
	bool bmetachars_are_most_probably_used = (Utils::RemoveTrailingChars(svalue, SLASH_DIRS_CHAR) != svalue) 
		|| nf::Parser::ContainsMetachars(svalue)
		|| ! PathFileExists(svalue.c_str());

	if (bmetachars_are_most_probably_used) { 
		nf::Search::CSearchFarPolice ssp(nf::WTS_DIRECTORIES); //nf::Search::CSearchSystemPolice ssp(false, false);
		nf::Search::SearchMatched(svalue, ssp, destListPaths);
	} else { 
		destListPaths.push_back(svalue);
	}
	if (! local_path.empty()) {	//��������� ��������� ���� ������������ ������ ��������� ����������	
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
	//����� ��� ����, ���������� ��� Value � LocalPath
	//���� ����������� ������������ ������� ��������� ���� 
	//� ������� ��� � ResultPath
	std::list<tstring> value_paths;
	GetPath(hPlugin, srcPath, localPath0, whatToSearch, value_paths);
	value_paths.sort();
	//��� ���� � \ � .. ����� ����� ��������� ������������� �������� ������ � ���� �� �����
	value_paths.unique();

	//���� ������ ���������
	if (! value_paths.empty()) return Menu::SelectPath(value_paths, destPath) != 0;
	return false;
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
			: ! list.empty();	//variant should be selected from menu

		if (bExpand) {
			int n = Menu::SelectShortcut(list, DestSh);
			if (n == -1) {	//������� �������� ���������..
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
	//������� ��� ���������� ���������� �� ������
	size_t nexact = Shell::SelectShortcuts(cmd.shortcut.c_str(), cmd.catalog.c_str(), destList);
	if (cmd.shortcut == L".") {	//����������� "."
		destList.push_back(nf::MakeShortcut(L"",  L".", false));
	}
	return ! destList.empty();
}

//������� �������� ���������� ������� �� ������ ���������
bool nf::Selectors::GetCatalog(HANDLE hPlugin, nf::tparsed_command const &cmd, nf::tcatalog_info &destCatalog) {
	nf::tcatalogs_list list;

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

bool nf::Selectors::FindBestDirectory(HANDLE hPlugin, nf::tshortcut_value_parsed const &p, tstring &destDir)
{	//����� ��������� ���������� 
	//���� ��������� ���������� ��� - ����� ���������
	assert(p.ValueType != nf::VAL_TYPE_PLUGIN_DIRECTORY);	//��������� ���� ������ ��������� ����� �������� ������� ������
	tstring dir = p.value;

	if (! ::PathFileExists(dir.c_str())) {	//find nearest directory
		nf::tautobuffer_char buf(dir.size()+1);	
		lstrcpy(&buf[0], dir.c_str());

		do {
			if (! ::PathRemoveFileSpec(&buf[0])) return false; //��������� ���������� �� ���������..
		} while (! ::PathFileExists(&buf[0]));

		if (! nf::Confirmations::AskToGoToNearest(hPlugin, dir, &buf[0])) return false;
		dir = &buf[0];
	}
	destDir.swap(dir);
	return true;
} 