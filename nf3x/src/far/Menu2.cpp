#include "stdafx.h"
#include "menu2.h"
#include "menus_impl.h"
#include "Panel.h"

#include <list>
#include <boost/variant.hpp>
#include <boost/variant/get.hpp>
#include <boost/foreach.hpp>

#include "strings_utils.h"
#include "MenuDialog.h"

namespace {
	template <class TMenu, class TList, class Policy> inline 
		int select_from_menu(TList const &ListItems, typename Policy::R &DestValue) {
		using namespace nf;
		using namespace Menu;

		CMenuDialog::tlist_menu_items list;
		list.reserve(ListItems.size());

		BOOST_FOREACH(TList::value_type const& value, ListItems) {
			list.push_back(std::make_pair(-1, tvariant_value(Policy::Encode(value))));	//-1 means, that the item hasn't add to farmenu 
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
	return select_from_menu<CMenuShortcuts, tshortcuts_list, Polices::tsh>(SrcList, DestSh);
}

int nf::Menu::SelectSoft(nf::tshortcuts_list const &SrcList, nf::tshortcut_info& DestSoft) {
	return select_from_menu<CMenuApplications, tshortcuts_list, Polices::tdirect<nf::tshortcut_info> >(SrcList, DestSoft);
}

int nf::Menu::SelectCatalog(nf::tcatalogs_list const &SrcList, nf::tcatalog_info& DestCatalog) {
	return select_from_menu<CMenuCatalogs, tcatalogs_list, Polices::tdirect<tstring> >(SrcList, DestCatalog);
}

int nf::Menu::SelectPath(std::list<tstring> const& SrcList, tstring &DestPath) {
	return select_from_menu<CMenuPaths, std::list<tstring>, Polices::tdirect<tstring> >(SrcList, DestPath);
}

int nf::Menu::SelectEnvVar(std::list<std::pair<tstring, tstring> > const &SrcList, std::pair<tstring, tstring> &DestVar) {
	return select_from_menu<CMenuEnvironmentVariables, std::list<tpair_strings>, Polices::tdirect<tpair_strings> >(SrcList, DestVar);
}

