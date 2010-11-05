/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "open_plugin.h"
#include "DialogApplyCommand.h"
#include "Panel.h"
#include "Parser.h"
#include "Executor.h"
#include "strings_utils.h"
#include "CommandPatternsDialog.h"

using namespace nf;

extern const wchar_t* GetMsg(int MsgId);

namespace {
	const int NUM_ITEMS = 3;
	const int PLUGINSMENU_ITEMS[NUM_ITEMS] = {	
		lg::PLUGINMENU_OPENPANEL
		, lg::PLUGINMENU_APPLYCOMMAND
		, lg::PLUGINMENU_PATTERNS
	};
}

HANDLE nf::OpenFromPluginsMenu() {
	std::vector<FarMenuItem> MenuItems(NUM_ITEMS);
	tlist_buffers menu_buffers(NUM_ITEMS);
	for (unsigned int i = 0; i < NUM_ITEMS; ++i) {
		menu_buffers[i].reset(Utils::Str2Buffer(GetMsg(PLUGINSMENU_ITEMS[i])));
		MenuItems[i].Text = &(*menu_buffers[i])[0];	
		MenuItems[i].Selected = 0;
		MenuItems[i].Checked = 0;
		MenuItems[i].Separator = 0;
	}

	int nSelectedItem = g_PluginInfo.Menu(g_PluginInfo.ModuleNumber
		, -1
		, -1
		, 0	//max count of visible items - regulated by FAR
		, FMENU_AUTOHIGHLIGHT | FMENU_WRAPMODE
		, L""
		, L""
		, L""
		, 0
		, 0
		, &MenuItems[0]
		, static_cast<int>(MenuItems.size())
	);

	if (-1 == nSelectedItem) return INVALID_HANDLE_VALUE;	//user has canceled menu
	
	switch(nSelectedItem) {
	case 0:
		return new nf::Panel::CPanel();
		break;
	case 1: {
			std::pair<HANDLE, bool> result(INVALID_HANDLE_VALUE, false);
			CDialogApplyCommand dlg;
			while (! result.second) {	//don't close dialog if command was performed unsuccessfully
				if (dlg.ShowModal()) {
					tstring command = dlg.GetCommandText();
					if (command.find_first_of(L":") == tstring::npos) {
						command =  CSettings::GetInstance().GetPrimaryPluginPrefix() + command;
					}
					result = nf::OpenFromCommandLine(command.c_str());
				} else {
					result.second = true;	//cancel is pressed
				}
			}
		} break;
	case 2: { //edit list of patterns
			nf::Patterns::CommandsManager cm(nf::GetRegistryKeyForCommandPatterns());
			nf::Patterns::EditPatterns(cm);
		} break;
	};

	return INVALID_HANDLE_VALUE;
}

namespace {
	tstring pattern_to_command(tstring const& srcCommand) {
		//decode srcCommand using patterns 
		nf::Patterns::CommandsManager cm(nf::GetRegistryKeyForCommandPatterns());
		tstring prefix = nf::Parser::ExtractPrefix(srcCommand);
		if (! cm.CheckIfPrefixIsFree(prefix)) {	//there is pattern srcCommand for the prefix 
			tstring dest;
			cm.TransformCommandRecursively(srcCommand, dest);
			return dest;
		} 
		return srcCommand;
	}
}

std::pair<HANDLE, bool> nf::OpenFromCommandLine(wchar_t const* pSrcCmd) {
	tstring command = pattern_to_command(pSrcCmd);

	nf::tparsed_command cmd;
	//распарсить строку запроса пользователя
	nf::Parser::ParseString(command, cmd);

	switch (cmd.kind) {
	case nf::QK_OPEN_PANEL:
		return std::make_pair(static_cast<HANDLE>(new nf::Panel::CPanel()), true);
		break;
	case nf::QK_OPEN_SHORTCUT:
		if (cmd.shortcut.empty()) {
			return std::make_pair(static_cast<HANDLE>(new nf::Panel::CPanel(cmd.catalog)), true);
			break;
		}
	default:
		return std::make_pair(INVALID_HANDLE_VALUE, nf::ExecuteCommand(cmd));
	};
	return std::make_pair(INVALID_HANDLE_VALUE, false);
} //OpenFromCommandLine