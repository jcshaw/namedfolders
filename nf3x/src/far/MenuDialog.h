#pragma once

#include <boost/variant.hpp>
#include <boost/variant/get.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "strings_utils.h"
#include "Panel.h"
#include "menus_impl.h"
#include "stlcatalogs.h"

#include "menu_header.h"

namespace nf {
namespace Menu {
	namespace Polices {	
		struct tsh {
			typedef tshortcut_info R;
			typedef tsh_info V;
			inline static V Encode(R const& SrcValue) {
				tstring value = L""; 
				sc::CCatalog c(SrcValue.catalog);
				c.GetShortcutInfo(SrcValue.shortcut, SrcValue.bIsTemporary, value);
				return V(boost::ref(SrcValue), value);
			}
			inline static R Decode(V const& SrcValue) { return SrcValue.first; }
		};

		template <class T>
		struct tdirect {
			typedef T R;
			typedef T V;			
			inline static V Encode(R const& SrcValue) { return SrcValue; }
			inline static R Decode(V const & SrcValue) { return SrcValue; }
		};

		struct tenv {
			typedef tstring R;
			typedef tenv_info V;		
			inline static V Encode(R const& SrcValue) {	
				return static_cast<tenv_info>(Utils::DivideString(SrcValue, _T('%'))); }
			inline static R Decode(V const& SrcValue) { 
				return Utils::CombinePath(SrcValue.first, SrcValue.second, L"%");	} //!TODO
		};
	}

	class CMenuDialog {
	public:
		typedef std::pair<int, tvariant_value> tmenu_item;	//first = item number in tlist_far_menu_items
		typedef std::vector<tmenu_item> tlist_menu_items;

		typedef std::vector<FarMenuItem> tlist_far_menu_items; 

		CMenuDialog(CMenu &M, tlist_menu_items &listItemsRef, tbackground_action_maker *pBckgActionMaker);
		bool ShowMenu(tvariant_value &DestValue, int &DestRetCode);

	private:			
		std::pair<size_t, size_t> get_column_widths(bool bOnlyVisibleItems);
		void set_items_visibility(tstring const& Filter, int Level, std::pair<size_t, size_t> maxSizes);
		void load_items(tlist_far_menu_items &destMenuItems, tlist_buffers &destMenuBuffers); 
		int show_menu(tlist_far_menu_items const& MenuItems, int& BreakCode, int &nSelectedItem);
		void sort_items_list();
	private:
		bool get_selected_item(int nselectedItem, tvariant_value& destValue);
		CMenu &m_Menu;
		tbackground_action_maker* m_pBckgActionMaker;
		tlist_menu_items &m_List;
		tstring m_Filter;	//введенный фильтр (только английские буквы)
		bool m_bFilterFullUpdateMode; 
		std::pair<size_t, size_t> m_MaxStringsSizes;
		int m_ViewModeForMaxStringsSizes;
		//при добавлении очередной буквы к фильтру нет необходимости проверять
		//строки, которые не попали в результаты предыдущего поиска
		//т.е. здесь возможна оптимизация, которая и реализуется через этот флаг
	};
} //Menu
}