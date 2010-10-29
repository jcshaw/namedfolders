/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include "Kernel.h"
#include "codec_for_values.h"
#include "PanelInfoWrap.h"
#include "strings_utils.h"
#include "lang.h"

namespace nf {
//��������� ������������� �������� � ������������
//������������ ��������: 
//		0 - �������� ��������
//		1 - ������������� �������� ������������� � ����������
//		2 - ������������ ���� ���������� ��������
	namespace Confirmations
	{
		typedef enum tconfirm_delete_result {R_DELETE, R_DELETEALL, R_SKIP, R_CANCEL};

		UINT AskForOverride(HANDLE hPlugin, nf::tshortcut_info const&cmd, tstring const& value);
		UINT AskForImplicitInsert(HANDLE hPlugin, nf::tshortcut_info const&cmd, tstring const& value);
		UINT AskToGoToNearest(HANDLE hPlugin
			, TCHAR const* OriginalDirectory
			, TCHAR const* NearestDirectory);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//�������������� �� �������� ���������� ��� �������� ��� ���������� ����������� � ���������
		namespace Private
		{	//private �� ����� ����� ����������� inline
			nf::Confirmations::tconfirm_delete_result
				ask_for_delete_general(tstring const& Title, int Msg0, int Msg1, bool bSeveral);
		} //Private

		inline tconfirm_delete_result AskForDelete(nf::tshortcut_info const& sh, bool bSeveral)
		{	//�������� �����������
			return Private::ask_for_delete_general(Utils::GetInternalToOem(sh.shortcut)
				, lg::MSG_DELETE_SHORTCUT
				, lg::CONFIRM_DELETE_SHORTCUT
				, bSeveral);
		}
		
		inline tconfirm_delete_result AskForDelete(nf::tcatalog_info const& cat
			, bool bNotEmptyCatalogConfirmation //����� ������������, ��� ��������� �� ������ �������
			, bool bSeveral)	//������� ��������� ������ � ������� ���������� �/��� ������������
		{	//�������� ���������
			return Private::ask_for_delete_general(Utils::GetInternalToOem(cat)
				, bNotEmptyCatalogConfirmation ? lg::MSG_DELETE_CATALOG_NOT_EMPTY : lg::MSG_DELETE_CATALOG
				, lg::CONFIRM_DELETE_CATALOG
				, bSeveral);
		}

		inline tconfirm_delete_result AskForDelete(tstring const& Item)	
		{	//����� ������ - �������� ��������
			return Private::ask_for_delete_general(Utils::GetInternalToOem(Item)
				, lg::MSG_DELETE
				, lg::CONFIRM_DELETE
				, false);
		}

	} //Confirmations
};