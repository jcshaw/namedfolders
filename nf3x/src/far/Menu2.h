#pragma once

#include "header.h"
#include "menu_header.h"
namespace nf {
namespace Menu {
	int SelectShortcut(nf::tshortcuts_list const &SrcList, nf::tshortcut_info& DestSh);
	int SelectSoft(nf::tshortcuts_list const &SrcList, nf::tshortcut_info& DestSh, nf::Menu::tbackground_action_maker* pBackgroundActionMaker);
	int SelectCatalog(nf::tvector_strings const &SrcList, nf::tcatalog_info& DestCatalog);
	int SelectPath(nf::tlist_strings const& SrcList, tstring &DestPath);
	int SelectStringPair(tlist_pairs_strings const &SrcList, tpair_strings &DestVar);
} //Menu
} //nf