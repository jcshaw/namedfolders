#pragma once

#include "header.h"
namespace nf
{
	namespace Menu {
		int SelectShortcut(nf::tshortcuts_list const &SrcList, nf::tshortcut_info& DestSh);
		int SelectSoft(nf::tshortcuts_list const &SrcList, nf::tshortcut_info& DestSh);
		int SelectCatalog(nf::tcatalogs_list const &SrcList, nf::tcatalog_info& DestCatalog);
		int SelectPath(std::list<tstring> const& SrcList, tstring &DestPath);
		int SelectPath(nf::tdirs_list const& SrcList, tstring &DestPath);
		int SelectEnvVar(std::list<tpair_strings> const &SrcList, tpair_strings &DestVar);
		inline int SelectPathByRegKey(std::list<tpair_strings> const& SrcList, tpair_strings &DestPath) {
			return SelectEnvVar(SrcList, DestPath);
		}
	} //Menu


} //nf