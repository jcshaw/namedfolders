/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/


#pragma once
#include "header.h"

int FarCmpName(const char *Pattern, const char *String, int SkipPath);

void CloseAndStartAnotherPlugin(HANDLE hPlugin
								, tstring const& Command
								, bool bActivePanel
								, bool bOpenBoth);

bool OpenShortcutOnPanel(HANDLE hPlugin
						 , nf::tshortcut_value_parsed &panel
						 , tstring path
						 , bool bActivePanel
						 , bool bOpenBoth	
						 , bool bClosePlugin
						 , nf::twhat_to_search_t WhatToSearch);

bool SelectAndOpenPathOnPanel(HANDLE hPlugin
							  , std::list<std::pair<tstring, tstring> > const& SrcListAliasPath
							  , nf::twhat_to_search_t WhatToSearch);

