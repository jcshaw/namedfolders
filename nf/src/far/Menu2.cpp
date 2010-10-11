#include "stdafx.h"
#include "menu2.h"
#include "menus_impl.h"
#include "Panel.h"

#include <list>
#include <boost/variant.hpp>
#include <boost/variant/get.hpp>
#include <boost/bind.hpp>
#include <Shlwapi.h>
#include <boost/foreach.hpp>

#include "strings_utils.h"
#include "stlcatalogs.h"
#include "MenuDialog.h"


using namespace nf;
using namespace Menu;

namespace {
	template <class TMenu, class TList, class Policy> inline 
		int select_from_menu(TList const &ListItems, typename Policy::R &DestValue)
	{
		CMenuDialog::tlist_menu_items list;
		list.reserve(ListItems.size());

		for (typename TList::const_iterator p = ListItems.begin(); p != ListItems.end(); ++p) {
			list.push_back(std::make_pair(-1, tvariant_value(Policy::Encode(*p))));	//-1 - в список элементов farmenu не добавлен
		}

		TMenu menu;
		CMenuDialog dlg(menu, list);

		int result_code;
		tvariant_value result_value;
		if (dlg.ShowMenu(result_value, result_code)) {
			DestValue = Policy::Decode(boost::get<typename Policy::V>(result_value));
			return result_code;
		}
		return 0;
	}
}

int nf::Menu::SelectShortcut(nf::tshortcuts_list const &SrcList, nf::tshortcut_info& DestSh) {
	return select_from_menu<tshortcuts_menu, tshortcuts_list, Polices::tsh>(SrcList, DestSh);
}

int nf::Menu::SelectSoft(nf::tshortcuts_list const &SrcList, nf::tshortcut_info& DestSoft) {
	return select_from_menu<tsoft_menu, tshortcuts_list, Polices::tdirect<nf::tshortcut_info> >(SrcList, DestSoft);
}

int nf::Menu::SelectCatalog(nf::tcatalogs_list const &SrcList, nf::tcatalog_info& DestCatalog) {
	return select_from_menu<tcatalogs_menu, tcatalogs_list, Polices::tdirect<tstring> >(SrcList, DestCatalog);
}

int nf::Menu::SelectPath(nf::tdirs_list const& SrcList, tstring &DestPath) {
	return select_from_menu<tpath_menu, tdirs_list, Polices::tdirect<tstring> >(SrcList, DestPath);
}

int nf::Menu::SelectPath(std::list<tstring> const& SrcList, tstring &DestPath) {
	return select_from_menu<tpath_menu, std::list<tstring>, Polices::tdirect<tstring> >(SrcList, DestPath);
}

int nf::Menu::SelectEnvVar(std::list<std::pair<tstring, tstring> > const &SrcList, std::pair<tstring, tstring> &DestVar) {
	return select_from_menu<tenv_menu, std::list<tpair_strings>, Polices::tdirect<tpair_strings> >(SrcList, DestVar);
}

