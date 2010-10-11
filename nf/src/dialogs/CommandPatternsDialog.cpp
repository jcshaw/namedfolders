#include "stdafx.h"
#include ".\commandpatternsdialog.h"

#include <algorithm>
#include <boost/foreach.hpp>

#include "lang.h"
#include "settings.h"
#include "DialogEditPattern.h"
#include "confirmations.h"

using namespace nf;
using namespace Patterns;


namespace
{
	void LoadListPatternsToMenu(nf::Patterns::CommandsManager const &cm
		, nf::Patterns::tlist_command_patterns const &List
		, std::vector<FarMenuItem> &MenuItems)
	{
		MenuItems.resize(List.size());

		int i = 0;
		BOOST_FOREACH(nf::Patterns::tcommand_pattern const& pattern, List) {
			MenuItems[i].Text = pattern.first.c_str();
			MenuItems[i].Selected = 0;
			MenuItems[i].Checked = 0;
			MenuItems[i].Separator = 0;
			++i;
		}
	}
}

void nf::Patterns::EditPatterns(nf::Patterns::CommandsManager &cm)
{
	const int NUM_BREAK_KEYS = 4;
	static int BreakKeys[NUM_BREAK_KEYS+1] = {VK_F4, VK_INSERT, VK_DELETE, VK_ESCAPE, 0};	

	std::vector<FarMenuItem> MenuItems;

	while (true) {
		nf::Patterns::tlist_command_patterns list;
		cm.GetListRegisteredCommands(list);
		::LoadListPatternsToMenu(cm, list, MenuItems);

		int selected_key;
		int nSelectedItem = g_PluginInfo.Menu(g_PluginInfo.ModuleNumber	//!TODO: в этом меню фильтр сейчас не действует...
			, -1
			, -1
			, 0	//макс кол-во видимых элементов
			, FMENU_WRAPMODE | FMENU_SHOWAMPERSAND
			, nf::GetMsg(lg::CP_PATTERNS_DIALOG_TITLE)
			, L"Ins, F4, Del, Esc"
			, L"Patterns"
			, &BreakKeys[0]
			, &selected_key
			, &MenuItems[0]
			, static_cast<int>(MenuItems.size())
		);

		//find selected item
		nf::Patterns::tlist_command_patterns::const_iterator pSelectedItem = list.begin();
		std::advance(pSelectedItem, nSelectedItem);

		switch (selected_key)
		{
		case 1: //Insert
			{
				nf::Patterns::DialogEditPattern dlg;
				if (dlg.ShowModal())
				{
					//if (! cm.CheckIfPrefixIsFree(dlg.GetResultPrefix()))
					//!TODO: перезапись существующего префикса
					cm.SetCommand(dlg.GetResultPrefix(), dlg.GetResultPattern());				
				}
			}
			break;
		case 2: //Delete
			if (pSelectedItem != list.end())
			{
				if (nf::Confirmations::AskForDelete(pSelectedItem->first) == nf::Confirmations::R_DELETE)
					cm.RemoveCommand(pSelectedItem->first); //prefix
			}
			break;
		case 3: return;
		default: //Edit
			if (pSelectedItem != list.end())
			{
				nf::Patterns::DialogEditPattern dlg(pSelectedItem->first	//prefix
					, pSelectedItem->second //pattern
				);
				if (dlg.ShowModal())
				{
					if (pSelectedItem->first != dlg.GetResultPrefix())
					{
						cm.RemoveCommand(pSelectedItem->first);
					}
					//if (! cm.CheckIfPrefixIsFree(dlg.GetResultPrefix()))
					//!TODO: перезапись существующего префикса

					cm.SetCommand(dlg.GetResultPrefix(), dlg.GetResultPattern());
				}
			} //if 
			break;
		}
	}
}