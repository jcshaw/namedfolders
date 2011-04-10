/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/


#pragma once
#pragma warning(disable: 4800)

#include "header.h"
#include "Kernel.h"
#include "Parser.h"
#include "PathsFinder.h"

namespace Start {
	bool OpenSoftShortcut(HANDLE hPlugin, nf::tparsed_command const &cmd);
	bool GetShortcutProgramPath(tstring const& PathToShortcut, tstring &destPath, bool bRemoveFileName);
	void LoadShortcuts(tstring const& srcRootPath, nf::Search::MaskMatcher& mmSh, int bTemporaryValue, nf::tshortcuts_list& destData);

	void OpenApplication(tstring const& applicationPath, tstring const& launchParams, bool bActivate);
	void OpenApplicationCatalogInExplorer(tstring const& applicationPath, bool bActivate);
	void OpenApplicationPathInFAR(tstring const& applicationPath);
}

#pragma warning(default: 4800)