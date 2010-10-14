/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#include "StdAfx.h"
#include "startsoftshortcut.h"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
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
			tstring comma_list = CSettings::GetInstance().GetValue(nf::ST_SOFT_MASKS_TO_IGNORE_COMMA_SEPARETED);
			if (! comma_list.empty()) comma_list += L",";
			//парсим строку "a,b,c,"  в список
			Utils::SplitStringByRegex(comma_list, m_ListEx, L",");
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

CStartSoftShortcut::CStartSoftShortcut(wchar_t const* RootPathInt
									   , wchar_t const* Pattern
									   , wchar_t const* PatternDir
									   , nf::tshortcuts_list& Data
									   , int bTemporaryValue)
									   : m_RootPathOem(RootPathInt)
									   , m_Pattern(Pattern)
									   , m_PatternDir(PatternDir)
									   , m_Data(Data)
									   , m_bTemporaryValue(bTemporaryValue)
{
	if (! nf::Parser::IsContainsMetachars(m_Pattern))
	{
		//если пользователь ввел маску - искать по маске
		//иначе искать "*шаблон*"
		m_Pattern = L"*" + m_Pattern + L"*";
	}
	if (! nf::Parser::IsContainsMetachars(m_PatternDir))
	{
		//если пользователь ввел маску - искать по маске
		//иначе искать "*шаблон*"
		m_PatternDir = L"*" + m_PatternDir + L"*";
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
	load_sub_catalogs(L"");
	if (is_dir_matched_to_pattern(L"")) load_shortcuts(L"");
}

void CStartSoftShortcut::load_sub_catalogs(tstring const& RelatedPathOem)
{
	tstring RootPath = Utils::CombinePath(m_RootPathOem, RelatedPathOem, SLASH_DIRS);
	WinSTL::findfile_sequence_t f(RootPath.c_str(), L"*.*", WinSTL::findfile_sequence_t::directories);
	BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
		tstring dir_name = Utils::CombinePath(RelatedPathOem, t.get_filename(), SLASH_DIRS);
		tstring dir_nameInt = dir_name;
		if (is_dir_matched_to_pattern(dir_nameInt)) load_shortcuts(dir_name);
		load_sub_catalogs(dir_name);
	}
}

void CStartSoftShortcut::load_shortcuts(tstring const& RelatedPathOem)
{
	tstring RootPathOem = Utils::CombinePath(m_RootPathOem, RelatedPathOem, SLASH_DIRS);
	WinSTL::findfile_sequence_t f(RootPathOem.c_str(), L"*.*", 
		WinSTL::findfile_sequence_t::files);
	BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
		nf::tshortcut_info sh;
		sh.shortcut = t.get_filename();
		if (is_matched_to_pattern(sh.shortcut)) {
			sh.catalog = RelatedPathOem;
			sh.bIsTemporary = static_cast<bool>(m_bTemporaryValue != 0);
			assert(static_cast<int>(sh.bIsTemporary) == m_bTemporaryValue);	
				//!TODO: индексы больше 1 не должны потеряться...
			m_Data.push_back(sh);
		}
	} //while
}

namespace
{
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
	//выделяем у всех директорий в списке общий элемент и переносим его в Paths
	//снижаем, тем самым, размер отображаемых названий каталогов

//		std::sort(ListShortcuts.begin(), ListShortcuts.end());
		tstring common_folder;	//директория, общая для всех элементов списка
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
	{	//составляем полный список программ из стартового меню
		//получаем путь к каталогу StartMenu для всех пользователей и для текущего пользователя
		wchar_t buffer[MAX_PATH];
		int ncount_dirs = 0;
		if (SHGetSpecialFolderPath(0, &buffer[0], CSIDL_STARTMENU, FALSE)) {
			Paths.push_back(buffer);
			ncount_dirs++;
		}

		if (SHGetSpecialFolderPath(0, &buffer[0], CSIDL_COMMON_STARTMENU, FALSE)) {
			Paths.push_back(buffer);
			ncount_dirs++;
		}

		if (!ncount_dirs)  {
			// В win9x под VMWare почему то не срабатывают функции SHGetSpecialFolderPath
			// на всякий случай обходим через одно место 
			//!TODO
			tstring path;
			nf::Selectors::GetPathByEnvvarPattern(CPanelInfoWrap(hPlugin)
				, L"windir"
				, L""
				, path);
			tstring path_rus = path + L"\\Главное меню\\Программы";
			tstring path_eng = path + L"\\Start menu\\Programs";

			Paths.push_back(path_rus);
			Paths.push_back(path_eng);
		}
	}
}

bool Start::OpenSoftShortcut(HANDLE hPlugin
							 , nf::tparsed_command const &cmd)
{
	//CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE); //!TODO: see msdn  ShellExecute

	std::vector<tstring> paths;
//получаем всевозможные пути
	get_soft_variants(hPlugin, paths);

	tstring shortcut_pattern;	// = (cmd.local_directory.size()) ? cmd.shortcut : cmd.local_directory;
	tstring catalog_pattern;	// = L"*";

	if (! cmd.local_directory.size())
	{
		shortcut_pattern = cmd.shortcut;
		catalog_pattern = L"*";
	} else 
	{
		shortcut_pattern = cmd.local_directory;
		catalog_pattern = cmd.shortcut;
		Utils::RemoveLeadingCharsOnPlace(shortcut_pattern, SLASH_DIRS_CHAR);
	}
//проверяем команду :
//если вызов в виде cd: :soft то пользователь хочет, чтобы 
//плагин не просто вызвал soft, но еще и скопировал текущий путь в буфер обмена
//(например, когда открывешь Word и хочешь сохранить в текущую директорию вновь созданный файш)
	bool bsave_directory_to_clipboard = false;
	if (shortcut_pattern.size())
		if (shortcut_pattern[0] == L':')
		{
			bsave_directory_to_clipboard = true;
			shortcut_pattern.erase(0, 1);
		}

	//shortcut может содержать параметры
	tstring params;	//параметры команды
	tstring::iterator spos = std::find(shortcut_pattern.begin(), shortcut_pattern.end(), L' ');
	if (spos != shortcut_pattern.end())
	{
		std::back_insert_iterator<tstring> ins(params);
		std::copy(spos+1, shortcut_pattern.end(), ins);
		shortcut_pattern.erase(spos, shortcut_pattern.end());
	}

	while (true)	//при переключении режима отображения генерируем список заново
	{

	//!!!ВНИМАНИЕ: используем флаг bTemporary чтобы различать какому path_XXX принадлежит выбраный ярлык
	//получаем список всех ярлыков, соответствующих паттерну
		nf::tshortcuts_list list_shortcuts;
//		list_shortcuts.reserve(50);
		for (unsigned int i = 0; i < paths.size(); ++i) 
			if (! paths[i].empty()) 
				CStartSoftShortcut sss(paths[i].c_str()
					, shortcut_pattern.c_str()
					, catalog_pattern.c_str()
					, list_shortcuts
					, i);

	//пытаемся минимизировать размеры списка 
		try_to_minimize(paths, list_shortcuts);
		remove_exceptions(list_shortcuts);

		nf::tshortcut_info sh;
		int nret = nf::Menu::SelectSoft(list_shortcuts, sh);
		
		if (! nret) return false;

		if (true)
		{
			assert(static_cast<unsigned int>(sh.bIsTemporary) < paths.size());
			tstring path = L"\"" + tstring(paths[static_cast<int>(sh.bIsTemporary)]) 
				+ Utils::CombinePath(sh.catalog, sh.shortcut, SLASH_DIRS) + L"\"";
// 			tstring path1 = tstring(paths[static_cast<int>(sh.bIsTemporary)]) 
// 				+ Utils::CombinePath(sh.catalog, sh.shortcut, SLASH_DIRS);

			if (nret > 0)
			{	//открыть выбранную программу
				OSVERSIONINFO version;
				version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
				GetVersionEx(&version);

				if (VER_PLATFORM_WIN32_WINDOWS == version.dwPlatformId ) {//Win9X
					//в Win9X ShellExecute требует oem кодировки...
					tstring oem = path;
					HINSTANCE value = ShellExecute(0, NULL, oem.c_str(), params.c_str(), NULL, SW_SHOWNORMAL);
// 					MessageBox(0, boost::lexical_cast<tstring>(value).c_str(), L"2", MB_OK);
// 					MessageBox(0, path.c_str(), L"path2", MB_OK);
// 					MessageBox(0, params.c_str(), L"params2", MB_OK);
				} else { 					 
					HINSTANCE value = ShellExecuteW(0
							, NULL //L"open" //!TODO: #1
							, path.c_str()
							, params.c_str(), NULL, SW_SHOWNORMAL); 
// 					if (long(value) < 32) {
// 						SHELLEXECUTEINFO si;
// 						ZeroMemory(&si, sizeof(si));
// 						si.cbSize = sizeof(si);
// 						si.lpFile = path1.c_str();
// 						si.nShow = SW_SHOWNORMAL;
// 
// 						bool b  = ShellExecuteEx(&si);
// 						value = 0;
// 					}
// 
//  					MessageBox(0, boost::lexical_cast<tstring>(value).c_str(), L"1", MB_OK);
// 					MessageBox(0, path.c_str(), L"path2", MB_OK);
// 					MessageBox(0, params.c_str(), L"params2", MB_OK);
				}

			} else 
			switch (-nret)
			{
			case Menu::tsoft_menu::OPEN_PATH_IN_EXPLORER:
				{	//открыть директорию, из которой запускается программа, в Far
					tstring program_directory;
					if (GetShortcutProgramPath(path, program_directory)) {
						nf::Commands::OpenPathInExplorer(program_directory);
					}
				} break;
			case Menu::tsoft_menu::OPEN_PATH_IN_FAR:
				{	//открыть директорию, из которой запускатся программа, в Explorer
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
			
			if (bsave_directory_to_clipboard) {
				//сохраняем текущую директорию в буфере обмена
				g_FSF.CopyToClipboard(CPanelInfoWrap(hPlugin).GetPanelCurDir(true).c_str());
			}
		} //debug if

		return true;
	}; //while

	return false;
}

bool Start::GetShortcutProgramPath(tstring const& PathToShortcut
								   , tstring &TargetProgramDirectory)
{
	//путь может быть в кавычках и Load не сработает. Удаляем кавычки если они есть
	//!TODO: воспользоваться типизированным буфером

	nf::tautobuffer_char wbuffer(2 * (PathToShortcut.size()+1 > MAX_PATH ? PathToShortcut.size()+1 : MAX_PATH));	
	lstrcpy((reinterpret_cast<wchar_t*>(&wbuffer[0])), PathToShortcut.c_str());
	PathUnquoteSpaces((reinterpret_cast<wchar_t*>(&wbuffer[0])));

	::CoInitialize(0);
	ScopeGuard sc_counitialize_on_exit = MakeGuard(CoUninitialize);
	
	IPersistFile* ppf = 0;
	IShellLink* psh = 0;

	HRESULT hr = ::CoCreateInstance(::CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER
									, IID_IPersistFile, reinterpret_cast<void**>(&ppf));
	if (FAILED(hr)) return false;
	ScopeGuard sc_ppf_release = MakeObjGuard(*ppf, &IPersistFile::Release);

// 	nf::tautobuffer_byte wbuffer(2 * buffer.size());  //!DO: test
// 	MultiByteToWideChar(CP_ACP, 0
// 		, &buffer[0]
// 		, static_cast<int>(buffer.size())
// 		, reinterpret_cast<wchar_t*>(&wbuffer[0])
// 		, static_cast<int>(wbuffer.size())
// 	);
	hr = ppf->Load((reinterpret_cast<wchar_t*>(&wbuffer[0])), STGM_READ);
	if (FAILED(hr)) return false;

	hr = ppf->QueryInterface(IID_IShellLink, reinterpret_cast<void**>(&psh));
	if (FAILED(hr)) return false;
	ScopeGuard sc_psh_release = MakeObjGuard(*psh, &IShellLink::Release);

	hr = psh->GetPath(&wbuffer[0], MAX_PATH, NULL, SLGP_UNCPRIORITY);
	if (FAILED(hr)) return false;

	::PathRemoveFileSpec(&wbuffer[0]);

	TargetProgramDirectory = &wbuffer[0];

	return true;
}