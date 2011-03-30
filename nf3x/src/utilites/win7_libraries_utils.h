/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#pragma  once
#include "header.h"
#include "PanelInfoWrap.h"

namespace nf {
	bool AddToWin7Library(CPanelInfoWrap &hPlugin, tstring const& targetDirectory, tstring const& defaultLibraryName);
	bool RemoveFromWin7Library(CPanelInfoWrap &hPlugin, tstring const& targetDirectory, tstring const& defaultLibraryName);

	tstring GetDefaultWin7LibraryForCatalog(tstring const& catalogName);
	bool IsAddToWin7BYDefaultFlagChecked(tstring const& catalogName);
}