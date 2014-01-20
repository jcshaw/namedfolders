/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#include "stdafx.h"
#include "win7_libraries.h"

#include <shobjidl.h>
#include <Shlobj.h>

#include <KnownFolders.h>
#include <boost/foreach.hpp>
#include <boost/scope_exit.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <INITGUID.H> //to avoid unresolved external error on definition FOLDERTYPEID_Library_FOR_NF, see http://support.microsoft.com/kb/130869

#include "stlsoft_def.h"
#include "known_folders.h"
#include "x64.h"

//see https://cfx.svn.codeplex.com/svn/Visual%20Studio%202008/CppWin7ShellLibrary/CppWin7ShellLibrary.cpp
namespace {
	class function_loader {
		typedef HRESULT (__stdcall t_SHCreateItemFromParsingName)(PCWSTR pszPath, IBindCtx *pbc, REFIID riid, void **ppv);
		typedef HRESULT (__stdcall t_SHLoadLibraryFromItem)(IShellItem *psiLibrary, DWORD grfMode, REFIID riid, void **ppv);
		t_SHCreateItemFromParsingName* m_pf;
		t_SHLoadLibraryFromItem* m_pf2;
		HINSTANCE m_hinstLib;
	public:
		function_loader() {
			m_pf = NULL;
			m_hinstLib = ::LoadLibrary(TEXT("Shell32.dll")); 
			if (m_hinstLib != NULL) { 
				m_pf = (t_SHCreateItemFromParsingName*)::GetProcAddress(m_hinstLib, "SHCreateItemFromParsingName"); 
				m_pf2 = (t_SHLoadLibraryFromItem*)::GetProcAddress(m_hinstLib, "SHLoadLibraryFromItem"); 
			} 
		}
		~function_loader() {
			::FreeLibrary(m_hinstLib); 
		}
		HRESULT SHCreateItemFromParsingName(PCWSTR pszPath, IBindCtx *pbc, REFIID riid, void **ppv) {			
			if (m_pf != NULL) {
				return m_pf(pszPath, pbc, riid, ppv);
			} else {
				return E_FAIL;
			}
		}
		HRESULT SHLoadLibraryFromItem(IShellItem *psiLibrary, DWORD grfMode, REFIID riid, void **ppv) {
			if (m_pf2 != NULL) {
				return m_pf2(psiLibrary, grfMode, riid, ppv);
			} else {
				return E_FAIL;
			}
		}
	};

	void open_library_and_make_action(function_loader &shell32, tstring const& libraryFilePath, boost::function<void (IShellLibrary*)> funcAction) {
		IShellItem2* psi_item = NULL;
// 		HRESULT hr = GetShellLibraryItem(Utils::ExtractFileName(libraryFilePath, false).c_str(), &psi_item);
		HRESULT hr = shell32.SHCreateItemFromParsingName(libraryFilePath.c_str(), NULL, IID_PPV_ARGS(&psi_item));
 		if (SUCCEEDED(hr))  {
			BOOST_SCOPE_EXIT ((&psi_item)) {
				psi_item->Release();
			} BOOST_SCOPE_EXIT_END;

			IShellLibrary *plib = NULL;
			hr = shell32.SHLoadLibraryFromItem(psi_item, STGM_READWRITE, IID_PPV_ARGS(&plib));
			if (SUCCEEDED(hr)) {
				BOOST_SCOPE_EXIT ((&plib)) {
					plib->Release();
				} BOOST_SCOPE_EXIT_END;

				funcAction(plib);
			}
		}
	}

	void extract_list_folders_from_lib(IShellLibrary *plib, nf::tlist_strings &destList) {
		IShellItemArray *pfolders;
		if (SUCCEEDED(plib->GetFolders(LFF_FORCEFILESYSTEM, IID_PPV_ARGS(&pfolders)))) {
			BOOST_SCOPE_EXIT ((&pfolders)) {
				pfolders->Release();
			} BOOST_SCOPE_EXIT_END;

			DWORD count_items = 0;
			if (SUCCEEDED(pfolders->GetCount(&count_items))) {
				for (unsigned int i = 0; i < count_items; ++i) {
					IShellItem* ppsi;
					if (SUCCEEDED(pfolders->GetItemAt(i, &ppsi))) {
						LPWSTR folder_path = NULL;
						if (SUCCEEDED(ppsi->GetDisplayName(SIGDN_FILESYSPATH, &folder_path))) {
							destList.push_back(folder_path);
							CoTaskMemFree(folder_path);
						}
						ppsi->Release();
					}
				}
			}
		}
	}

	void add_remove_folder_to_lib(function_loader &shell32, IShellLibrary *plib, tstring const& folderPath, bool bAdd) {
		IShellItem *psi_item;
		HRESULT hr = shell32.SHCreateItemFromParsingName(folderPath.c_str(), NULL, IID_PPV_ARGS(&psi_item));
		if (SUCCEEDED(hr)) {
			if (bAdd) {
				HRESULT hr = plib->AddFolder(psi_item);
				if (SUCCEEDED(hr)) {
					plib->Commit();
				}
			} else {
				if (SUCCEEDED(plib->RemoveFolder(psi_item))) {
					plib->Commit();
				}
			}
			psi_item->Release();
		}
	}	
}


nf::Win7LibrariesManager::Win7LibrariesManager() 
	: m_bEnabled(true || ! nf::x64::IsWow64())
{
	// when running the 32-bit version of Plugin on x64 OS,
	// we must not create the library! This would break
	// the library in the x64 explorer.
}

nf::Win7LibrariesManager::~Win7LibrariesManager()
{

}

void nf::Win7LibrariesManager::GetListLibraries(nf::tlist_pairs_strings& destList) const{
	if (! m_bEnabled) return;

	KnownFoldersManager kfm;
//we need to enumerate list of files in Known Folder "Library"
	if (!kfm.AreKnownFoldersEnabled()) return;

	tstring libraries_catalog_path = kfm.GetLibraryPath(FOLDERID_Libraries);
	if (libraries_catalog_path.empty()) return;

	WinSTL::findfile_sequence_t f(libraries_catalog_path.c_str(), L"*.library-ms", WinSTL::findfile_sequence_t::files);
	BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
		tstring dir_name = Utils::CombinePath(libraries_catalog_path, t.get_filename(), SLASH_DIRS);
		destList.push_back(std::make_pair(dir_name, Utils::RemoveExtension(Utils::ExtractFileName(dir_name, false)))); //!TODO: remove extension
	}
}

void nf::Win7LibrariesManager::GetListFoldersInLibrary(tstring const& libraryFilePath, nf::tlist_strings& destList) const {
	if (! m_bEnabled) return;

	function_loader f;
	open_library_and_make_action(f, libraryFilePath
		, boost::bind(&extract_list_folders_from_lib, _1, boost::ref(destList)));
}

void nf::Win7LibrariesManager::AddFolderToLibrary(tstring const& libraryFilePath, tstring const& folderPath) const {
	if (! m_bEnabled) return;

	function_loader f;
	open_library_and_make_action(f, libraryFilePath
		, boost::bind(&add_remove_folder_to_lib, boost::ref(f), _1, boost::cref(folderPath), true));
}

void nf::Win7LibrariesManager::RemoveFolderFromLibrary(tstring const& libraryFilePath, tstring const& folderPath) const {
	if (! m_bEnabled) return;

	function_loader f;
	open_library_and_make_action(f, libraryFilePath
		, boost::bind(&add_remove_folder_to_lib, boost::ref(f), _1, boost::cref(folderPath), false));	
}