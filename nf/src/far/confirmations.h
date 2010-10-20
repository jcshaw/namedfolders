/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include "Kernel.h"
#include "codec_for_values.h"
#include "PanelInfoWrap.h"
#include "strings_utils.h"
#include "lang.h"

namespace nf {
//ask user to confirm operation
//possible return values:
//		0 - cancel operation
//		1 - confirmation is canceled in settings
//		2 - user has confirmed the operation
	namespace Confirmations {
		typedef enum tconfirm_delete_result {R_DELETE, R_DELETEALL, R_SKIP, R_CANCEL};

		UINT AskForOverride(HANDLE hPlugin, nf::tshortcut_info const&cmd, tstring const& value);
		UINT AskForImplicitInsert(HANDLE hPlugin, nf::tshortcut_info const&cmd, tstring const& value);
		UINT AskToGoToNearest(HANDLE hPlugin, tstring const& srcDir, tstring const& nearestDir);
		UINT AskForCreateCatalog(HANDLE hPlugin, nf::tcatalog_info const&cat);

//confirmation to delete shortcut(s)/catalog(s)
		namespace Private {
			nf::Confirmations::tconfirm_delete_result
				ask_for_delete_general(tstring const& Title, int Msg0, int Msg1, bool bSeveral);
		}
//shortcuts
		inline tconfirm_delete_result AskForDelete(nf::tshortcut_info const& sh, bool bSeveral) {
			return Private::ask_for_delete_general(sh.shortcut
				, lg::MSG_DELETE_SHORTCUT
				, lg::CONFIRM_DELETE_SHORTCUT
				, bSeveral);
		}
//catalogs		
		inline tconfirm_delete_result AskForDelete(nf::tcatalog_info const& cat
			, bool bNotEmptyCatalogConfirmation //ask confirmation for deletion if catalog is not empty
			, bool bSeveral)	//delete catalog with other catalogs/shortcuts
		{	
			return Private::ask_for_delete_general(cat
				, bNotEmptyCatalogConfirmation ? lg::MSG_DELETE_CATALOG_NOT_EMPTY : lg::MSG_DELETE_CATALOG
				, lg::CONFIRM_DELETE_CATALOG
				, bSeveral);
		}
//general case: any items
		inline tconfirm_delete_result AskForDelete(tstring const& Item)	 {	
			return Private::ask_for_delete_general(Item
				, lg::MSG_DELETE
				, lg::CONFIRM_DELETE
				, false);
		}
	} 
};