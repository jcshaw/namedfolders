/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "confirmations.h"
#include "header.h"
#include "settings.h"

using namespace nf;
using namespace nf::Confirmations;

//запросить подтверждение у пользователя на создание псевдонима 
//с неявно заданным именем
UINT nf::Confirmations::AskForImplicitInsert(HANDLE hPlugin
											 , nf::tshortcut_info const&cmd
											 , tstring const& value)
{
	BOOL bIsImplicitConfirmationRequired = CSettings::GetInstance().GetValue(nf::ST_CONFIRM_IMPLICIT_CREATION);	
	if (! bIsImplicitConfirmationRequired) 
		return 1;	//подтверждений не требуется

	tstring sh_oem = Utils::GetInternalToOem(cmd.shortcut);
	tstring value_oem = Utils::GetInternalToOem(value);
	const char* Msg[4];
	Msg[0] = GetMsg(lg::MSG_INSERT_SHORTCUT);      
	Msg[1] = GetMsg(lg::CONFIRM_INSERT_SHORTCUT);   
	Msg[2] = sh_oem.c_str();
	Msg[3] = value_oem.c_str();

	if (g_PluginInfo.Message(g_PluginInfo.ModuleNumber
		, FMSG_MB_OKCANCEL
		, 0
		, Msg
		, sizeof(Msg)/sizeof(Msg[0])
		, 3) == 0) return 2;
	else return 0;
}

//запросить подтверждение у пользователя на создание псевдонима
UINT nf::Confirmations::AskForOverride(HANDLE hPlugin
									   , nf::tshortcut_info const&cmd
									   , tstring const& value)
{
	BOOL bIsOverrideConfirmationRequired = CSettings::GetInstance().GetValue(nf::ST_CONFIRM_OVERRIDE);	
	
	if (! bIsOverrideConfirmationRequired)
		return 1;	//подтверждений не требуется

	const char* Msg[5];
	tstring value_oem = Utils::GetInternalToOem(value);

	tstring s = GetMsg(lg::MSG_OVERRIDE_SHORTCUT);
	s += _T(": ");
	s += Utils::GetInternalToOem(cmd.shortcut);
	Msg[0] = s.c_str();      
	Msg[1] = GetMsg(lg::CONFIRM_OVERRIDE_SHORTCUT);   
	Msg[2] = value_oem.c_str();
	Msg[3] = GetMsg(lg::MSG_OVERRIDE);      
	Msg[4] = GetMsg(lg::MSG_CANCEL);


	if (g_PluginInfo.Message(g_PluginInfo.ModuleNumber
		, FMSG_WARNING 
		, 0
		, Msg
		, sizeof(Msg)/sizeof(Msg[0])
		, 2) == 0) return 2;
	else return 0;
}

UINT nf::Confirmations::AskToGoToNearest(HANDLE hPlugin
										 , TCHAR const* OriginalDirectory
										 , TCHAR const* NearestDirectory)
{
	BOOL bIsConfirmationRequired = CSettings::GetInstance().GetValue(nf::ST_CONFIRM_GO_TO_NEAREST);	
	if (! bIsConfirmationRequired) return 1;	//подтверждений не требуется

	const char* Msg[5];

	Msg[0] = GetMsg(lg::MSG_ERROR);   
	Msg[1] = GetMsg(lg::MSG_PATH_NOT_FOUND);
	Msg[2] = OriginalDirectory;
	Msg[3] = GetMsg(lg::MSG_JUMP_TO_THE_NEAREST_EXISTING_FOLDER);
	Msg[4] = NearestDirectory;

	if (g_PluginInfo.Message(g_PluginInfo.ModuleNumber
		, FMSG_WARNING | FMSG_MB_YESNO
		, 0
		, Msg
		, sizeof(Msg)/sizeof(Msg[0])
		, 2) == 0) return 2;
	else return 0;
}

//////////////////////////////////////////////////////////////////////////
nf::Confirmations::tconfirm_delete_result 
nf::Confirmations::Private::ask_for_delete_general(tstring const& Title, int Msg0, int Msg1, bool bSeveral)
{	
	const unsigned int number_buttons = bSeveral ? 4 : 2;
	char const* Msg[7];	//std::vector<const char*> Msg(bSeveral ? 7 : 5);
	Msg[0] = GetMsg(Msg0);
	Msg[1] = GetMsg(Msg1);
	Msg[2] = Title.c_str();
	Msg[3] = GetMsg(lg::MSG_DELETE);  
	if (bSeveral)
	{
		Msg[4] = GetMsg(lg::MSG_DELETE_ALL);
		Msg[5] = GetMsg(lg::MSG_SKIP);
		Msg[6] = GetMsg(lg::MSG_CANCEL);	
	} else {
		Msg[4] = GetMsg(lg::MSG_CANCEL);	
	}

	int code = g_PluginInfo.Message(g_PluginInfo.ModuleNumber
		, FMSG_WARNING 
		, 0
		, &Msg[0]
		, bSeveral ? 7 : 5
		, number_buttons);
	switch (code) 
	{
		case 0: return R_DELETE;//удалить		
		case 1: return R_DELETEALL ;//удалить все
		case 2: return R_SKIP;	//пропустить
		default: return R_CANCEL;	//отмена
	};
}

