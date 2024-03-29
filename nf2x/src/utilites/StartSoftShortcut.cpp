/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/

#include "StdAfx.h"
#include "startsoftshortcut.h"
#include <boost/bind.hpp>
#include <ShlObj.h>  
#include <shellapi.h>
#include <comdef.h>
#include <vector>
#include <Shlwapi.h>
#include <iterator>
#include "stlsoft_def.h"

#include "menu2.h"
#include "menus_impl.h"
#include "ScopeGuard.h"
#include "registry.h"
#include "executor.h"

#include "executor_addons.h"
#include "strings_utils.h"

//#include <boost/lexical_cast.hpp>

using namespace nf;
using namespace Start;

namespace
{
	class exceptions_checker
	{
		std::list<tstring> m_ListEx;
	public:
		exceptions_checker()
		{
			tstring comma_list = Utils::GetOemToInternal(CSettings::GetInstance().GetValue(nf::ST_SOFT_MASKS_TO_IGNORE_COMMA_SEPARETED));
			if (! comma_list.empty()) comma_list += _T(",");
			//������ ������ "a,b,c,"  � ������
			Utils::SplitStringByRegex(comma_list, m_ListEx, _T(","));
		}
		bool IsShouldBeSkipped(nf::tshortcut_info const& sh)
		{
			return std::find_if(m_ListEx.begin(), m_ListEx.end()
				, boost::bind(&exceptions_checker::is_matched, this, sh.shortcut, _1)) != m_ListEx.end();
		}
	private:
		inline bool is_matched(tstring const& s, tstring const& StrException)
		{
			return nf::Parser::IsTokenMatchedToPattern(s, StrException, false);
		}
	};
}

CStartSoftShortcut::CStartSoftShortcut(TCHAR const* RootPathInt
									   , TCHAR const* Pattern
									   , TCHAR const* PatternDir
									   , nf::tshortcuts_list& Data
									   , int bTemporaryValue)
									   : m_RootPathOem(Utils::GetInternalToOem(RootPathInt))
									   , m_Pattern(Pattern)
									   , m_PatternDir(PatternDir)
									   , m_Data(Data)
									   , m_bTemporaryValue(bTemporaryValue)
{
	if (! nf::Parser::IsContainsMetachars(m_Pattern))
	{
		//���� ������������ ���� ����� - ������ �� �����
		//����� ������ "*������*"
		m_Pattern = _T("*") + m_Pattern + _T("*");
	}
	if (! nf::Parser::IsContainsMetachars(m_PatternDir))
	{
		//���� ������������ ���� ����� - ������ �� �����
		//����� ������ "*������*"
		m_PatternDir = _T("*") + m_PatternDir + _T("*");
	}
	load_data();
}

CStartSoftShortcut::~CStartSoftShortcut(void)
{
}

//////////////////////////////////////////////////////////////////////////
// private:
void CStartSoftShortcut::load_data()
{
	load_sub_catalogs(_T(""));
	if (is_dir_matched_to_pattern(_T(""))) load_shortcuts(_T(""));
}

void CStartSoftShortcut::load_sub_catalogs(tstring const& RelatedPathOem)
{
	tstring RootPath = Utils::CombinePath(m_RootPathOem, RelatedPathOem, SLASH_DIRS);
	WinSTL::findfile_sequence_t f(RootPath.c_str(), _T("*.*"), 
		WinSTL::findfile_sequence_t::directories);
	WinSTL::findfile_sequence_t::const_iterator p = f.begin();
	while (p != f.end())
	{
		tstring dir_name = Utils::CombinePath(RelatedPathOem, (*p).get_filename(), SLASH_DIRS);
		tstring dir_nameInt = Utils::GetOemToInternal(dir_name);
		if (is_dir_matched_to_pattern(dir_nameInt)) load_shortcuts(dir_name);
		load_sub_catalogs(dir_name);
		++p;
	} //while
}

void CStartSoftShortcut::load_shortcuts(tstring const& RelatedPathOem)
{
	tstring RootPathOem = Utils::CombinePath(m_RootPathOem, RelatedPathOem, SLASH_DIRS);
	WinSTL::findfile_sequence_t f(RootPathOem.c_str(), _T("*.*"), 
		WinSTL::findfile_sequence_t::files);
	WinSTL::findfile_sequence_t::const_iterator p = f.begin();
	while (p != f.end())
	{
		nf::tshortcut_info sh;
		sh.shortcut = Utils::GetOemToInternal((*p).get_filename());
		if (is_matched_to_pattern(sh.shortcut))
		{
			sh.catalog = Utils::GetOemToInternal(RelatedPathOem);
			sh.bIsTemporary = static_cast<bool>(m_bTemporaryValue != 0);
			assert(static_cast<int>(sh.bIsTemporary) == m_bTemporaryValue);	
				//!TODO: ������� ������ 1 �� ������ ����������...
			m_Data.push_back(sh);
		}
		++p;
	} //while
}

namespace
{
	class minimizer
	{
		tstring m_Folder;
	public:
		minimizer(tstring const &Folder) : m_Folder(Folder) {}

		bool is_catalog_not_started_from(nf::tshortcut_info const &src)
		{
			std::size_t pos = src.catalog.find(m_Folder, 0);
			return (pos == tstring::npos) || (pos != 0);
		}
		nf::tshortcut_info remove_start_substring(nf::tshortcut_info const &src)
		{
			nf::tshortcut_info sh = src;
			assert(sh.catalog.size() >= m_Folder.size());
			sh.catalog.erase(0, m_Folder.size());
			return sh;
		}
	};

	bool extract_common_folder(nf::tshortcuts_list& ListShortcuts, tstring &CommonFolder)
	{
		nf::tshortcuts_list new_list;
//		new_list.reserve(ListShortcuts.size());

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

	void try_to_minimize(std::vector<tstring> &Paths, nf::tshortcuts_list& ListShortcuts)
	{
	//�������� � ���� ���������� � ������ ����� ������� � ��������� ��� � Paths
	//�������, ��� �����, ������ ������������ �������� ���������

//		std::sort(ListShortcuts.begin(), ListShortcuts.end());
		tstring common_folder;	//����������, ����� ��� ���� ��������� ������
		while (extract_common_folder(ListShortcuts, common_folder))
		{
			for (unsigned int i = 0; i < Paths.size(); ++i) 
				Paths[i] += common_folder;
		}
	}
	void remove_exceptions(nf::tshortcuts_list& ListShortcuts)
	{
		if (! nf::CSettings::GetInstance().GetValue(nf::ST_SELECT_SH_MENU_SHOWCATALOGS_MODE) 
			== 1	//don't ignore any masks
		) return;

		exceptions_checker checker;
		ListShortcuts.erase(
			std::remove_if(ListShortcuts.begin(), ListShortcuts.end()
				, boost::bind(&exceptions_checker::IsShouldBeSkipped, &checker, _1))
			, ListShortcuts.end());
	}
}

namespace 
{
	void get_soft_variants(HANDLE hPlugin, std::vector<tstring> &Paths)
	{	//���������� ������ ������ �������� �� ���������� ����
		//�������� ���� � �������� StartMenu ��� ���� ������������� � ��� �������� ������������
		TCHAR buffer[MAX_PATH];
		int ncount_dirs = 0;
		if (SHGetSpecialFolderPath(0, &buffer[0], CSIDL_STARTMENU, FALSE))
		{
			Paths.push_back(buffer);
			ncount_dirs++;
		}

		if (SHGetSpecialFolderPath(0, &buffer[0], CSIDL_COMMON_STARTMENU, FALSE))
		{
			Paths.push_back(buffer);
			ncount_dirs++;
		}

		if (!ncount_dirs) 
		{
			// � win9x ��� VMWare ������ �� �� ����������� ������� SHGetSpecialFolderPath
			// �� ������ ������ ������� ����� ���� ����� 
			//!TODO
			tstring path;
			nf::Selectors::GetPathByEnvvarPattern(CPanelInfoWrap(hPlugin)
				, _T("windir")
				, _T("")
				, path);
			tstring path_rus = path + _T("\\������� ����\\���������");
			tstring path_eng = path + _T("\\Start menu\\Programs");

			Paths.push_back(path_rus);
			Paths.push_back(path_eng);
		}
	}
}

bool Start::OpenSoftShortcut(HANDLE hPlugin
							 , nf::tparsed_command const &cmd)
{
	std::vector<tstring> paths;
//�������� ������������ ����
	get_soft_variants(hPlugin, paths);

	tstring shortcut_pattern;	// = (cmd.local_directory.size()) ? cmd.shortcut : cmd.local_directory;
	tstring catalog_pattern;	// = _T("*");

	if (! cmd.local_directory.size())
	{
		shortcut_pattern = cmd.shortcut;
		catalog_pattern = _T("*");
	} else 
	{
		shortcut_pattern = cmd.local_directory;
		catalog_pattern = cmd.shortcut;
		Utils::RemoveLeadingChars(shortcut_pattern, SLASH_DIRS_CHAR);
	}
//��������� ������� :
//���� ����� � ���� cd: :soft �� ������������ �����, ����� 
//������ �� ������ ������ soft, �� ��� � ���������� ������� ���� � ����� ������
//(��������, ����� ��������� Word � ������ ��������� � ������� ���������� ����� ��������� ����)
	bool bsave_directory_to_clipboard = false;
	if (shortcut_pattern.size())
		if (shortcut_pattern[0] == _T(':'))
		{
			bsave_directory_to_clipboard = true;
			shortcut_pattern.erase(0, 1);
		}

	//shortcut ����� ��������� ���������
	tstring params;	//��������� �������
	tstring::iterator spos = std::find(shortcut_pattern.begin(), shortcut_pattern.end(), _T(' '));
	if (spos != shortcut_pattern.end())
	{
		std::back_insert_iterator<tstring> ins(params);
		std::copy(spos+1, shortcut_pattern.end(), ins);
		shortcut_pattern.erase(spos, shortcut_pattern.end());
	}

	while (true)	//��� ������������ ������ ����������� ���������� ������ ������
	{

	//!!!��������: ���������� ���� bTemporary ����� ��������� ������ path_XXX ����������� �������� �����
	//�������� ������ ���� �������, ��������������� ��������
		nf::tshortcuts_list list_shortcuts;
//		list_shortcuts.reserve(50);
		for (unsigned int i = 0; i < paths.size(); ++i) 
			if (! paths[i].empty()) 
				CStartSoftShortcut sss(paths[i].c_str()
					, shortcut_pattern.c_str()
					, catalog_pattern.c_str()
					, list_shortcuts
					, i);

	//�������� �������������� ������� ������ 
		try_to_minimize(paths, list_shortcuts);
		remove_exceptions(list_shortcuts);

		nf::tshortcut_info sh;
		int nret = nf::Menu::SelectSoft(list_shortcuts, sh);
		
		if (! nret) return false;

		if (true)
		{
			assert(static_cast<unsigned int>(sh.bIsTemporary) < paths.size());
			tstring path = _T("\"") + tstring(paths[static_cast<int>(sh.bIsTemporary)]) 
				+ Utils::CombinePath(sh.catalog, sh.shortcut, SLASH_DIRS) + _T("\"");

			if (nret > 0)
			{	//������� ��������� ���������
				OSVERSIONINFO version;
				version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
				GetVersionEx(&version);

				if (VER_PLATFORM_WIN32_WINDOWS == version.dwPlatformId ) //Win9X
				{
					//� Win9X ShellExecute ������� oem ���������...
					tstring oem = Utils::GetInternalToOem(path);
					HINSTANCE value = ShellExecute(0, NULL, oem.c_str(), params.c_str(), NULL, SW_SHOWNORMAL);
// 					MessageBox(0, boost::lexical_cast<tstring>(value).c_str(), _T("2"), MB_OK);
// 					MessageBox(0, path.c_str(), _T("path2"), MB_OK);
// 					MessageBox(0, params.c_str(), _T("params2"), MB_OK);
				} else {
					HINSTANCE value = ShellExecute(0, NULL, path.c_str(), params.c_str(), NULL, SW_SHOWNORMAL);
// 					MessageBox(0, boost::lexical_cast<tstring>(value).c_str(), _T("1"), MB_OK);
// 					MessageBox(0, path.c_str(), _T("path2"), MB_OK);
// 					MessageBox(0, params.c_str(), _T("params2"), MB_OK);
				}

			} else 
			switch (-nret)
			{
			case Menu::tsoft_menu::OPEN_PATH_IN_EXPLORER:
				{	//������� ����������, �� ������� ����������� ���������, � Far
					tstring program_directory;
					if (GetShortcutProgramPath(path, program_directory)) {
						nf::Commands::OpenPathInExplorer(program_directory);
					}
				} break;
			case Menu::tsoft_menu::OPEN_PATH_IN_FAR:
				{	//������� ����������, �� ������� ���������� ���������, � Explorer
					tstring program_directory;
					if (GetShortcutProgramPath(path, program_directory))
					{
						CPanelInfoWrap wrap(INVALID_HANDLE_VALUE);
						nf::Commands::OpenPath(wrap, program_directory);
					}
				} break;
			case Menu::tsoft_menu::SWITCH_IGNORE_MODE_ONOFF: continue;
			default: return false;
			}; //switch			
			
			if (bsave_directory_to_clipboard)
			{
				//��������� ������� ���������� � ������ ������
				g_FSF.CopyToClipboard(CPanelInfoWrap(hPlugin).GetPanelInfo(true).CurDir);
				//!TODO: ansi oem
			}
		} //debug if

		return true;
	}; //while

	return false;
}

bool Start::GetShortcutProgramPath(tstring const& PathToShortcut
								   , tstring &TargetProgramDirectory)
{
	//���� ����� ���� � �������� � Load �� ���������. ������� ������� ���� ��� ����
	//!TODO: ��������������� �������������� �������

	nf::tautobuffer_char buffer(PathToShortcut.size()+1 > MAX_PATH ? PathToShortcut.size()+1 : MAX_PATH);	
	lstrcpy(&buffer[0], PathToShortcut.c_str());
	PathUnquoteSpaces(&buffer[0]);

	::CoInitialize(0);
	ScopeGuard sc_counitialize_on_exit = MakeGuard(CoUninitialize);
	
	IPersistFile* ppf = 0;
	IShellLink* psh = 0;

	HRESULT hr = ::CoCreateInstance(::CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER
									, IID_IPersistFile, reinterpret_cast<void**>(&ppf));
	if (FAILED(hr)) return false;
	ScopeGuard sc_ppf_release = MakeObjGuard(*ppf, &IPersistFile::Release);

	nf::tautobuffer_byte wbuffer(2 * buffer.size()); 
	MultiByteToWideChar(CP_ACP, 0
		, &buffer[0]
		, static_cast<int>(buffer.size())
		, reinterpret_cast<wchar_t*>(&wbuffer[0])
		, static_cast<int>(wbuffer.size())
	);
	hr = ppf->Load((reinterpret_cast<wchar_t*>(&wbuffer[0])), STGM_READ);
	if (FAILED(hr)) return false;

	hr = ppf->QueryInterface(IID_IShellLink, reinterpret_cast<void**>(&psh));
	if (FAILED(hr)) return false;
	ScopeGuard sc_psh_release = MakeObjGuard(*psh, &IShellLink::Release);

	hr = psh->GetPath(&buffer[0], MAX_PATH, NULL, SLGP_UNCPRIORITY);
	if (FAILED(hr)) return false;

	::PathRemoveFileSpec(&buffer[0]);

	TargetProgramDirectory = Utils::GetOemToInternal(&buffer[0]);

	return true;
}