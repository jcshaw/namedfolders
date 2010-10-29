/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/


#pragma once
#include "header.h"

int FarCmpName(const wchar_t *Pattern, const wchar_t *String, int SkipPath);

void CloseAndStartAnotherPlugin(HANDLE hPlugin, tstring const& Command, bool bActivePanel, bool bOpenBoth);

bool OpenShortcutOnPanel(HANDLE hPlugin
						 , nf::tshortcut_value_parsed &panel
						 , tstring path
						 , bool bActivePanel
						 , bool bOpenBoth	
						 , bool bClosePlugin
						 , nf::twhat_to_search_t WhatToSearch);

bool SelectAndOpenPathOnPanel(HANDLE hPlugin, std::list<tpair_strings> const& SrcListAliasPath
							  , nf::twhat_to_search_t WhatToSearch, bool bActivePanel);

