#pragma once
#include <boost/variant.hpp>
#include <boost/function.hpp>
namespace nf {
namespace Menu {
	typedef std::pair<tshortcut_info, tstring> tsh_info;
	typedef tshortcut_info tsoft_info;
	typedef std::pair<tstring, tstring> tenv_info;	
	typedef tstring tstr_info;

	//элементы в списке меню будем хранить в виде варианта
	typedef boost::variant<tsh_info, tenv_info, tsoft_info, tstr_info> tvariant_value;

	//optional background maker; it makes action for selected item without closing menu;
	//gets two params: break code ID and selected item.
	//returns true if action was handled by it.
	typedef boost::function<bool (int, tvariant_value)> tbackground_action_maker;

}
}
