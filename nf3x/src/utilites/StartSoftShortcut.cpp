/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#include "StdAfx.h"
#include "startsoftshortcut.h"
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/scope_exit.hpp>
#include <ShlObj.h>  
#include <shellapi.h>
#include <vector>
#include <Shlwapi.h>
#include <iterator>

#include "stlsoft_def.h"
#include "menu2.h"
#include "menus_impl.h"
#include "registry.h"
#include "executor.h"
#include "panel_functions.h"
#include "PathsFinder.h"

#include "executor_addons.h"
#include "strings_utils.h"

using namespace nf;
using namespace Start;

namespace {
	class helper {
		nf::Search::MaskMatcher m_MaskSh;
		nf::Search::MaskMatcher m_MaskCatalogs;
		tstring m_RootPath;
		nf::tshortcuts_list m_Data;
		int m_bTemporaryValue;	//по значению Temporary различаем пути, к которым принадлежат ярлыки	
	public:
		helper(tstring const& srcRootPath, nf::Search::MaskMatcher& mmSh, int bTemporaryValue) 
			: m_RootPath(srcRootPath)
			, m_MaskSh(mmSh)
			, m_MaskCatalogs(L"*", ASTERIX_MODE_BOTH)
			, m_bTemporaryValue(bTemporaryValue)
		{}
	public:
		void LoadSubCatalogs(tstring const& relatedPath) {
			tstring root_path = Utils::CombinePath(m_RootPath, relatedPath, SLASH_DIRS);
			WinSTL::findfile_sequence_t f(root_path.c_str(), L"*.*", WinSTL::findfile_sequence_t::directories);
			BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
				tstring dir_name = Utils::CombinePath(relatedPath, t.get_filename(), SLASH_DIRS);
				if (m_MaskCatalogs.MatchTo(dir_name)) LoadShortcuts(dir_name);
				LoadSubCatalogs(dir_name);
			}
		}
		void LoadShortcuts(tstring const& relatedPath) {
			tstring root_path = Utils::CombinePath(m_RootPath, relatedPath, SLASH_DIRS);
			WinSTL::findfile_sequence_t f(root_path.c_str(), L"*.*", WinSTL::findfile_sequence_t::files);
			BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
				nf::tshortcut_info sh;
				sh.shortcut = t.get_filename();
				if (m_MaskSh.MatchTo(sh.shortcut)) {
					sh.catalog = relatedPath;
					sh.bIsTemporary = static_cast<bool>(m_bTemporaryValue != 0);
					assert(static_cast<int>(sh.bIsTemporary) == m_bTemporaryValue);	
					//!TODO: индексы больше 1 не должны потеряться...
					m_Data.push_back(sh);
				}
			} 
		}
		nf::tshortcuts_list& GetResults() {
			return m_Data;
		}
	};
}

void Start::LoadShortcuts(tstring const& srcRootPath, nf::Search::MaskMatcher& mmSh, int bTemporaryValue, nf::tshortcuts_list& destData) {
	helper h(srcRootPath, mmSh, bTemporaryValue);
	h.LoadSubCatalogs(L"");
	/*if (h.IsDirMatchedToPattern(L"")) */h.LoadShortcuts(L"");
	BOOST_FOREACH(nf::tshortcut_info const& item, h.GetResults()) {
		destData.push_back(item);
	}
}

namespace {
	class minimizer {
		tstring m_Folder;
	public:
		minimizer(tstring const &Folder) : m_Folder(Folder) {}
		bool is_catalog_not_started_from(nf::tshortcut_info const &src) {
			std::size_t pos = src.catalog.find(m_Folder, 0);
			return (pos == tstring::npos) || (pos != 0);
		}
		nf::tshortcut_info remove_start_substring(nf::tshortcut_info const &src) {
			nf::tshortcut_info sh = src;
			assert(sh.catalog.size() >= m_Folder.size());
			sh.catalog.erase(0, m_Folder.size());
			return sh;
		}
	};

	bool extract_common_folder(nf::tshortcuts_list& ListShortcuts, tstring &CommonFolder) {
		nf::tshortcuts_list new_list;
		if (ListShortcuts.empty()) return false;

		tstring const &first_catalog_name = ListShortcuts.begin()->catalog;
		std::size_t pos = first_catalog_name.find_first_of(SLASH_DIRS_CHAR, 1);
		if (pos == tstring::npos) return false;

		CommonFolder = tstring(first_catalog_name, 0, pos);
		minimizer m(CommonFolder);
		if (std::find_if(ListShortcuts.begin(), ListShortcuts.end()
			, boost::bind(&minimizer::is_catalog_not_started_from, &m, _1)) == ListShortcuts.end())
		{
			std::insert_iterator<nf::tshortcuts_list> ins(new_list, new_list.begin());
			std::transform(ListShortcuts.begin(), ListShortcuts.end(), ins
				, boost::bind(&minimizer::remove_start_substring, &m, _1));

			new_list.swap(ListShortcuts);
			return true;
		}
		return false;
	}

	void try_to_minimize(nf::tvector_strings &Paths, nf::tshortcuts_list& ListShortcuts) {
	//выделяем у всех директорий в списке общий элемент и переносим его в Paths
	//снижаем, тем самым, размер отображаемых названий каталогов
//		std::sort(ListShortcuts.begin(), ListShortcuts.end());
		tstring common_folder;	//директория, общая для всех элементов списка
		while (extract_common_folder(ListShortcuts, common_folder)) {
			for (unsigned int i = 0; i < Paths.size(); ++i) 
				Paths[i] = Utils::CombinePath(Paths[i], common_folder, SLASH_DIRS);
		}
	}
}

namespace {
	void get_soft_variants(HANDLE hPlugin, nf::tvector_strings &destPaths)
	{	//составляем полный список программ из стартового меню
		//получаем путь к каталогу StartMenu для всех пользователей и для текущего пользователя
		wchar_t buffer[MAX_PATH];
		int ncount_dirs = 0;
		if (SHGetSpecialFolderPath(0, &buffer[0], CSIDL_STARTMENU, FALSE)) {
			destPaths.push_back(buffer);
			ncount_dirs++;
		}
		if (SHGetSpecialFolderPath(0, &buffer[0], CSIDL_COMMON_STARTMENU, FALSE)) {
			destPaths.push_back(buffer);
			ncount_dirs++;
		}
		if (!ncount_dirs)  {
			// В win9x под VMWare почему то не срабатывают функции SHGetSpecialFolderPath  //!TODO: is win95 actual?
			// на всякий случай обходим через одно место 			
			tstring path;
			nf::Selectors::GetPathByEnvvarPattern(CPanelInfoWrap(hPlugin), L"windir", L"", path);
			tstring path_rus = path + L"\\Главное меню\\Программы";
			tstring path_eng = path + L"\\Start menu\\Programs";

			destPaths.push_back(path_rus);
			destPaths.push_back(path_eng);
		}
	}
}

namespace {
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process;

	BOOL IsWow64()	{
		BOOL bIsWow64 = FALSE;

		//IsWow64Process is not available on all supported versions of Windows.
		//Use GetModuleHandle to get a handle to the DLL that contains the function
		//and GetProcAddress to get a pointer to the function if available.
		fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
		if (NULL != fnIsWow64Process) {
			if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64)) {
				//handle error
			}
		}
		return bIsWow64;
	}

	void execute_selected_program64_under_w32( tstring & path, tstring const &params ) {
		tstring dest_path;
		if (GetShortcutProgramPath(path, dest_path, false)) {
			DWORD buf_size = 2048;
			nf::tautobuffer_byte buffer(buf_size * sizeof(wchar_t));
			::ExpandEnvironmentStringsW(L"%ProgramFiles%", reinterpret_cast<wchar_t*>(&buffer[0]), buf_size);
			tstring pf32 = reinterpret_cast<wchar_t*>(&buffer[0]);
			::ExpandEnvironmentStringsW(L"%ProgramW6432%", reinterpret_cast<wchar_t*>(&buffer[0]), buf_size);
			tstring pf64 = reinterpret_cast<wchar_t*>(&buffer[0]);

			if (pf32 != pf64) {
				dest_path = Utils::ReplaceStringAll(dest_path, pf32, pf64);
				HINSTANCE value = ShellExecuteW(0, NULL , dest_path.c_str(), params.c_str(), NULL, SW_SHOWNORMAL); 
			}
		}
	}
	void execute_selected_program(tstring &path, tstring const &params) {
		HINSTANCE value = ShellExecuteW(0, NULL , path.c_str(), params.c_str(), NULL, SW_SHOWNORMAL); 
		if ((int)(intptr_t)value < 32) { 
			if (IsWow64()) { //workaround for #6
				execute_selected_program64_under_w32(path, params);
			}
		}
	}
}

namespace {
	typedef enum tadd_action {FH_NONE = 0
		, FH_COLON = 1
		, FH_PLUS = 2
	}; 
	tadd_action prepare_add_action(HANDLE hPlugin, tstring& pattern, tstring& params) {
		tadd_action action = FH_NONE;
		if (pattern.size()) {
			if (pattern[0] == L':') {
				action = FH_COLON;
			} else if (pattern[0] == L'+') {
				action = FH_PLUS;
			}
			if (action != FH_NONE) pattern.erase(0, 1);
		}
		return action;
	}
	void make_add_action(HANDLE hPlugin, tadd_action addAction) {
		switch (addAction) {
		case FH_COLON:
			g_FSF.CopyToClipboard(CPanelInfoWrap(hPlugin).GetPanelCurDir(true).c_str()); //store current directory to clipboard
			break;
		case FH_PLUS:
			PluginPanelItem* ppi = nf::Panel::allocate_PluginPanelItem(hPlugin, FCTL_GETCURRENTPANELITEM, 0);
			BOOST_SCOPE_EXIT( (&ppi) ) {
				nf::Panel::deallocate_PluginPanelItem(ppi);
			} BOOST_SCOPE_EXIT_END;
			g_FSF.CopyToClipboard(
				Utils::CombinePath(CPanelInfoWrap(hPlugin).GetPanelCurDir(true)
					, ppi->FindData.lpwszFileName, SLASH_DIRS).c_str()); //store current directory and file name to clipboard
			break;
		}
	}
	tlist_strings get_exceptions() { //generate list of exceptions that shouldn't be displayed
		tlist_strings dest;
		if (! nf::CSettings::GetInstance().GetValue(nf::ST_SELECT_SH_MENU_SHOWCATALOGS_MODE) 
			== 1	//don't ignore any masks
			) return dest; //empty list

		tstring comma_list = CSettings::GetInstance().GetValue(nf::ST_SOFT_MASKS_TO_IGNORE_COMMA_SEPARETED);
		if (! comma_list.empty()) comma_list += L",";
		//парсим строку "a,b,c,"  в список
		Utils::SplitStringByRegex(comma_list, dest, L",");
		return dest;
	}
}

bool Start::OpenSoftShortcut(HANDLE hPlugin, nf::tparsed_command const &cmd) {
	nf::tvector_strings paths;
	get_soft_variants(hPlugin, paths); //get all possible paths

	tstring shortcut_pattern = cmd.shortcut;
	tstring params = cmd.param;
	tadd_action add_action = prepare_add_action(hPlugin, shortcut_pattern, params); //cd: :soft, cd: +soft, etc

	while (true) {	//при переключении режима отображения генерируем список заново
		tlist_strings masks;
		Utils::SplitStringByRegex(Utils::TrimChar(shortcut_pattern, L' '), masks, L"\\s");
		nf::Search::MaskMatcher sh_mask(masks, get_exceptions(), ASTERIX_MODE_BOTH);

	//ВНИМАНИЕ: используем флаг bTemporary чтобы различать какому path_XXX принадлежит выбраный ярлык
	//получаем список всех ярлыков, соответствующих паттерну
		nf::tshortcuts_list list_shortcuts;
		for (unsigned int i = 0; i < paths.size(); ++i) 
			if (! paths[i].empty()) {
				Start::LoadShortcuts(paths[i], sh_mask, i, list_shortcuts);
			}
	//пытаемся минимизировать размеры списка 
		try_to_minimize(paths, list_shortcuts);

		nf::tshortcut_info sh;
		int nret = nf::Menu::SelectSoft(list_shortcuts, sh);		
		if (! nret) return false;

		assert(static_cast<unsigned int>(sh.bIsTemporary) < paths.size());
		tstring path = L"\"" + Utils::CombinePath(tstring(paths[static_cast<int>(sh.bIsTemporary)])
			, Utils::CombinePath(sh.catalog, sh.shortcut, SLASH_DIRS), SLASH_DIRS)+ L"\"";
		if (nret > 0) {	
			execute_selected_program(path, params); 
		} else {
			switch (-nret) {
			case Menu::CMenuApplications::OPEN_PATH_IN_EXPLORER:
				{	//открыть директорию, из которой запускается программа, в Far
					tstring program_directory;
					if (GetShortcutProgramPath(path, program_directory, true)) {
						nf::Commands::OpenPathInExplorer(program_directory);
					}
				} break;
			case Menu::CMenuApplications::OPEN_PATH_IN_FAR:
				{	//открыть директорию, из которой запускатся программа, в Explorer
					tstring program_directory;
					if (GetShortcutProgramPath(path, program_directory, true)) {
						CPanelInfoWrap wrap(INVALID_HANDLE_VALUE);
						nf::Commands::OpenPath(wrap, program_directory, L"", nf::WTS_DIRECTORIES);
					}
				} break;
			case Menu::CMenuApplications::SWITCH_IGNORE_MODE_ONOFF: continue;
			default: return false;
			}; //switch			
		}		
		make_add_action(hPlugin, add_action);
		return true;
	}; //while

	return false;
}

bool Start::GetShortcutProgramPath(tstring const& PathToShortcut, tstring &destPath, bool bRemoveFileName) {
	//путь может быть в кавычках и Load не сработает. Удаляем кавычки если они есть
	//!TODO: воспользоваться типизированным буфером

	nf::tautobuffer_char wbuffer(2 * (PathToShortcut.size()+1 > MAX_PATH ? PathToShortcut.size()+1 : MAX_PATH));	
	lstrcpy((reinterpret_cast<wchar_t*>(&wbuffer[0])), PathToShortcut.c_str());
	PathUnquoteSpaces((reinterpret_cast<wchar_t*>(&wbuffer[0])));

	::CoInitialize(0);
	BOOST_SCOPE_EXIT ( (&wbuffer) ) { //wbuffer is used because BOOST_SCOPE_EXIT doesn't support empty list params
		CoUninitialize();
	} BOOST_SCOPE_EXIT_END;
	
	IPersistFile* ppf = 0;
	IShellLink* psh = 0;

	HRESULT hr = ::CoCreateInstance(::CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER
									, IID_IPersistFile, reinterpret_cast<void**>(&ppf));
	if (FAILED(hr)) return false;
	BOOST_SCOPE_EXIT( (&ppf) ) {
		ppf->Release();
	} BOOST_SCOPE_EXIT_END;

	hr = ppf->Load((reinterpret_cast<wchar_t*>(&wbuffer[0])), STGM_READ);
	if (FAILED(hr)) return false;

	hr = ppf->QueryInterface(IID_IShellLink, reinterpret_cast<void**>(&psh));
	if (FAILED(hr)) return false;
	BOOST_SCOPE_EXIT( (&psh) ) {
		psh->Release();
	} BOOST_SCOPE_EXIT_END;

	hr = psh->GetPath(&wbuffer[0], MAX_PATH, NULL, SLGP_UNCPRIORITY);
	if (FAILED(hr)) return false;

	if (bRemoveFileName) ::PathRemoveFileSpec(&wbuffer[0]);

	destPath = &wbuffer[0];

	return true;
}