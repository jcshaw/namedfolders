/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#pragma once
#include "searcherpaths.h"
#include <ShObjIdl.h>

namespace nf {

class KnownFoldersManager {
public:
	KnownFoldersManager();
	virtual ~KnownFoldersManager();

	bool AreKnownFoldersEnabled() const {
		return m_pKfm != NULL; }
	void GetListUsefulFolders(nf::tlist_pairs_strings &destNamePath);
	void FindFolders(nf::Search::MaskMatcher &maskMatcher, nf::tlist_pairs_strings &destNamePath);
private:
	IKnownFolderManager* m_pKfm;
};

}