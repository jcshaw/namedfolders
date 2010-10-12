#include "stdafx.h"
#include "MenuDialog.h"

#include <boost/foreach.hpp>

using namespace nf;
using namespace Menu;
using namespace Polices;

namespace {
	int additional_chars[] = { VK_OEM_PLUS
		, VK_OEM_1
		, MAKELONG(VK_OEM_1, PKF_SHIFT)
		, MAKELONG(VK_OEM_COMMA, PKF_SHIFT) 
		, VK_OEM_MINUS
		, MAKELONG(VK_OEM_PERIOD, PKF_SHIFT)
		, MAKELONG(VK_OEM_7, PKF_SHIFT)
		, VK_OEM_4
		, VK_OEM_6
		, MAKELONG(VK_OEM_4, PKF_SHIFT)
		, MAKELONG(VK_OEM_6, PKF_SHIFT)
	};
	const int number_additional_chars = sizeof(additional_chars)/sizeof(int);
	wchar_t additional_chars_translation[number_additional_chars] = {L'+'
		, L';'
		, L'�'
		, L'�'
		, L'-'
		, L'�'
		, L'�'
		, L'['
		, L']'
		, L'�'
		, L'�'
	};

	bool decode_additional_character(int KeyCode, wchar_t &Resulwchar_t) {
		const int len = sizeof(additional_chars);
		for (int i = 0; i < number_additional_chars; ++i) {
			if (additional_chars[i] == KeyCode) {
				Resulwchar_t = additional_chars_translation[i];
				return true;
			}
		}
		return false;
	}

//class to generate menu item strings
	class menu_string_maker_visitor : public boost::static_visitor<tstring> { 
		std::pair<size_t, size_t> const& m_Widths;
		int m_ViewMode;
	public:
		menu_string_maker_visitor(int ViewMode, std::pair<size_t, size_t> const& W) : m_ViewMode(ViewMode), m_Widths(W){};
		tstring operator() (tsh_info const& Value) const {
			switch (m_ViewMode) {
			case tshortcuts_menu::MM_VAL: 
				return Value.second;
			case tshortcuts_menu::MM_SH_VAL:
				return Utils::CombineStrings(Value.first.shortcut, Value.second, m_Widths.first);
			case tshortcuts_menu::MM_CAT_SH: 
				return Utils::CombineStrings(Value.first.catalog, Value.first.shortcut, m_Widths.first);
			case tshortcuts_menu::MM_CAT_SH_VAL: 
				return Utils::CombineStrings(Value.first.catalog, Value.first.shortcut, Value.second, m_Widths.first, m_Widths.second);
			default: //tshortcuts_menu::MM_SH: 
				return Value.first.shortcut;
			}; 
		}
		tstring operator() (tenv_info const& Value) const {
			switch(m_ViewMode) {
			case tenv_menu::MM_PATH: return Value.second;
			default: //tenv_menu::MM_VAR_PATH
				return Utils::CombineStrings(Value.first, Value.second, m_Widths.first);
			}
		}
		inline tstring operator() (tsoft_info const& Value) const {	
			switch (m_ViewMode) { //!TODO: �������� ������ �����������
			case tsoft_menu::MM_PATH: return Value.shortcut;
			default: return Utils::CombineStrings(Value.catalog, Value.shortcut, m_Widths.first);
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
			case tshortcuts_menu::MM_VAL: return std::make_pair(value.size(), 0);
			case tshortcuts_menu::MM_SH_VAL: return std::make_pair(sh.shortcut.size(),value.size());
			case tshortcuts_menu::MM_CAT_SH: 
			case tshortcuts_menu::MM_CAT_SH_VAL: 
				return std::make_pair(sh.catalog.size(), sh.shortcut.size());
			default: return std::make_pair(sh.shortcut.size(), 0);
			}; 
		}
		inline std::pair<size_t, size_t> operator() (tenv_info const& Value) const {
			switch(m_ViewMode) {
			case tenv_menu::MM_PATH: return std::make_pair(Value.second.size(), 0);
			default: return std::make_pair(Value.first.size(), Value.second.size());
			}; 
		}
		inline std::pair<size_t, size_t> operator() (tsoft_info const& Value) const {	
			switch(m_ViewMode) {
			case tenv_menu::MM_PATH: return std::make_pair(Value.catalog.size(), 0);
			default: return std::make_pair(Value.catalog.size(), Value.shortcut.size());
			}; //switch		
		}
		inline std::pair<size_t, size_t> operator() (tstr_info const& Value) const { 
			return std::make_pair(Value.size(), 0); }
	};
}


nf::Menu::CMenuDialog::CMenuDialog(tmenu &M, tlist_menu_items &listItemsRef, tlist_far_menu_buffers &buffersRef) 
: m_Menu(M)
, m_List(listItemsRef)
, m_bFilterFullUpdateMode(true)
, m_Buffers(buffersRef)
{

}
int nf::Menu::CMenuDialog::fill_menu_break_keys_buf( const int BufSize, int *buf, int &DestNumDefaultBreakKeys )
{
	int nbuf_count = 0;
	int *pbreak_codes = m_Menu.GetBreakKeys();
	while (0 != pbreak_codes && 0 != *pbreak_codes) {
		buf[nbuf_count++] = *pbreak_codes;
		++pbreak_codes;
		assert(nbuf_count < BufSize);
	}
	DestNumDefaultBreakKeys = nbuf_count;
	const wchar_t first_letter = L'A';
	const wchar_t last_letter = L'Z';
	assert(BufSize > nbuf_count + (2*(last_letter - first_letter) + 1) + 1 + 10);

	buf[nbuf_count++] = VK_BACK; //��������, 1 ������
	buf[nbuf_count++] = VK_SPACE; //������, 1 ������
	buf[nbuf_count++] = MAKELONG(VK_BACK, PKF_SHIFT); //��������, 1 ������
	buf[nbuf_count++] = MAKELONG(VK_SPACE, PKF_SHIFT); //������, 1 ������

	for (unsigned int i = first_letter; i <= last_letter; ++i) {
		buf[nbuf_count++] = i;  //���������� �����
		buf[nbuf_count++] = MAKELONG(i, PKF_SHIFT);  //������� ����� (� ������)
	}
	for (unsigned int i = L'0'; i <= L'9'; ++i) buf[nbuf_count++] = i;	 //�����, 10 ��������
	for (unsigned int i = 0; i < number_additional_chars; ++i) {
		buf[nbuf_count++] = additional_chars[i];
	}
	buf[nbuf_count++] = 0;

	return nbuf_count;
}

int nf::Menu::CMenuDialog::show_menu(tlist_far_menu_items const& MenuItems, int& BreakCode, int &nSelectedItem) {
	const int MAX_SIZE = 512;	//!TODO: use autobuffer
	int buf[MAX_SIZE];
	int num_custom_break_codes = 0;
	int num_break_keys = fill_menu_break_keys_buf(MAX_SIZE, buf, num_custom_break_codes);

	tstring title = m_Menu.GetMenuTitle();
	if (m_Filter.size()) {
		if (title.size()) title += L": ";
		title += m_Filter;
	}

	nSelectedItem = g_PluginInfo.Menu (
		g_PluginInfo.ModuleNumber
		, -1
		, -1
		, 0	//���� ���-�� ������� ���������
		, FMENU_WRAPMODE | FMENU_SHOWAMPERSAND
		, title.c_str()
		, m_Menu.m_KeysInMenu.c_str()
		, m_Menu.m_HelpTopic.c_str()
		, &buf[0]
		, &BreakCode
		, &MenuItems[0]
		, static_cast<int>(MenuItems.size())
	);
	if (BreakCode == -1) {
		if (nSelectedItem == -1) return true;	//esc
		if (nSelectedItem >= 0) return true;	//enter
	}
	m_bFilterFullUpdateMode = true;
	if (BreakCode < static_cast<int>(num_custom_break_codes)) return true; //������ �������� � m_Menu.GetBreakKeys �������
	if (VK_BACK == LOWORD(buf[BreakCode]))
	{	//������� ��������� ������ �������
		if (m_Filter.size()) m_Filter.erase(m_Filter.size()-1, 1);
	} else {
		wchar_t ch; //!DO: ������� ����� ��� ������ ��� �� �������� ����������� � �������, TODO
		if (decode_additional_character(buf[BreakCode], ch)) {
			m_Filter += ch;
		} else if (HIWORD(buf[BreakCode]) == PKF_SHIFT) {	//������� �����
			wchar_t ch[2]; 
			ch[0] = static_cast<wchar_t>(LOWORD(buf[BreakCode]));
			ch[1] = 0;
			g_FSF.XLat(&ch[0], 0, 1, 0);
			m_Filter += &ch[0];	
		} else { //���������� �����
			m_Filter += buf[BreakCode];
		}
		m_bFilterFullUpdateMode = false;			
	}

	return false;
}

bool nf::Menu::CMenuDialog::ShowMenu(tvariant_value &DestValue, int &DestRetCode) {
	if (! m_List.size()) return 0;	//there is no suitable item 

	if ( (! (m_Menu.m_Flags & tmenu::FG_SHOW_SINGLE_VARIANT)) && (1 == m_List.size())) {	//there is a single variant - menu is not required
		DestValue = m_List.begin()->second;
		DestRetCode = 1;		 
		return true;
	}

	while (true) {
		tlist_far_menu_items menu_items;
		tlist_far_menu_buffers menu_buffers;
		load_items(menu_items, menu_buffers);

		int break_code = 0;
		int nselected_item = 0;
		if (! show_menu(menu_items, break_code, nselected_item)) continue;	//filter key is pressed

		if (-1 == break_code && -1 == nselected_item) return false;	//user has canceled menu
	
		tvariant_value const*const pvalue = find_selected_item(nselected_item); //std::advance(p, nselected_item) is not suitable because some items can be invisible
		assert(pvalue != NULL);
		DestValue = *pvalue;

		if (-1 == break_code) {
			DestRetCode = 1; //menu item is selected
			return true;
		} else {
			DestRetCode = m_Menu.MakeAction(break_code); //which action should be made
		}
		if (DestRetCode < 0) return true; //action should be made outside (i.e. delete selected shortcut)
	};
}

std::pair<size_t, size_t> nf::Menu::CMenuDialog::get_column_widths() {
	//���������� ������������ ������ ������� � ������� �������
	//� ������� ���������
	std::pair<size_t, size_t> widths = std::make_pair(0, 0);
	width_visitor v1(m_Menu.GetCurrentMenuMode());
	for (tlist_menu_items::const_iterator p = m_List.begin(); p != m_List.end(); ++p)
	{
		if (p->first >= 0)
		{	//��������� ������ ������� ��������
			std::pair<size_t, size_t> w = boost::apply_visitor(v1, p->second);
			if (w.first > widths.first) widths.first = w.first;
			if (w.second > widths.second) widths.second = w.second;
		}
	}
	if (widths.first > FIRST_COLUMN_MAX_WIDTH) widths.first = FIRST_COLUMN_MAX_WIDTH;
	return widths;
}

nf::Menu::tvariant_value const*const nf::Menu::CMenuDialog::find_selected_item( int nSelectedItem ) {
	//m_List.first contain indices of items in farmenu
	//find item with index nSelectedItem
	BOOST_FOREACH(tmenu_item const& mi, m_List) {
		if (mi.first == nSelectedItem) return &mi.second;
	}
	return NULL;
}

void nf::Menu::CMenuDialog::set_items_visibility(tstring const& Filter, int Level) {
	//����������, ����� �������� ����� ������ 
	//������ ��������� ��� ����������� ������, ����� ������ ������ �� ������ ������.
	menu_string_maker_visitor string_maker(m_Menu.GetCurrentMenuMode(), std::make_pair(1024, 1024));
	int nitems = 0;
	std::list<tstring> filters;
	Utils::SplitStringByRegex(Filter, filters, L" ");

	for (tlist_menu_items::iterator p = m_List.begin(); p != m_List.end(); ++p) {
		if ((! m_bFilterFullUpdateMode) && (p->first < 0) && (p->first > -(Level-1))) continue;
		if (filters.empty() || is_satisfy_to_filter(filters, boost::apply_visitor(string_maker, p->second))	) {
			p->first = nitems++;
		} else {
			p->first = -Level;
		}
	}
}

void nf::Menu::CMenuDialog::load_items(tlist_far_menu_items &destMenuItems, tlist_far_menu_buffers &destMenuBuffers) {
	//��������� � ���� ������ ������� ��������
	assert(destMenuItems.empty());
	destMenuItems.reserve(m_List.size());

	set_items_visibility(m_Filter, static_cast<int>(m_Filter.size()) );	//����������, ����� �������� �������
	std::pair<size_t, size_t> widths = get_column_widths();	//������� ������������ ������ ��������
	menu_string_maker_visitor string_maker(m_Menu.GetCurrentMenuMode(), widths);
	BOOST_FOREACH(tmenu_item const& mi, m_List) {
		if (mi.first >= 0) {
			append_farmenu_item(destMenuItems, destMenuBuffers, boost::apply_visitor(string_maker, mi.second));
		}
	}
}

void nf::Menu::CMenuDialog::append_farmenu_item(tlist_far_menu_items &destMenuItems, tlist_far_menu_buffers &destMenuBuffers, tstring const& Value) {
	FarMenuItem m;
	m.Checked = 0;
	m.Selected = 0;
	m.Separator = 0;
	boost::shared_ptr<tstring> buffer;
	buffer.reset(new tstring(Value));
	if (buffer->size() > MAX_WIDTH) {
		buffer->erase(MAX_WIDTH, buffer->size() - MAX_WIDTH);
	} 
	m.Text = &(*buffer)[0];
	destMenuItems.push_back(m);
	destMenuBuffers.push_back(buffer);
	assert(destMenuItems.size() == destMenuBuffers.size());
}
