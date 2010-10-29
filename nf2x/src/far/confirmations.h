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
//запросить подтверждение операции у пользователя
//возвращаемые значения: 
//		0 - отменить операцию
//		1 - подтверждение отменено пользователем в настройках
//		2 - пользователь явно подтвердил операцию
	namespace Confirmations
	{
		typedef enum tconfirm_delete_result {R_DELETE, R_DELETEALL, R_SKIP, R_CANCEL};

		UINT AskForOverride(HANDLE hPlugin, nf::tshortcut_info const&cmd, tstring const& value);
		UINT AskForImplicitInsert(HANDLE hPlugin, nf::tshortcut_info const&cmd, tstring const& value);
		UINT AskToGoToNearest(HANDLE hPlugin
			, TCHAR const* OriginalDirectory
			, TCHAR const* NearestDirectory);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//предупреждения об удалении псевдонима или каталога или нескольких псевдонимов и каталогов
		namespace Private
		{	//private на нужен чтобы реализовать inline
			nf::Confirmations::tconfirm_delete_result
				ask_for_delete_general(tstring const& Title, int Msg0, int Msg1, bool bSeveral);
		} //Private

		inline tconfirm_delete_result AskForDelete(nf::tshortcut_info const& sh, bool bSeveral)
		{	//удаление псевдонимов
			return Private::ask_for_delete_general(Utils::GetInternalToOem(sh.shortcut)
				, lg::MSG_DELETE_SHORTCUT
				, lg::CONFIRM_DELETE_SHORTCUT
				, bSeveral);
		}
		
		inline tconfirm_delete_result AskForDelete(nf::tcatalog_info const& cat
			, bool bNotEmptyCatalogConfirmation //нужно предупредить, что удаляется не пустой каталог
			, bool bSeveral)	//каталог удаляется вместе с другими каталогами и/или псевдонимами
		{	//удаление каталогов
			return Private::ask_for_delete_general(Utils::GetInternalToOem(cat)
				, bNotEmptyCatalogConfirmation ? lg::MSG_DELETE_CATALOG_NOT_EMPTY : lg::MSG_DELETE_CATALOG
				, lg::CONFIRM_DELETE_CATALOG
				, bSeveral);
		}

		inline tconfirm_delete_result AskForDelete(tstring const& Item)	
		{	//общий случай - удаление элемента
			return Private::ask_for_delete_general(Utils::GetInternalToOem(Item)
				, lg::MSG_DELETE
				, lg::CONFIRM_DELETE
				, false);
		}

	} //Confirmations
};