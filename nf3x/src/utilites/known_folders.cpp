/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/


#include "stdafx.h"
#include "known_folders.h"
#include "searcherpaths.h"

#include <boost/scope_exit.hpp>
#include <boost/foreach.hpp>

nf::KnownFoldersManager::KnownFoldersManager() 
: m_pKfm(NULL)
{
	HRESULT hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pKfm));
	if (! SUCCEEDED(hr)) {
		m_pKfm = NULL;
	} 
}

nf::KnownFoldersManager::~KnownFoldersManager() {
	if (m_pKfm != NULL) {
		m_pKfm->Release();
	}
}

//see https://cfx.svn.codeplex.com/svn/Visual%20Studio%202008/CppShellKnownFolders/ReadMe.txt
void nf::KnownFoldersManager::GetListUsefulFolders(nf::tlist_pairs_strings &destNamePath) {
	KNOWNFOLDERID *rgKFIDs = NULL;
	UINT cKFIDs = 0;
	HRESULT hr = m_pKfm->GetFolderIds(&rgKFIDs, &cKFIDs);
	if (SUCCEEDED(hr)) {
		IKnownFolder *pkfCurrent = NULL;
		for (UINT i = 0; i < cKFIDs; ++i) {
			hr = m_pKfm->GetFolder(rgKFIDs[i], &pkfCurrent);
			if (SUCCEEDED(hr)) {
				KNOWNFOLDER_DEFINITION kfd;
				hr = pkfCurrent->GetFolderDefinition(&kfd);
				if (SUCCEEDED(hr)) {
					PWSTR pszPath = NULL;
					hr = pkfCurrent->GetPath(0, &pszPath); 
					if (SUCCEEDED(hr)) {
						destNamePath.push_back(std::make_pair(kfd.pszName, pszPath));						
						CoTaskMemFree(pszPath);
					}					
					FreeKnownFolderDefinitionFields(&kfd);
				}
				pkfCurrent->Release();
			}			
		}		
		CoTaskMemFree(rgKFIDs);
	}
}

void nf::KnownFoldersManager::FindFolders(nf::Search::MaskMatcher &maskMatcher, nf::tlist_pairs_strings &destNamePath) {
	nf::tlist_pairs_strings list;
	GetListUsefulFolders(list);
	BOOST_FOREACH (tpair_strings const& kvp, list) {
		if (maskMatcher.MatchTo(kvp.first) ) {
			destNamePath.push_back(kvp);
		}
	}
}

tstring nf::KnownFoldersManager::GetLibraryPath(GUID libraryGUID) {
	PWSTR pszPath = NULL;
	IKnownFolder* pkf = NULL;
	HRESULT hr = m_pKfm->GetFolder(libraryGUID, &pkf);
	if (SUCCEEDED(hr)) {
		hr = pkf->GetPath(0, &pszPath);
		if (SUCCEEDED(hr)) {
			tstring ret = pszPath;
			CoTaskMemFree(pszPath);
			return ret;
		}
		pkf->Release();
	}
	return L"";
}