#pragma once

#include <boost/variant.hpp>
#include <boost/variant/get.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "strings_utils.h"
#include "Panel.h"
#include "menus_impl.h"
#include "stlcatalogs.h"

namespace nf {
namespace Menu {
	typedef std::pair<tshortcut_info, tstring> tsh_info;
	typedef tshortcut_info tsoft_info;
	typedef std::pair<tstring, tstring> tenv_info;	
	typedef tstring tstr_info;

	//элементы в списке меню будем хранить в виде варианта
	typedef boost::variant<tsh_info, tenv_info, tsoft_info, tstr_info> tvariant_value;

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
		typedef std::vector<boost::shared_ptr<tstring> > tlist_far_menu_buffers;

		CMenuDialog(CMenu &M, tlist_menu_items &listItemsRef);
		bool ShowMenu(tvariant_value &DestValue, int &DestRetCode);
	private:			
		std::pair<size_t, size_t> get_column_widths();
		void set_items_visibility(tstring const& Filter, int Level);
		void load_items(tlist_far_menu_items &destMenuItems, tlist_far_menu_buffers &destMenuBuffers); 
		int show_menu(tlist_far_menu_items const& MenuItems, int& BreakCode, int &nSelectedItem);
	private:
		CMenu &m_Menu;
		tlist_menu_items &m_List;
		tstring m_Filter;	//введенный фильтр (только английские буквы)
		bool m_bFilterFullUpdateMode; 
		//при добавлении очередной буквы к фильтру нет необходимости проверять
		//строки, которые не попали в результаты предыдущего поиска
		//т.е. здесь возможна оптимизация, которая и реализуется через этот флаг
	};
} //Menu
}