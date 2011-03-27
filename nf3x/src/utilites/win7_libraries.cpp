/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#include "stdafx.h"
#include "win7_libraries.h"

#include <shobjidl.h>
#include <KnownFolders.h>
#include <boost/foreach.hpp>
#include <boost/scope_exit.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <INITGUID.H> //to avoid unresolved external error on definition FOLDERTYPEID_Library_FOR_NF, see http://support.microsoft.com/kb/130869

#include "stlsoft_def.h"
#include "known_folders.h"

namespace {
	void open_library_and_make_action(tstring const& libraryFilePath, boost::function<void (IShellLibrary*)> funcAction) {
		IShellItem *psi_item;
		if (SUCCEEDED(SHCreateItemFromParsingName(libraryFilePath.c_str(), NULL, IID_PPV_ARGS(&psi_item)))) 	{
			BOOST_SCOPE_EXIT ((&psi_item)) {
				psi_item->Release();
			} BOOST_SCOPE_EXIT_END;

			IShellLibrary *plib;
			if (SUCCEEDED(SHLoadLibraryFromItem(psi_item, STGM_READ | STGM_SHARE_DENY_NONE, IID_PPV_ARGS(&plib)))) {
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
						BOOST_SCOPE_EXIT ((&ppsi)) {
							ppsi->Release();
						} BOOST_SCOPE_EXIT_END;

						LPWSTR folder_path = NULL;
						if (SUCCEEDED(ppsi->GetDisplayName(SIGDN_FILESYSPATH, &folder_path))) {
							BOOST_SCOPE_EXIT ((&folder_path)) {
								CoTaskMemFree(folder_path);
							} BOOST_SCOPE_EXIT_END;

							destList.push_back(folder_path);
						}
					}
				}
			}

		}
	}

	void add_remove_folder_to_lib(IShellLibrary *plib, tstring const& folderPath, bool bAdd) {
		IShellItem *psi_item;
		if (SUCCEEDED(SHCreateItemFromParsingName(folderPath.c_str(), NULL, IID_PPV_ARGS(&psi_item)))) 	{
			BOOST_SCOPE_EXIT ((&psi_item)) {
				psi_item->Release();
			} BOOST_SCOPE_EXIT_END;

			if (bAdd) {
				plib->AddFolder(psi_item);
			} else {
				plib->RemoveFolder(psi_item);
			}
		}
	}
}

nf::Win7LibrariesManager::Win7LibrariesManager() {

}

nf::Win7LibrariesManager::~Win7LibrariesManager()
{

}

void nf::Win7LibrariesManager::GetListLibraries(nf::tlist_pairs_strings& destList) {
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

void nf::Win7LibrariesManager::GetListFoldersInLibrary(tstring const& libraryFilePath, nf::tlist_strings& destList) {
	open_library_and_make_action(libraryFilePath
		, boost::bind(&extract_list_folders_from_lib, _1, boost::ref(destList)));
}

void nf::Win7LibrariesManager::AddFolderToLibrary(tstring const& libraryFilePath, tstring const& folderPath) {
	open_library_and_make_action(libraryFilePath
		, boost::bind(&add_remove_folder_to_lib, _1, boost::cref(folderPath), true));
}

void nf::Win7LibrariesManager::RemoveFolderFromLibrary(tstring const& libraryFilePath, tstring const& folderPath) {
	open_library_and_make_action(libraryFilePath
		, boost::bind(&add_remove_folder_to_lib, _1, boost::cref(folderPath), false));	
}