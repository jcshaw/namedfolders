#include "stdafx.h"
#include "MenuDialog.h"

#include <boost/foreach.hpp>
#include "autobuffer_wrapper.h"
using namespace nf;
using namespace Menu;
using namespace Polices;

namespace {
	// {F53E66D6-3514-40C1-ABDD-2F74D2F92A85}
	static const GUID NF_MENU_DIALOG_GUID = { 0xf53e66d6, 0x3514, 0x40c1, { 0xab, 0xdd, 0x2f, 0x74, 0xd2, 0xf9, 0x2a, 0x85 } };


	enum {FIRST_COLUMN_MAX_WIDTH = 60, MAX_WIDTH = 256};

	FarKey additional_chars[] = { {VK_OEM_PLUS, 0}
		, {VK_OEM_1, 0}
		, {VK_OEM_1, SHIFT_PRESSED}
		, {VK_OEM_COMMA, SHIFT_PRESSED}
		, {VK_OEM_MINUS, 0}
		, {VK_OEM_PERIOD, SHIFT_PRESSED}
		, {VK_OEM_7, SHIFT_PRESSED}
		, {VK_OEM_4, 0}
		, {VK_OEM_6, 0}
		, {VK_OEM_4, SHIFT_PRESSED}
		, {VK_OEM_6, SHIFT_PRESSED}
	};
	const int number_additional_chars = sizeof(additional_chars)/sizeof(int);
	wchar_t additional_chars_translation[number_additional_chars] = {L'+'
		, L';'
		, L'Ж'
		, L'Б'
		, L'-'
		, L'Ю'
		, L'Э'
		, L'['
		, L']'
		, L'Х'
		, L'Ъ'
	};

	bool decode_additional_character(FarKey& keyCode, wchar_t &Resulwchar_t) {
		const int len = sizeof(additional_chars);
		for (int i = 0; i < number_additional_chars; ++i) {
			if (additional_chars[i].VirtualKeyCode == keyCode.VirtualKeyCode && additional_chars[i].ControlKeyState == keyCode.ControlKeyState) { 
				Resulwchar_t = additional_chars_translation[i];
				return true;
			}
		}
		return false;
	}

	inline FarKey get_fk(WORD virtualKeyCode, DWORD controlKeyState) {
		FarKey dest = {virtualKeyCode, controlKeyState};
		return dest;
	}

	void fill_menu_break_keys_buf(FarKey* pBreakCodes, autobuffer_wrapper<FarKey>& destBuf, size_t &destNumDefaultBreakKeys) {
		FarKey *p = &pBreakCodes[0];
		destNumDefaultBreakKeys = 0;
		while (p != nullptr && p->VirtualKeyCode != 0) {
			++destNumDefaultBreakKeys;
			++p;
		}
		size_t count_items = destNumDefaultBreakKeys
			+ 4  //VK_BACK, VK_SPACE + these keys with shift
			+ (L'Z' - L'A' + 1)*2 + (L'9' - L'0' + 1) 
			+ number_additional_chars
			+ 1; // 0
		destBuf.resize(static_cast<unsigned int>(count_items));

		while (nullptr != pBreakCodes && 0 != pBreakCodes->VirtualKeyCode) {
			destBuf.push_back(*pBreakCodes);
			++pBreakCodes;
		}

		destBuf.push_back(get_fk(VK_BACK, 0)); //стирание, 1 символ
		destBuf.push_back(get_fk(VK_SPACE, 0)); //пробел, 1 символ
		destBuf.push_back(get_fk(VK_BACK, SHIFT_PRESSED)); //стирание, 1 символ
		destBuf.push_back(get_fk(VK_SPACE, SHIFT_PRESSED)); //пробел, 1 символ

		for (unsigned int i = L'A'; i <= L'Z'; ++i) {
			destBuf.push_back(get_fk(i, 0));  //английские буквы
			destBuf.push_back(get_fk(i, SHIFT_PRESSED));  //русские буквы (с шифтом)
		}
		for (unsigned int i = L'0'; i <= L'9'; ++i) {
			destBuf.push_back(get_fk(i, 0));	 //цифры, 10 символов
		}
		for (unsigned int i = 0; i < number_additional_chars; ++i) {
			destBuf.push_back(additional_chars[i]);
		}
		destBuf.push_back(get_fk(0, 0));
	}

//class to generate menu item strings
	class menu_string_maker_visitor : public boost::static_visitor<tstring> { 
		std::pair<size_t, size_t> const m_Widths;
		int m_ViewMode;
	public:
		menu_string_maker_visitor(int ViewMode, std::pair<size_t, size_t> const& W) : m_ViewMode(ViewMode), m_Widths(W){};
		tstring operator() (tsh_info const& Value) const {
			switch (m_ViewMode) {
			case CMenuShortcuts::MM_VALUES_ONLY: 
				return Value.second;
			case CMenuShortcuts::MM_SHORTCUTS_AND_VALUES:
				return Utils::CombineStrings(Value.first.shortcut, Value.second, m_Widths.first);
			case CMenuShortcuts::MM_CATALOGS_AND_SHORTCUTS: 
				return Utils::CombineStrings(Value.first.catalog, Value.first.shortcut, m_Widths.first);
			case CMenuShortcuts::MM_CATALOGS_SHORTCUTS_VALUES: 
				return Utils::CombineStrings(Value.first.catalog, Value.first.shortcut, Value.second, m_Widths.first, m_Widths.second);
			default: //tshortcuts_menu::MM_SH: 
				return Value.first.shortcut;
			}; 
		}
		tstring operator() (tenv_info const& Value) const {
			switch(m_ViewMode) {
			case CMenuEnvironmentVariables::MM_PATH_ONLY: return Value.second;
			default: //tenv_menu::MM_VAR_PATH
				return Utils::CombineStrings(Value.first, Value.second, m_Widths.first);
			}
		}
		inline tstring operator() (tsoft_info const& Value) const {	
			switch (m_ViewMode) { //!TODO: добавить режимы отображения
			case CMenuApplications::MM_PATH: return Value.shortcut;
			case CMenuApplications::MM_PATH_CAT: 
				return Utils::CombineStrings(Value.shortcut, Value.catalog, m_Widths.first);
			case CMenuApplications::MM_CAT_PATH: 
				return Utils::CombineStrings(Value.catalog, Value.shortcut, m_Widths.first);
			default: return Utils::CombineStrings(Value.catalog, Value.shortcut, m_Widths.first); //VALUE_CAT_PATH
			} 
		}
		inline tstring operator() (tstr_info const& Value) const { 
			return Value; }
	};

//class to compute widths of first and second columns
	class width_visitor : public boost::static_visitor<std::pair<size_t, size_t> > { 
		int const m_ViewMode;
	public:
		width_visitor(int ViewMode) : m_ViewMode(ViewMode) {}
		inline std::pair<size_t, size_t> operator() (tsh_info const& Value) const {
			tshortcut_info const& sh = Value.first;
			tstring const& value = Value.second;
			switch (m_ViewMode) {
			case CMenuShortcuts::MM_VALUES_ONLY: return std::make_pair(value.size(), 0);
			case CMenuShortcuts::MM_SHORTCUTS_AND_VALUES: return std::make_pair(sh.shortcut.size(),value.size());
			case CMenuShortcuts::MM_CATALOGS_AND_SHORTCUTS: 
			case CMenuShortcuts::MM_CATALOGS_SHORTCUTS_VALUES: 
				return std::make_pair(sh.catalog.size(), sh.shortcut.size());
			default: return std::make_pair(sh.shortcut.size(), 0);
			}; 
		}
		inline std::pair<size_t, size_t> operator() (tenv_info const& Value) const {
			switch(m_ViewMode) {
			case CMenuEnvironmentVariables::MM_PATH_ONLY: return std::make_pair(Value.second.size(), 0);
			default: return std::make_pair(Value.first.size(), Value.second.size());
			}; 
		}
		inline std::pair<size_t, size_t> operator() (tsoft_info const& Value) const {	
			switch(m_ViewMode) {
			case CMenuApplications::MM_PATH: return std::make_pair(Value.catalog.size(), 0);
			case CMenuApplications::MM_PATH_CAT: return std::make_pair(Value.shortcut.size(), Value.catalog.size());
			default: return std::make_pair(Value.catalog.size(), Value.shortcut.size());
			}; //switch		
		}
		inline std::pair<size_t, size_t> operator() (tstr_info const& Value) const { 
			return std::make_pair(Value.size(), 0); }
	};

	//class to get column value that should be used for sorting
	class sort_visitor : public boost::static_visitor<tstring> { 
		int const m_SortMode;
		int const m_ViewMode;
		tstring const m_EmptyString;
	public:
		sort_visitor(int sortMode, int viewMode) : m_SortMode(sortMode), m_ViewMode(viewMode) {}
		inline tstring const& operator() (tsh_info const& Value) const {
			switch (m_SortMode) {
			case 1:
				switch (m_ViewMode) {
				case CMenuShortcuts::MM_SHORTCUTS_AND_VALUES: 
				case CMenuShortcuts::MM_SHORTCUTS_ONLY:
					return Value.first.shortcut;
				case CMenuShortcuts::MM_CATALOGS_AND_SHORTCUTS: 
				case CMenuShortcuts::MM_CATALOGS_SHORTCUTS_VALUES: 
					return Value.first.catalog;
				case CMenuShortcuts::MM_VALUES_ONLY: 
					return Value.second;
				}; 
				break;
			case 2:
				switch (m_ViewMode) {
				case CMenuShortcuts::MM_SHORTCUTS_AND_VALUES: 
					return Value.second;
				case CMenuShortcuts::MM_CATALOGS_AND_SHORTCUTS: 
				case CMenuShortcuts::MM_CATALOGS_SHORTCUTS_VALUES: 
					return Value.first.shortcut;
				}; 
				break;
			case 3:
				if (m_ViewMode == CMenuShortcuts::MM_CATALOGS_SHORTCUTS_VALUES) {
					return Value.second;
				}
				break;
			}
			return m_EmptyString; //sort mode is not applicable
		}
		inline tstring const& operator() (tenv_info const& Value) const {
			switch(m_SortMode) {
			case 1:
				switch (m_ViewMode) {
				case CMenuEnvironmentVariables::MM_PATH_ONLY: return Value.second;
				case CMenuEnvironmentVariables::MM_VARIABLE_AND_PATH: return Value.first;
				}
				break;
			case 2:
				if (m_ViewMode == CMenuEnvironmentVariables::MM_VARIABLE_AND_PATH) return Value.second;
				break;
			}
			return m_EmptyString; //sort mode is not applicable
		}
		inline tstring const& operator() (tsoft_info const& Value) const {	
			switch(m_SortMode) {
			case 1:
				switch (m_ViewMode) {
				case CMenuApplications::MM_PATH: 
				case CMenuApplications::MM_PATH_CAT: return Value.shortcut;
				case CMenuApplications::MM_CAT_PATH: return Value.catalog;
				}
				break;
			case 2:
				switch (m_ViewMode) {
				case CMenuApplications::MM_PATH_CAT: return Value.catalog;
				case CMenuApplications::MM_CAT_PATH: return Value.shortcut;
				}
				break;
			}
			return m_EmptyString; //sort mode is not applicable
		}
		inline tstring const& operator() (tstr_info const& Value) const { 
			if (m_SortMode == 1) {
				return Value;
			} else {
				return m_EmptyString;
			}
		}
	};
}


nf::Menu::CMenuDialog::CMenuDialog(CMenu &srcMenu, tlist_menu_items &listItemsRef, tbackground_action_maker *pBckgActionMaker)
: m_Menu(srcMenu)
, m_List(listItemsRef)
, m_bFilterFullUpdateMode(true) 
, m_MaxStringsSizes(get_column_widths(false))
, m_ViewModeForMaxStringsSizes(srcMenu.GetCurrentViewMode())
, m_pBckgActionMaker(pBckgActionMaker)
{	
	sort_items_list();
}

int nf::Menu::CMenuDialog::show_menu(tlist_far_menu_items const& MenuItems, intptr_t& BreakCode, intptr_t &nSelectedItem) {
	autobuffer_wrapper<FarKey> buf;
	size_t num_custom_break_codes = 0;
	fill_menu_break_keys_buf(m_Menu.GetBreakKeys(), buf, num_custom_break_codes);

	tstring title = m_Menu.GetMenuTitle();
	if (m_Filter.size()) {
		if (title.size()) title += L": ";
		title += m_Filter;
	}

	//display count of items at the bottom of menu
	tstring bottom = m_Menu.m_KeysInMenu;
	if (! bottom.empty()) bottom += L" ";
	bottom += L"(" + Utils::itoa(static_cast<int>(MenuItems.size()))  //visualization of results count
		+ L"," +  Utils::itoa(static_cast<int>(m_Menu.GetCurrentSortMode()))  //visualization of sort mode
		+ L")";
 
	nSelectedItem = g_PluginInfo.Menu(&nf::NF_PLUGIN_GUID
		, &NF_MENU_DIALOG_GUID 
		, -1
		, -1
		, 0	//макс кол-во видимых элементов
		, FMENU_WRAPMODE | FMENU_SHOWAMPERSAND
		, title.c_str()
		, bottom.c_str()// m_Menu.m_KeysInMenu.c_str()
		, m_Menu.m_HelpTopic.c_str()
		, (buf.size() == 0 ? 0 : &buf[0])
		, &BreakCode
		, (MenuItems.size() == 0 ? 0 : &MenuItems[0])
		, static_cast<int>(MenuItems.size())
	);
	if (BreakCode == -1) {
		if (nSelectedItem == -1) return true;	//esc
		if (nSelectedItem >= 0) return true;	//enter
	}
	m_bFilterFullUpdateMode = true;
	if (m_pBckgActionMaker != NULL && (VK_BACK != buf[static_cast<unsigned int>(BreakCode)].VirtualKeyCode) ) {
		tvariant_value dest;
		if (get_selected_item(nSelectedItem, dest)) {
			if ((*m_pBckgActionMaker)(static_cast<unsigned int>(BreakCode), dest)) return true; //action was made; continue work without closing the menu
		}
	}

	if (BreakCode < static_cast<int>(num_custom_break_codes)) return true; //нажата заданная в m_Menu.GetBreakKeys клавиша
	if (VK_BACK == buf[static_cast<unsigned int>(BreakCode)].VirtualKeyCode) {	//удаляем последний символ фильтра
		if (m_Filter.size()) m_Filter.erase(m_Filter.size()-1, 1);
	} else {
		wchar_t ch; //!DO: русские буквы так просто уже не заменить английскими в фильтре, TODO
		if (decode_additional_character(buf[static_cast<unsigned int>(BreakCode)], ch)) {
			m_Filter += ch;
		} else if (buf[static_cast<unsigned int>(BreakCode)].ControlKeyState == SHIFT_PRESSED) {	//russian letter
			wchar_t ch[2]; 
			ch[0] = static_cast<wchar_t>(buf[static_cast<unsigned int>(BreakCode)].VirtualKeyCode);
			ch[1] = 0;
			wchar_t* ptransformed_string = g_FSF.XLat(&ch[0], 0, 1, 0); //!TODO: if xlat macros is not activated then Xlat won't work properly
			m_Filter += ptransformed_string;	
		} else { //english letter
			m_Filter += buf[static_cast<unsigned int>(BreakCode)].VirtualKeyCode;
		}
		m_bFilterFullUpdateMode = false;			
	}

	return false;
}

bool nf::Menu::CMenuDialog::ShowMenu(tvariant_value &destValue, int &DestRetCode) {
	if (! m_List.size()) return 0;	//there is no suitable item 

	if ( (! (m_Menu.m_Flags & CMenu::FG_SHOW_SINGLE_VARIANT)) && (1 == m_List.size())) {	//there is a single variant - menu is not required
		destValue = m_List.begin()->second;
		DestRetCode = 1;		 
		return true;
	}

	while (true) {
		if (m_ViewModeForMaxStringsSizes != m_Menu.GetCurrentViewMode()) {
			m_ViewModeForMaxStringsSizes = m_Menu.GetCurrentViewMode();
			m_MaxStringsSizes = get_column_widths(false);
		}

		tlist_far_menu_items menu_items;
		tlist_buffers menu_buffers;
		sort_items_list();
		load_items(menu_items, menu_buffers);

		intptr_t break_code = 0;
		intptr_t nselected_item = 0;
		if (! show_menu(menu_items, break_code, nselected_item)) continue;	//filter key is pressed

		if (-1 == break_code && -1 == nselected_item) return false;	//user has canceled menu
	
		//m_List.first contain indices of items in farmenu; find item with index nselected_item
		get_selected_item(nselected_item, destValue);

		if (-1 == break_code) {
			DestRetCode = 1; //menu item is selected
			return true;
		} else {
			DestRetCode = m_Menu.MakeAction(break_code); //which action should be made
		}
		if (DestRetCode < 0) return true; //action should be made outside (i.e. delete selected shortcut)
	};
}

bool nf::Menu::CMenuDialog::get_selected_item(intptr_t nselectedItem, tvariant_value& destValue) {
	tvariant_value const *pvalue = NULL;  //std::advance(p, nselected_item) is not suitable because some items can be invisible
	BOOST_FOREACH(tmenu_item const& mi, m_List) {
		if (mi.first == nselectedItem) {
			pvalue = &mi.second;
			destValue = *pvalue;
			return true;
		}
	}
	return false;
}

std::pair<size_t, size_t> nf::Menu::CMenuDialog::get_column_widths(bool bOnlyVisibleItems) {
	//определяем максимальную ширину первого и второго столбца
	//у видимых элементов
	std::pair<size_t, size_t> widths = std::make_pair(0, 0);
	width_visitor v1(m_Menu.GetCurrentViewMode());
	BOOST_FOREACH(tmenu_item const& mi, m_List) {
		if (! bOnlyVisibleItems || mi.first >= 0) { //take into account only visible items
			std::pair<size_t, size_t> w = boost::apply_visitor(v1, mi.second);
			if (w.first > widths.first) widths.first = w.first;
			if (w.second > widths.second) widths.second = w.second;
		}
	}
	if (widths.first > FIRST_COLUMN_MAX_WIDTH) widths.first = FIRST_COLUMN_MAX_WIDTH;
	return widths;
}

void nf::Menu::CMenuDialog::set_items_visibility(tstring const& Filter, int Level, std::pair<size_t, size_t> maxSizes) {
	//decide which items will be visible 
	//menu items strings are unlimited in width to have possibility to filter menu using whole strings contents
	menu_string_maker_visitor string_maker(m_Menu.GetCurrentViewMode(), maxSizes);
	int nitems = 0;
	nf::tlist_strings filters;
	Utils::SplitStringByRegex(Filter, filters, L" ");

	BOOST_FOREACH(tmenu_item& mi, m_List) {
		if ((! m_bFilterFullUpdateMode) && (mi.first < 0) && (mi.first > -(Level-1))) continue;
		if (filters.empty()) {
			mi.first = nitems++;		
		} else {
			tstring sfilter = boost::apply_visitor(string_maker, mi.second);
		//check if sfilter fits to all filter items
			bool b = filters.end() == std::find_if(filters.begin(), filters.end() 
				, boost::bind(std::logical_not<bool>()
					, boost::bind<bool>(&Utils::iFindFirst, boost::cref(sfilter), _1)));
			if (b) { 
				mi.first = nitems++;
			} else {
				mi.first = -Level;
			}		
		}
	}
}

void nf::Menu::CMenuDialog::load_items(tlist_far_menu_items &destMenuItems, tlist_buffers &destMenuBuffers) {
	//load to menu visible items only
	assert(destMenuItems.empty());
	assert(destMenuBuffers.empty());
	destMenuItems.reserve(m_List.size());
	destMenuBuffers.reserve(m_List.size());

	set_items_visibility(m_Filter, static_cast<int>(m_Filter.size()), m_MaxStringsSizes);	//find items that are visible
	std::pair<size_t, size_t> widths = m_Filter.empty() ? m_MaxStringsSizes : get_column_widths(true);	//find max required width of menu
	menu_string_maker_visitor string_maker(m_Menu.GetCurrentViewMode(), widths);

	BOOST_FOREACH(tmenu_item const& mi, m_List) {
		if (mi.first >= 0) {
			FarMenuItem m;			
			memset(&m, 0, sizeof(FarMenuItem));
			tstring_buffer buffer(Utils::Str2Buffer(boost::apply_visitor(string_maker, mi.second)));
			if (buffer->size() > MAX_WIDTH) {
				(*buffer)[MAX_WIDTH] = 0; //same as buffer->erase(MAX_WIDTH, buffer->size() - MAX_WIDTH);
			} 
			m.Text = &(*buffer)[0];
			destMenuItems.push_back(m);
			destMenuBuffers.push_back(buffer);
			assert(destMenuItems.size() == destMenuBuffers.size());
		}
	}
}

namespace {
	inline bool compare_variants(CMenuDialog::tmenu_item &mi1, CMenuDialog::tmenu_item &mi2, sort_visitor& fs) {		
		tstring const& s1 = boost::apply_visitor(fs, mi1.second);
		tstring const& s2 = boost::apply_visitor(fs, mi2.second);
		return Utils::CmpStringLessCI()(s1, s2);
	}
}

void nf::Menu::CMenuDialog::sort_items_list() {
	//NF supports of sorting menu items since build 256
	int sort_mode = m_Menu.GetCurrentSortMode();
	if (sort_mode == 0) return; //sort is turned off
	sort_visitor fs(sort_mode, m_Menu.GetCurrentViewMode());
	std::sort(m_List.begin(), m_List.end()
		, boost::bind(&compare_variants, _1, _2, fs) );
}