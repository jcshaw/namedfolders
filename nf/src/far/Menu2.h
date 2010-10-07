#pragma once

#include "header.h"
namespace nf
{

namespace Menu
{
	int SelectShortcut(nf::tshortcuts_list const &SrcList, nf::tshortcut_info& DestSh);
	int SelectSoft(nf::tshortcuts_list const &SrcList, nf::tshortcut_info& DestSh);
	int SelectCatalog(nf::tcatalogs_list const &SrcList, nf::tcatalog_info& DestCatalog);
	int SelectPath(std::list<tstring> const& SrcList, tstring &DestPath);
	int SelectPath(nf::tdirs_list const& SrcList, tstring &DestPath);
	int SelectEnvVar(std::list<std::pair<tstring, tstring> > const &SrcList
		, std::pair<tstring, tstring> &DestVar);
	inline int SelectPathByRegKey(std::list<std::pair<tstring, tstring> > const& SrcList
		, std::pair<tstring, tstring> &DestPath)
	{
		return SelectEnvVar(SrcList, DestPath);
	}
} //Menu


} //nf