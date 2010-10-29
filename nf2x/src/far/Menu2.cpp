#include "stdafx.h"
#include "menu2.h"
#include "menus_impl.h"

#include <list>
#include <boost/variant.hpp>
#include <boost/variant/get.hpp>
#include <boost/bind.hpp>

#include "strings_utils.h"
#include "stlcatalogs.h"

#include <Shlwapi.h>


using namespace nf;
using namespace Menu;

namespace
{
	inline void safe_copy(char *t, tstring const &SrcAnsi, UINT MaxSize)
	{	//скопировать строку в буфер, сжать до заданной ширины
		tstring src = Utils::GetInternalToOem(SrcAnsi);
		if (static_cast<UINT>(src.size()) > MaxSize)
		{
			PathCompactPathEx(t, src.c_str(), MaxSize, 0);
		} else 
			lstrcpy(t, src.c_str());
	}
}

namespace 
{
	typedef std::pair<tshortcut_info, tstring> tsh_info;
	typedef tshortcut_info tsoft_info;
	typedef std::pair<tstring, tstring> tenv_info;	
	typedef tstring tstr_info;

	//элементы в списке меню будем хранить в виде варианта
	typedef boost::variant<tsh_info, tenv_info, tsoft_info, tstr_info> tvariant_value;

	namespace Polices
	{	
		struct tsh
		{
			typedef tshortcut_info R;
			typedef tsh_info V;
			inline static V Encode(R const& SrcValue)
			{
				tstring value = _T(""); 
				sc::CCatalog c(SrcValue.catalog);
				c.GetShortcutInfo(SrcValue.shortcut, SrcValue.bIsTemporary, value);
				return V(boost::ref(SrcValue), value);
			}
			inline static R Decode(V const& SrcValue) { return SrcValue.first; }
		};

		template <class T>
		struct tdirect
		{
			typedef T R;
			typedef T V;			
			inline static V Encode(R const& SrcValue) { return SrcValue; }
			inline static R Decode(V const & SrcValue) { return SrcValue; }
		};

		struct tenv 
		{
			typedef tstring R;
			typedef tenv_info V;		
			inline static V Encode(R const& SrcValue) {	return static_cast<tenv_info>(Utils::DivideString(SrcValue, _T('%'))); }
			inline static R Decode(V const& SrcValue) { return Utils::CombinePath(SrcValue.first, SrcValue.second, _T("%"));	} //!TODO
		};
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
class menu_string_maker_visitor : public boost::static_visitor<tstring>
{	// формирование строк меню
	std::pair<size_t, size_t> const& m_Widths;
	int m_ViewMode;
public:
	menu_string_maker_visitor(int ViewMode, std::pair<size_t, size_t> const& W) : m_ViewMode(ViewMode), m_Widths(W){};

	tstring operator() (tsh_info const& Value) const
	{
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
		}; //switch
	}

	tstring operator() (tenv_info const& Value) const
	{
		switch(m_ViewMode) {
		case tenv_menu::MM_PATH: return Value.second;
		default: //tenv_menu::MM_VAR_PATH
			return Utils::CombineStrings(Value.first, Value.second, m_Widths.first);
		} //switch
	}

	inline tstring operator() (tsoft_info const& Value) const
	{	//добавить режимы отображения
		switch (m_ViewMode)
		{
		case tsoft_menu::MM_PATH: return Value.shortcut;
		default: return Utils::CombineStrings(Value.catalog, Value.shortcut, m_Widths.first);
		} //switch
	}

	inline tstring operator() (tstr_info const& Value) const { return Value; }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
class width_visitor : public boost::static_visitor<std::pair<size_t, size_t> >
{	//ширина первого и второго столбца
	int const m_ViewMode;
public:
	width_visitor(int ViewMode) : m_ViewMode(ViewMode) {}
	inline std::pair<size_t, size_t> operator() (tsh_info const& Value) const
	{
		tshortcut_info const& sh = Value.first;
		tstring const& value = Value.second;
		switch (m_ViewMode) {
		case tshortcuts_menu::MM_VAL: return std::make_pair(value.size(), 0);
		case tshortcuts_menu::MM_SH_VAL: return std::make_pair(sh.shortcut.size(),value.size());
		case tshortcuts_menu::MM_CAT_SH: 
		case tshortcuts_menu::MM_CAT_SH_VAL: return std::make_pair(sh.catalog.size(), sh.shortcut.size());
		default: return std::make_pair(sh.shortcut.size(), 0);
		}; //switch
	}

	inline std::pair<size_t, size_t> operator() (tenv_info const& Value) const
	{
		switch(m_ViewMode) {
		case tenv_menu::MM_PATH: return std::make_pair(Value.second.size(), 0);
		default: return std::make_pair(Value.first.size(), Value.second.size());
		}; //switch
	}

	inline std::pair<size_t, size_t> operator() (tsoft_info const& Value) const
	{	
		switch(m_ViewMode) {
		case tenv_menu::MM_PATH: return std::make_pair(Value.catalog.size(), 0);
		default: return std::make_pair(Value.catalog.size(), Value.shortcut.size());
		}; //switch		
	}

	inline std::pair<size_t, size_t> operator() (tstr_info const& Value) const { return std::make_pair(Value.size(), 0); }
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
namespace
{
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
	TCHAR additional_chars_translation[number_additional_chars] = {'+'
		, ';'
		, 'Ж'
		, 'Б'
		, '-'
		, 'Ю'
		, 'Э'
		, '['
		, ']'
		, 'Х'
		, 'Ъ'
	};
	bool decode_additional_character(int KeyCode, TCHAR &ResultChar)
	{
		const int len = sizeof(additional_chars);
		for (int i = 0; i < number_additional_chars; ++i)
		{
			if (additional_chars[i] == KeyCode)
			{
				ResultChar = additional_chars_translation[i];
				return true;
			}
		}
		return false;
	}
}
class CMenuDialog
{
	enum {FIRST_COLUMN_MAX_WIDTH=50, MAX_WIDTH = 128};
public:
	typedef std::pair<int, tvariant_value> tmenu_item;	//first = номер элемента в tlist_far_menu_items
	typedef std::vector<tmenu_item> tlist_menu_items;
	typedef std::vector<FarMenuItem> tlist_far_menu_items;

	CMenuDialog(tmenu &M, tlist_menu_items &List) 
		: m_Menu(M)
		, m_List(List)
		, m_bFilterFullUpdateMode(true) {}
	
	bool ShowMenu(tvariant_value &DestValue, int &DestRetCode)
	{
		if (! m_List.size()) return 0;	//нет подходящего элемента

		if ( (! (m_Menu.m_Flags & tmenu::FG_SHOW_SINGLE_VARIANT)) && (1 == m_List.size()))
    	{	//единственный вариант - показывать меню не требуется..
			DestValue = m_List.begin()->second;
			DestRetCode = 1;		 
			return true;
		}

		while (true)
		{
			tlist_far_menu_items menu_items;
			load_items(menu_items);

			int break_code = 0;
			int nselected_item = 0;
			if (! show_menu(menu_items, break_code, nselected_item)) continue;	//нажата клавиша фильтра

			if (-1 == break_code && -1 == nselected_item) return false;	//user has canceled menu
			//tlist_menu_items::const_iterator p = m_List.begin();
			//std::advance(p, nselected_item); //advance не подходит,т.к. не все элементы могут быть видимы
			tlist_menu_items::const_iterator p = find_selected_item(nselected_item);
			DestValue = p->second;

			if (-1 == break_code) {
				DestRetCode = 1; //пользователь выбрал элемент меню
				return true;
			}
			else DestRetCode = m_Menu.MakeAction(break_code);//, ResultValue);
			if (DestRetCode < 0) return true; //команда "вовне" (например, удалить выбранный псевдоним)
		};
	}
private:
	tlist_menu_items::const_iterator find_selected_item(int nSelectedItem)
	{	//в m_List в first занесены индексы элементов в farmenu
		//находим элемент соответствующий индексу nSelectedItem
		tlist_menu_items::const_iterator p = m_List.begin();
		while (p != m_List.end())
		{
			if (p->first == nSelectedItem) break;
			++p;
		}
		return p;
	}
	std::pair<size_t, size_t> get_column_widths()
	{	//определяем максимальную ширину первого и второго столбца
		//у видимых элементов
		std::pair<size_t, size_t> widths = std::make_pair(0, 0);
		width_visitor v1(m_Menu.GetCurrentMenuMode());
		for (tlist_menu_items::const_iterator p = m_List.begin(); p != m_List.end(); ++p)
		{
			if (p->first >= 0)
			{	//учитываем только видимые элементы
				std::pair<size_t, size_t> w = boost::apply_visitor(v1, p->second);
				if (w.first > widths.first) widths.first = w.first;
				if (w.second > widths.second) widths.second = w.second;
			}
		}
		if (widths.first > FIRST_COLUMN_MAX_WIDTH) widths.first = FIRST_COLUMN_MAX_WIDTH;
		return widths;
	}
	inline bool is_satisfy_to_filter(std::list<tstring> const& Filter
		, tstring SrcStr	//константая ссылка здесь не проходит, в релизи висим..
	)
	{	// Строка SrcStr удовлетворяем всем элементам фильтра
		return std::find_if(Filter.begin(), Filter.end()
			, boost::bind(std::logical_not<bool>(), boost::bind<bool>(Utils::iFindFirst, SrcStr, _1))) == Filter.end();
	}

	void set_items_visibility(tstring const& Filter, int Level)
	{	//определяем, какие элементы будут видимы 
		//строки формируем без ограничения ширины, чтобы искать фильтр по полной строке.
		menu_string_maker_visitor string_maker(m_Menu.GetCurrentMenuMode(), std::make_pair(1024, 1024));
		int nitems = 0;
		std::list<tstring> filters;
		Utils::SplitStringByRegex(Filter, filters, _T(" "));

		for (tlist_menu_items::iterator p = m_List.begin(); p != m_List.end(); ++p)
		{
			if ((! m_bFilterFullUpdateMode) && (p->first < 0) && (p->first > -(Level-1))) continue;
			if (filters.empty() || is_satisfy_to_filter(filters, boost::apply_visitor(string_maker, p->second))
			)
			{
				p->first = nitems++;
			} else {
				p->first = -Level;
			}
		}
	}

	void load_items(tlist_far_menu_items &menu_items)	//!TODO: заменить вектор массивом
	{	//загружаем в меню только видимые элементы
		assert(menu_items.empty());
		menu_items.reserve(m_List.size());
			
		set_items_visibility(m_Filter, static_cast<int>(m_Filter.size()) );	//определяем, какие элементы видимые
		std::pair<size_t, size_t> widths = get_column_widths();	//находим максимальную ширину столбцов
		menu_string_maker_visitor string_maker(m_Menu.GetCurrentMenuMode(), widths);
		for (tlist_menu_items::iterator p = m_List.begin(); p != m_List.end(); ++p)
		{
			if (p->first >= 0) append_menu_item(menu_items, boost::apply_visitor(string_maker, p->second));
		}
	}
	inline void append_menu_item(tlist_far_menu_items &DestListFarItems, tstring const& Value)
	{
		FarMenuItem m;
		m.Checked = 0;
		m.Selected = 0;
		m.Separator = 0;
		safe_copy(m.Text, Value, MAX_WIDTH);	
		DestListFarItems.push_back(m);
	}

	int fill_menu_break_keys_buf(const int BufSize, int *buf, int &DestNumDefaultBreakKeys)
	{
		int nbuf_count = 0;
		int *pbreak_codes = m_Menu.GetBreakKeys();
		while (0 != pbreak_codes && 0 != *pbreak_codes) 
		{
			buf[nbuf_count++] = *pbreak_codes;
			++pbreak_codes;
			assert(nbuf_count < BufSize);
		}
		DestNumDefaultBreakKeys = nbuf_count;
		const char first_letter = 'A';
		const char last_letter = 'Z';
		assert(BufSize > nbuf_count + (2*(last_letter - first_letter) + 1) + 1 + 10);

		buf[nbuf_count++] = VK_BACK; //стирание, 1 символ
		buf[nbuf_count++] = VK_SPACE; //пробел, 1 символ
		buf[nbuf_count++] = MAKELONG(VK_BACK, PKF_SHIFT); //стирание, 1 символ
		buf[nbuf_count++] = MAKELONG(VK_SPACE, PKF_SHIFT); //пробел, 1 символ
		
		for (unsigned int i = first_letter; i <= last_letter; ++i) 
		{
			buf[nbuf_count++] = i;  //английские буквы
			buf[nbuf_count++] = MAKELONG(i, PKF_SHIFT);  //русские буквы (с шифтом)
		}
		for (unsigned int i = '0'; i <= '9'; ++i) buf[nbuf_count++] = i;	 //цифры, 10 символов

		for (unsigned int i = 0; i < number_additional_chars; ++i)
		{
			buf[nbuf_count++] = additional_chars[i];
		}
		buf[nbuf_count++] = 0;

		return nbuf_count;
	}

	int show_menu(tlist_far_menu_items const& MenuItems, int& BreakCode, int &nSelectedItem)
	{
		const int MAX_SIZE = 512;	//!TODO: лишний расход памяти
		int buf[MAX_SIZE];
		int num_custom_break_codes = 0;
		int num_break_keys = fill_menu_break_keys_buf(MAX_SIZE, buf, num_custom_break_codes);

		tstring title = m_Menu.GetMenuTitle();
		if (m_Filter.size()) 
		{
			if (title.size()) title += _T(": ");
			title += Utils::GetInternalToOem(m_Filter);
		}
		nSelectedItem = g_PluginInfo.Menu (
			g_PluginInfo.ModuleNumber
			, -1
			, -1
			, 0	//макс кол-во видимых элементов
			, FMENU_WRAPMODE | FMENU_SHOWAMPERSAND
			, title.c_str()
			, m_Menu.m_KeysInMenu.c_str()
			, m_Menu.m_HelpTopic.c_str()
			, &buf[0]
			, &BreakCode
			, &MenuItems[0]
			, static_cast<int>(MenuItems.size())
		);
		if (nSelectedItem == -1 && BreakCode == -1) return true;	//esc
		if (nSelectedItem >= 0 && BreakCode == -1) return true;	//enter

		m_bFilterFullUpdateMode = true;
		if (BreakCode < static_cast<int>(num_custom_break_codes)) return true; //нажата заданная в m_Menu.GetBreakKeys клавиша
		if (VK_BACK == LOWORD(buf[BreakCode]))
		{	//удаляем последний символ фильтра
			if (m_Filter.size()) m_Filter.erase(m_Filter.size()-1, 1);
		} else {
			TCHAR ch;
			if (decode_additional_character(buf[BreakCode], ch))
			{
				m_Filter += ch;
			} else if (HIWORD(buf[BreakCode]) == PKF_SHIFT)
			{	//русская буква
				char ch[2]; 
				ch[0] = static_cast<char>(LOWORD(buf[BreakCode]));
				ch[1] = 0;
				g_FSF.XLat(&ch[0], 0, 1, 0, 0);
				m_Filter += Utils::GetOemToInternal(&ch[0]);	
			} else { //английская буква
				m_Filter += buf[BreakCode];
			}
			m_bFilterFullUpdateMode = false;			
 		}

		return false;
	}
private:
	tmenu &m_Menu;
	tlist_menu_items &m_List;
	tstring m_Filter;	//введенный фильтр (только английские буквы)
	bool m_bFilterFullUpdateMode; 
		//при добавлении очередной буквы к фильтру нет необходимости проверять
		//строки, которые не попали в результаты предыдущего поиска
		//т.е. здесь возможна оптимизация, которая и реализуется через этот флаг
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
namespace 
{
	template <class TMenu, class TList, class Policy> inline 
		int select_from_menu(TList const &ListItems, typename Policy::R &DestValue)
	{
		CMenuDialog::tlist_menu_items list;
		list.reserve(ListItems.size());

		for (typename TList::const_iterator p = ListItems.begin(); p != ListItems.end(); ++p)
		{
			list.push_back(std::make_pair(-1, tvariant_value(Policy::Encode(*p))));	//-1 - в список элементов farmenu не добавлен
		}

		TMenu menu;
		CMenuDialog dlg(menu, list);

		int result_code;
		tvariant_value result_value;
		if (dlg.ShowMenu(result_value, result_code))
		{
			DestValue = Policy::Decode(boost::get<typename Policy::V>(result_value));
			return result_code;
		}
		return 0;
	}

}

int nf::Menu::SelectShortcut(nf::tshortcuts_list const &SrcList, nf::tshortcut_info& DestSh)
{
	return select_from_menu<tshortcuts_menu, tshortcuts_list, Polices::tsh>(SrcList, DestSh);
}

int nf::Menu::SelectSoft(nf::tshortcuts_list const &SrcList, nf::tshortcut_info& DestSoft)
{
	return select_from_menu<tsoft_menu, tshortcuts_list, Polices::tdirect<nf::tshortcut_info> >(SrcList, DestSoft);
}

int nf::Menu::SelectCatalog(nf::tcatalogs_list const &SrcList, nf::tcatalog_info& DestCatalog)
{
	return select_from_menu<tcatalogs_menu, tcatalogs_list, Polices::tdirect<tstring> >(SrcList, DestCatalog);
}

int nf::Menu::SelectPath(nf::tdirs_list const& SrcList, tstring &DestPath)
{
	return select_from_menu<tpath_menu, tdirs_list, Polices::tdirect<tstring> >(SrcList, DestPath);
}

int nf::Menu::SelectPath(std::list<tstring> const& SrcList, tstring &DestPath)
{
	return select_from_menu<tpath_menu, std::list<tstring>, Polices::tdirect<tstring> >(SrcList, DestPath);
}

int nf::Menu::SelectEnvVar(std::list<std::pair<tstring, tstring> > const &SrcList
						   , std::pair<tstring, tstring> &DestVar)
{
	return select_from_menu<tenv_menu
		, std::list<std::pair<tstring, tstring> >
		, Polices::tdirect<std::pair<tstring, tstring> > 
	>(SrcList, DestVar);
}

