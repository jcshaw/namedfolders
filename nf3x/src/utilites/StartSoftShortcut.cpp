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
#include "menu_header.h"
#include "executor.h"
#include "panel_functions.h"
#include "PathsFinder.h"

#include "executor_addons.h"
#include "strings_utils.h"
#include "x64.h"

using namespace nf;
using namespace Start;

namespace {
	class buffer_wrapper {
		nf::Search::MaskMatcher m_MaskSh;
		nf::Search::MaskMatcher m_MaskCatalogs;
		tstring m_RootPath;
		nf::tshortcuts_list m_Data;
		int m_bTemporaryValue;	//�� �������� Temporary ��������� ����, � ������� ����������� ������	
	public:
		buffer_wrapper(tstring const& srcRootPath, nf::Search::MaskMatcher& mmSh, int bTemporaryValue) 
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
					//!TODO: ������� ������ 1 �� ������ ����������...
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
	buffer_wrapper h(srcRootPath, mmSh, bTemporaryValue);
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
	//�������� � ���� ���������� � ������ ����� ������� � ��������� ��� � Paths
	//�������, ��� �����, ������ ������������ �������� ���������
//		std::sort(ListShortcuts.begin(), ListShortcuts.end());
		tstring common_folder;	//����������, ����� ��� ���� ��������� ������
		while (extract_common_folder(ListShortcuts, common_folder)) {
			for (unsigned int i = 0; i < Paths.size(); ++i) 
				Paths[i] = Utils::CombinePath(Paths[i], common_folder, SLASH_DIRS);
		}
	}

	void get_soft_variants(HANDLE hPlugin, nf::tvector_strings &destPaths)
	{	//���������� ������ ������ �������� �� ���������� ����
		//�������� ���� � �������� StartMenu ��� ���� ������������� � ��� �������� ������������
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
			// � win9x ��� VMWare ������ �� �� ����������� ������� SHGetSpecialFolderPath  //!TODO: is win95 actual?
			// �� ������ ������ ������� ����� ���� ����� 			
			tstring path;
			nf::Selectors::GetPathByEnvvarPattern(CPanelInfoWrap(hPlugin), L"windir", L"", path);
			tstring path_rus = path + L"\\������� ����\\���������";
			tstring path_eng = path + L"\\Start menu\\Programs";

			destPaths.push_back(path_rus);
			destPaths.push_back(path_eng);
		}
	}

	void execute_selected_program64_under_w32(tstring & path, tstring const &params, bool bActivate) {
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
				HINSTANCE value = ShellExecuteW(0, NULL , dest_path.c_str(), params.c_str(), NULL, bActivate ? SW_SHOWNORMAL : SW_SHOWMINNOACTIVE); 
			}
		}
	}
	void execute_selected_program(tstring &path, tstring const &params, bool bActivate) {
		HINSTANCE value = ShellExecuteW(0, NULL , path.c_str(), params.c_str(), NULL, bActivate ? SW_SHOWNORMAL : SW_SHOWMINNOACTIVE); 
		if ((int)(intptr_t)value < 32) { 
			if (nf::x64::IsWow64()) { //workaround for #6
				execute_selected_program64_under_w32(path, params, bActivate);
			}
		}
	}

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
			g_FSF.CopyToClipboard(FCT_STREAM, CPanelInfoWrap(hPlugin).GetPanelCurDir(true).c_str()); //store current directory to clipboard
			break;
		case FH_PLUS:
			PluginPanelItem* ppi = nf::Panel::allocate_PluginPanelItem(hPlugin, FCTL_GETCURRENTPANELITEM, 0);
			BOOST_SCOPE_EXIT( (&ppi) ) {
				nf::Panel::deallocate_PluginPanelItem(ppi);
			} BOOST_SCOPE_EXIT_END;
			g_FSF.CopyToClipboard(FCT_STREAM, 
				Utils::CombinePath(CPanelInfoWrap(hPlugin).GetPanelCurDir(true), ppi->FileName, SLASH_DIRS).c_str()); //store current directory and file name to clipboard
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
		//������ ������ "a,b,c,"  � ������
		Utils::SplitStringByRegex(comma_list, dest, L",");
		return dest;
	}
}

namespace {
	tstring get_application_path(nf::tvector_strings const& srcPaths, nf::tshortcut_info const& sh) {
		assert(static_cast<unsigned int>(sh.bIsTemporary) < srcPaths.size());
		return L"\"" + Utils::CombinePath(tstring(srcPaths[static_cast<int>(sh.bIsTemporary)])
			, Utils::CombinePath(sh.catalog, sh.shortcut, SLASH_DIRS), SLASH_DIRS)+ L"\"";
	}

	bool make_action_in_background(nf::tvector_strings const& srcPaths, int breakCode, nf::Menu::tvariant_value selectedItem) {		
		nf::tshortcut_info const& sh = boost::get<nf::tshortcut_info>(selectedItem);

		//!TODO: ���������� ���������, ����� ������
		FarKey& fk = nf::Menu::CMenuApplications::GetTotalListBreakKeys()[breakCode];
		if (fk.ControlKeyState  == nf::Menu::CMenuApplications::OPEN_APPLICATION_IN_BACKGROUND.ControlKeyState && fk.VirtualKeyCode  == nf::Menu::CMenuApplications::OPEN_APPLICATION_IN_BACKGROUND.VirtualKeyCode) {
			OpenApplication(get_application_path(srcPaths, sh), L"", false);
			return true;
		} else if (fk.ControlKeyState  == nf::Menu::CMenuApplications::OPEN_PATH_IN_EXPLORER_IN_BACKGROUND.ControlKeyState && fk.VirtualKeyCode  == nf::Menu::CMenuApplications::OPEN_PATH_IN_EXPLORER_IN_BACKGROUND.VirtualKeyCode) {
			OpenApplicationCatalogInExplorer(get_application_path(srcPaths, sh), false);
			return true;
		}

		return false;
	}
}

bool Start::OpenSoftShortcut(HANDLE hPlugin, nf::tparsed_command const &cmd) {
	nf::tvector_strings paths;
	get_soft_variants(hPlugin, paths); //get all possible paths

	tstring shortcut_pattern = cmd.shortcut;
	tstring params = cmd.param;
	tadd_action add_action = prepare_add_action(hPlugin, shortcut_pattern, params); //cd: :soft, cd: +soft, etc

	while (true) {	//��� ������������ ������ ����������� ���������� ������ ������
		tlist_strings masks;
		Utils::SplitStringByRegex(Utils::TrimChar(shortcut_pattern, L' '), masks, L"\\s");
		nf::Search::MaskMatcher sh_mask(masks, get_exceptions(), ASTERIX_MODE_BOTH, false);

	//��������: ���������� ���� bTemporary ����� ��������� ������ path_XXX ����������� �������� �����
	//�������� ������ ���� �������, ��������������� ��������
		nf::tshortcuts_list list_shortcuts;
		for (unsigned int i = 0; i < paths.size(); ++i) 
			if (! paths[i].empty()) {
				Start::LoadShortcuts(paths[i], sh_mask, i, list_shortcuts);
			}
	//�������� �������������� ������� ������ 
		try_to_minimize(paths, list_shortcuts);

		nf::tshortcut_info sh;
		nf::Menu::tbackground_action_maker bkg = boost::bind(&make_action_in_background, boost::cref(paths), _1, _2);
		int nret = nf::Menu::SelectSoft(list_shortcuts, sh, &bkg);		
		if (! nret) return false;

		tstring path = get_application_path(paths, sh);

		if (nret > 0) {	
			OpenApplication(path, params, true);
		} else {
			switch (-nret) {
			case Menu::CMenuApplications::CMD_OPEN_PATH_IN_EXPLORER:
				OpenApplicationCatalogInExplorer(path, true);
				break;
			case Menu::CMenuApplications::CMD_OPEN_PATH_IN_FAR:
				OpenApplicationPathInFAR(path);
				break;
			case Menu::CMenuApplications::CMD_SWITCH_IGNORE_MODE_ONOFF: 
				continue;
			default: return false;
			}; //switch			
		}		
		make_add_action(hPlugin, add_action);
		return true;
	}; //while

	return false;
}

bool Start::GetShortcutProgramPath(tstring const& PathToShortcut, tstring &destPath, bool bRemoveFileName) {
	//���� ����� ���� � �������� � Load �� ���������. ������� ������� ���� ��� ����
	//!TODO: ��������������� �������������� �������

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

namespace {
	struct tthread_param { //data for launch app / open app catalog in separate thread 
		tstring Path;
		tstring Params;
		int actionKind;
		bool activateWindow;
		tthread_param(int action_kind, tstring path, tstring params, bool activate_window) {
			this->Path = path;
			this->Params = params;
			this->actionKind = action_kind;
			this->activateWindow = activate_window;
		}
	};
	DWORD WINAPI thread_proc_execute_app(__in  LPVOID lpParameter) {
		//launch app in separate thread to avoid FAR "hanging" during process of application launching
		tthread_param* pp = (tthread_param*)lpParameter;
		if (pp != NULL) {
			switch (pp->actionKind) {
			case 0:
				execute_selected_program(pp->Path, pp->Params, pp->activateWindow);
				break;
			case 1:
				tstring program_directory;
				if (GetShortcutProgramPath(pp->Path, program_directory, true)) {
					nf::Commands::OpenPathInExplorer(program_directory, pp->activateWindow);
				}
				break;
			}
			delete pp;
		}
		return 0;
	}
}

void Start::OpenApplication(tstring const& applicationPath, tstring const& launchParams, bool bActivate) {
	tthread_param* p_path_param = new tthread_param(0, applicationPath, launchParams, bActivate);
	::CreateThread(NULL, 0, &thread_proc_execute_app, p_path_param, 0, NULL); //execute application in separate thread to avoid any delays in FAR

}
void Start::OpenApplicationCatalogInExplorer(tstring const& applicationPath, bool bActivate) {
	tthread_param* p_path_param = new tthread_param(1, applicationPath, L"", bActivate);
	::CreateThread(NULL, 0, &thread_proc_execute_app, p_path_param, 0, NULL);
}

void Start::OpenApplicationPathInFAR(tstring const& applicationPath) {
	tstring program_directory;
	if (GetShortcutProgramPath(applicationPath, program_directory, true)) {
		nf::Commands::OpenPath(CPanelInfoWrap(INVALID_HANDLE_VALUE), program_directory, L"", nf::WTS_DIRECTORIES);
	}
}