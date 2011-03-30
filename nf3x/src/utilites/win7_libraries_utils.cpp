/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "stdafx.h"
#include "win7_libraries_utils.h"
#include <boost/foreach.hpp>

#include "win7_libraries.h"
#include "catalog_utils.h"
#include "Menu2.h"
#include "confirmations.h"

namespace {
	bool select_library( tstring const& defaultLibraryName, nf::tlist_pairs_strings listLibs, tpair_strings& destLib) {
		BOOST_FOREACH(tpair_strings const& kvp, listLibs) {
			if (kvp.second == defaultLibraryName) {
				destLib = kvp;
				return true;
			}
		}
		int ret = nf::Menu::SelectStringPair(listLibs, destLib);
		return (ret >= 0);
	}

	bool find_library(nf::Win7LibrariesManager const& m, tstring const& defaultLibraryName, nf::tlist_pairs_strings listLibs, tstring const& targetDirectory, nf::tlist_pairs_strings& destLibs) {
		BOOST_FOREACH(tpair_strings const& kvp, listLibs) {
			nf::tlist_strings paths;
			m.GetListFoldersInLibrary(kvp.first, paths);
			BOOST_FOREACH(tstring const& path, paths) {
				if (lstrcmpi(path.c_str(), targetDirectory.c_str()) == 0) {
					destLibs.push_back(kvp);
					break;
				}
			}
		}	
		return destLibs.size() != 0;
	}
	inline bool ask_user(CPanelInfoWrap &hPlugin, tstring const& targetLibrary, tstring const& directoryName, bool bRemove) {
		return 2 == nf::Confirmations::AskForAddRemoveDirectoryToWin7Library(hPlugin, targetLibrary, directoryName, bRemove);
	}
}

bool nf::AddToWin7Library(CPanelInfoWrap &plugin, tstring const& targetDirectory, tstring const& defaultLibraryName) {
	nf::tlist_pairs_strings list_libs;
	nf::Win7LibrariesManager m;
	m.GetListLibraries(list_libs);

	if (list_libs.size() == 0) return false; 

	if (ask_user(plugin, L"", targetDirectory, false)) {
		tpair_strings target_lib;
		if (! select_library(defaultLibraryName, list_libs, target_lib)) return false;

		m.AddFolderToLibrary(target_lib.first, targetDirectory);
	}
	return true;
};

bool nf::RemoveFromWin7Library(CPanelInfoWrap &plugin, tstring const& targetDirectory, tstring const& defaultLibraryName) {
	nf::tlist_pairs_strings list_libs;
	nf::Win7LibrariesManager m;
	m.GetListLibraries(list_libs);

	if (list_libs.size() == 0) return false; 

	nf::tlist_pairs_strings target_libs;
	if (! find_library(m, defaultLibraryName, list_libs, targetDirectory, target_libs)) return false;

	BOOST_FOREACH(tpair_strings const& lib, target_libs) {
		if (ask_user(plugin, lib.second, targetDirectory, true)) {
			m.RemoveFolderFromLibrary(lib.first, targetDirectory);
		}
	}
	return true;
}

tstring nf::GetDefaultWin7LibraryForCatalog(tstring const& catalogName) {
	tcatalog_properties p;
	nf::sc::LoadCatalogProperties(catalogName, p);
	return p.default_win7_lib;
}

bool nf::IsAddToWin7BYDefaultFlagChecked(tstring const& catalogName) {
	tcatalog_properties p;
	nf::sc::LoadCatalogProperties(catalogName, p);
	return p.flag_add_to_win7_lib_by_default;
}