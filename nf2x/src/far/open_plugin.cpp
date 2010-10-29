/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
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

extern const char* GetMsg(int MsgId);

namespace
{
	const int NUM_ITEMS = 3;
	const int PLUGINSMENU_ITEMS[NUM_ITEMS] = {	
		lg::PLUGINMENU_OPENPANEL
		, lg::PLUGINMENU_APPLYCOMMAND
		, lg::PLUGINMENU_PATTERNS
	};
}

HANDLE nf::OpenFromPluginsMenu()
{
	std::vector<FarMenuItem> MenuItems(NUM_ITEMS);
	for (unsigned int i = 0; i < NUM_ITEMS; ++i)
	{
		lstrcpy(MenuItems[i].Text, GetMsg(PLUGINSMENU_ITEMS[i]));
		MenuItems[i].Selected = 0;
		MenuItems[i].Checked = 0;
		MenuItems[i].Separator = 0;
	}

	int nSelectedItem = g_PluginInfo.Menu(g_PluginInfo.ModuleNumber
		, -1
		, -1
		, 0	//���� ���-�� ������� ���������
		, FMENU_AUTOHIGHLIGHT | FMENU_WRAPMODE
		, ""
		, ""
		, ""
		, 0
		, 0
		, &MenuItems[0]
		, static_cast<int>(MenuItems.size())
	);

	if (-1 == nSelectedItem) return INVALID_HANDLE_VALUE;	//user has canceled menu

		
	switch(nSelectedItem) {
	case 0:
		return new CPanel();
		break;
	case 1:
		{
			std::pair<HANDLE, bool> result(INVALID_HANDLE_VALUE, false);
			CDialogApplyCommand dlg;
			while (! result.second)
			{	//���� ������� ��������� ��������, ������ �� ���������
				if (dlg.ShowModal())
				{
					tstring command = dlg.GetCommandText();
					if (command.find_first_of(_T(":")) == tstring::npos)
					{
						command =  CSettings::GetInstance().GetPrimaryPluginPrefix() + command;
					}

					result = nf::OpenFromCommandLine(command.c_str());
				} else {
					result.second = true;	//������������ ����� cancel
				}
			}
		}
		break;
	case 2: //�������������� ������ ��������� 
		{
			nf::Patterns::CommandsManager cm(nf::GetRegistryKeyForCommandPatterns());
			nf::Patterns::EditPatterns(cm);
		}
		break;
	};

	return INVALID_HANDLE_VALUE;
}


std::pair<HANDLE, bool> nf::OpenFromCommandLine(TCHAR const* pSrcCmd)
{
	tstring command = Utils::GetOemToInternal(pSrcCmd);
	{
		//���������� ������� � ������� �������� ������
		nf::Patterns::CommandsManager cm(nf::GetRegistryKeyForCommandPatterns());
		tstring prefix = nf::Parser::ExtractPrefix(command);
		if (! cm.CheckIfPrefixIsFree(prefix))
		{	//��� �������� ������ ��������� �������
			cm.TransformCommandRecursively(command, command);
		}
	} 

	nf::tparsed_command cmd;
	//���������� ������ ������� ������������
	nf::Parser::ParseString(command, cmd);

	switch (cmd.kind)
	{
	case nf::QK_OPEN_PANEL:
		return std::make_pair(new nf::CPanel(), true);
		break;
	case nf::QK_OPEN_SHORTCUT:
		if (cmd.shortcut.empty())
		{
			return std::make_pair(new nf::CPanel(cmd.catalog), true);
			break;
		}
	default:
		return std::make_pair(INVALID_HANDLE_VALUE, nf::ExecuteCommand(cmd));
	};
	return std::make_pair(INVALID_HANDLE_VALUE, false);
} //OpenFromCommandLine