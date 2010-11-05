#pragma once

#include "header.h"
namespace nf {
namespace Menu {
	int SelectShortcut(nf::tshortcuts_list const &SrcList, nf::tshortcut_info& DestSh);
	int SelectSoft(nf::tshortcuts_list const &SrcList, nf::tshortcut_info& DestSh);
	int SelectCatalog(nf::tvector_strings const &SrcList, nf::tcatalog_info& DestCatalog);
	int SelectPath(nf::tlist_strings const& SrcList, tstring &DestPath);
	int SelectEnvVar(tlist_pairs_strings const &SrcList, tpair_strings &DestVar);
	inline int SelectPathByRegKey(tlist_pairs_strings const& SrcList, tpair_strings &DestPath) {
		return SelectEnvVar(SrcList, DestPath);
	}
} //Menu
} //nf