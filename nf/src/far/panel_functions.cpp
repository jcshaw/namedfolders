/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/

#include "stdafx.h"
#include "panel_functions.h"

#include <boost/scope_exit.hpp>

#include "Kernel.h"
#include "PanelUpdater.h"
#include "Panel.h"

#include "DialogEditCatalog.h"
#include "DialogEditShortcut.h"
#include "DialogMove.h"

#include "executor/Executor.h"
#include "catalog_names.h"


namespace nf {
	namespace Panel {
		tstring get_inactive_panel_current_catalog(nf::Panel::CPanel *pPanel) {
			tstring s = pPanel->get_hPlugin().GetPanelCurDir(false);
			if (s.find_first_of(L":") != tstring::npos) return L"";
			else return s;
		}
		void get_selected_shortcut_info(CPanel *pPanel, nf::tshortcut_info &sh) {
			PanelInfo const &pi = pPanel->get_hPlugin().GetPanelInfo(true);
			assert(! IsSelectedItemIsCatalog(pPanel, pi));

			PluginPanelItem *ppi = allocate_PluginPanelItem(pPanel, FCTL_GETSELECTEDPANELITEM, 0);
			BOOST_SCOPE_EXIT( (&ppi) ) {
				deallocate_PluginPanelItem(ppi);
			} BOOST_SCOPE_EXIT_END

				sh = nf::MakeShortcut(pPanel->GetCurrentCatalog(), ppi->FindData.lpwszFileName
				, ppi->FindData.dwFileAttributes && FILE_ATTRIBUTE_TEMPORARY);
		}
	}
}

BOOL nf::Panel::MoveItems(CPanel* pPanel, PanelInfo const &pi, bool bCopy) {
	bool bSeveralCatalogs = 1 != pi.SelectedItemsNumber;
	bool bFirstItemIsCatalog = IsSelectedItemIsCatalog(pPanel, pi, 0);
	tstring name;
	if (bFirstItemIsCatalog) {
		name = GetSelectedCatalog(pPanel, pi, 0);
	} else {
		nf::tshortcut_info sh;
		GetSelectedShortcut(pPanel, pi, sh, 0);
		name = sh.catalog + tstring(SLASH_CATS) + sh.shortcut;			
	};

	//куда копируем? проверяем, не открыта ли директория на другой панели...
	tstring default_path = get_inactive_panel_current_catalog(pPanel);
	bool bPluginOpenedOnInactivePanel = ! default_path.empty();

	nf::CDialogMove dlg(name.c_str()
		, default_path
		, bFirstItemIsCatalog
		, bSeveralCatalogs
		, bCopy);
	if (dlg.ShowModal()) {
		CPanelUpdater pu(pPanel, pi.CurrentItem);
		bool bSingleItem = pi.SelectedItemsNumber == 1;

		bool bRet = false;
		for (int i = 0; i < pi.SelectedItemsNumber; ++i) {
			tstring new_path = dlg.GetItemName();

			//!TODO: разобраться; добавлять слеш тут просто так нельзя, иначе нарушается работа ExpandCatalogPath
// 			//плагин предлагает переименовать "/11", указываем "22" получаем каталог, вместо псевдонима.
// 			if (! new_path.empty() && new_path[0] != SLASH_CATS_CHAR) {
// 				new_path = SLASH_CATS_CHAR + new_path;
// 			}

			if (IsSelectedItemIsCatalog(pPanel, pi, i)) {
				tstring new_catalog_name;
				tstring catalog_name = GetSelectedCatalog(pPanel, pi, i); 
				if (bCopy) {
					bRet |= Shell::CopyCatalog(catalog_name.c_str(), new_path);
				} else {
					bRet |= Shell::MoveCatalog(catalog_name.c_str(), new_path);
				}
				if (bSingleItem) {
					//если был переименован только один элемент и каталог его не изменился, то
					//позиционируем курсор на этом элементе.
					if (catalog_name == new_path) 
						pu.SetCursorOnItem(new_catalog_name, FG_CATALOGS);
					//!!!! сравнивать только имена каталогов, не полные пути
				}
			} else {
				nf::tshortcut_info new_sh;
				nf::tshortcut_info sh;
				GetSelectedShortcut(pPanel, pi, sh, i);
				if (bCopy) { 
					bRet |= Shell::CopyShortcut(sh, new_path, new_sh);
				} else {
					bRet |= Shell::MoveShortcut(sh, new_path, new_sh);
				}
				if (bSingleItem) {
					//если был переименован только один элемент и каталог его не изменился, то
					//позиционируем курсор на этом элементе.
					if (sh.catalog == new_sh.catalog) pu.SetCursorOnItem(new_sh.shortcut, FG_SHORTCUTS);
				}
			}
		}

		pu.UpdateActivePanel();
		pu.UpdateInactivePanel();

		return static_cast<BOOL>(bRet);
	}

	return FALSE;
}

bool nf::Panel::EditShortcut(CPanel* pPanel, nf::tshortcut_info const &shortcut_name, PanelInfo const &pi) {
	nf::tshortcut_info sh;
	get_selected_shortcut_info(pPanel, sh);
	tstring value;
	Shell::GetShortcutValue(sh, value);
	nf::tshortcut_value_parsed_pair vp = nf::DecodeValues(value);

	nf::CDialogEditShortcut dlg(sh, vp.first.value, vp.second.value, false);
	if (dlg.ShowModal()) {
		CPanelUpdater pu(pPanel, pi.CurrentItem);
		nf::tshortcut_info const& sh2 = dlg.GetShortcutInfo();
		tstring value2 = nf::EncodeValues(dlg.GetValueForActivePanel(), dlg.GetValueForInActivePanel());
		if (Shell::ModifyShortcut(sh, sh2, &value2)) {
			if (sh.catalog == sh2.catalog) pu.SetCursorOnItem(sh2.shortcut, FG_SHORTCUTS);
			pu.UpdateActivePanel();
			return true;
		}
	}
	return false;
}

bool nf::Panel::InsertShortcut(CPanel* pPanel, PanelInfo const &pi) {
	nf::tshortcut_info default_sh;
	nf::tshortcut_value_parsed apv;

	std::pair<tstring, tstring> pair_name_value = nf::Commands::get_implicit_name_and_value(pPanel->get_hPlugin(), false);
	default_sh.shortcut.swap(pair_name_value.first);
	apv.value.swap(pair_name_value.second);

	default_sh.bIsTemporary = false;
	default_sh.catalog = pPanel->GetCurrentCatalog();

	return InsertShortcut(pPanel, pi, default_sh, apv.value);
}

bool nf::Panel::InsertShortcut(CPanel* pPanel, PanelInfo const &pi, tshortcut_info const &Sh, tstring const &Value) {
	nf::CDialogEditShortcut dlg(Sh, Value, tstring(), true);
	if (dlg.ShowModal()) {
		CPanelUpdater pu(pPanel, pi.CurrentItem);
		nf::tshortcut_info const& sh = dlg.GetShortcutInfo();
		tstring value = nf::EncodeValues(dlg.GetValueForActivePanel(), dlg.GetValueForInActivePanel());

		if (nf::Commands::AddShortcut(pPanel->get_hPlugin(), sh, value.c_str(), false)) {
			//делаем update два раза, чтобы корректно позиционироваться на вновь созданный псевдоним
			pu.UpdateActivePanel();	//!TODO
			pu.SetCursorOnItem(sh.shortcut, FG_SHORTCUTS);
			pu.UpdateActivePanel();	
			return true;	
		}
	}
	return false;
}


bool nf::Panel::CreateCatalog(CPanel* pPanel, tstring const& ParentCatalog, tstring &CreatedCatalogName) {
	nf::CDialogEditCatalog dlg(ParentCatalog.c_str(), L"", true);
	if (dlg.ShowModal()) {
		CreatedCatalogName = dlg.GetCatalogName(true);
		return nf::Commands::AddCatalog(pPanel->get_hPlugin(), CreatedCatalogName);
	}
	return false;
}

void nf::Panel::SaveSetup(CPanel* pPanel) {
	//запоминаем состояние панели
	int ViewMode = pPanel->get_hPlugin().GetPanelInfo(true).ViewMode;
	const wchar_t *Msg = GetMsg(lg::MSG_SAVE_SETUP_MESSAGE);      

	if (g_PluginInfo.Message(g_PluginInfo.ModuleNumber
		, FMSG_MB_OKCANCEL | FMSG_ALLINONE
		, 0
		, (wchar_t const * const*) Msg
		, 0, 0) == 0) 
	{
		CSettings::GetInstance().SetValue(nf::ST_PANEL_MODE, ViewMode);	
		CSettings::GetInstance().SaveSettings();
	}
}

bool nf::Panel::IsSelectedItemIsCatalog(CPanel* pPanel, PanelInfo const &pi, int nSelectedItem) {
	PluginPanelItem *ppi = allocate_PluginPanelItem(pPanel, FCTL_GETSELECTEDPANELITEM, nSelectedItem);
	BOOST_SCOPE_EXIT( (&ppi) ) {
		deallocate_PluginPanelItem(ppi);
	} BOOST_SCOPE_EXIT_END
	return (ppi->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

tstring nf::Panel::GetSelectedCatalog(CPanel* pPanel, PanelInfo const&pi, int nSelectedItem) {
	PluginPanelItem *ppi = allocate_PluginPanelItem(pPanel, FCTL_GETSELECTEDPANELITEM, nSelectedItem);
	BOOST_SCOPE_EXIT( (&ppi) ) {
		deallocate_PluginPanelItem(ppi);
	} BOOST_SCOPE_EXIT_END

	return Utils::CombinePath(pPanel->GetCurrentCatalog()
		, tstring(ppi->FindData.lpwszFileName)
		, SLASH_CATS);
}

nf::tshortcut_info& nf::Panel::GetSelectedShortcut(CPanel* pPanel, PanelInfo const &pi, nf::tshortcut_info& sh, int nSelectedItem) {
	PluginPanelItem *ppi = allocate_PluginPanelItem(pPanel, FCTL_GETSELECTEDPANELITEM, nSelectedItem);
	BOOST_SCOPE_EXIT( (&ppi) ) {
		deallocate_PluginPanelItem(ppi);
	} BOOST_SCOPE_EXIT_END

	sh.catalog = pPanel->GetCurrentCatalog();
	sh.shortcut = ppi->FindData.lpwszFileName;
	sh.bIsTemporary = 0 != ppi->CRC32;
	return sh;
}

void nf::Panel::DeleteSelectedCatalogsAndShortcuts(CPanel *pPanel, PanelInfo const &pi) {
	if (! pi.SelectedItemsNumber) return;	// ".."

	CPanelUpdater pu(pPanel, pi.CurrentItem);		
	//составляем полный список выделенных псевдонимов и каталогов
	std::list<nf::tshortcut_info> list_sh;
	std::list<nf::tcatalog_info> list_c;
	for (int i = 0; i < pi.SelectedItemsNumber; ++i) {
		if (IsSelectedItemIsCatalog(pPanel, pi, i)) {
			list_c.push_back(GetSelectedCatalog(pPanel, pi, i));
		} else {
			nf::tshortcut_info sh;
			list_sh.push_back(GetSelectedShortcut(pPanel, pi, sh, i));
		}
	}
	//удаляем их и, если хотя бы один из них был удален, обновляем панель
	if (nf::Commands::DeleteCatalogsAndShortcuts(list_sh, list_c, false)) pu.UpdateActivePanel(); 
}

void nf::Panel::UpdateCursorPositionOnFarPanel(CPanel *pPanel, PanelInfo const& pi) {
	CPanelUpdater pu(pPanel);
	if (IsSelectedItemIsCatalog(pPanel, pi)) {
		nf::tcatalog_info c = GetSelectedCatalog(pPanel, pi);		
		pu.SetCursorOnItem(Utils::ExtractCatalogName(c), FG_CATALOGS);
	} else {
		nf::tshortcut_info sh;
		GetSelectedShortcut(pPanel, pi, sh);
		pu.SetCursorOnItem(Utils::ExtractCatalogName(sh.shortcut), FG_SHORTCUTS);
	}
	pu.UpdateActivePanel();
}

int nf::Panel::OpenSelectedItem(CPanel *pPanel, unsigned int ControlState, PanelInfo const &pi) {
	nf::tshortcut_info sh;
	if (PKF_SHIFT == ControlState) {	//shift -> open explorer
		if (! IsSelectedItemIsCatalog(pPanel, pi)) {
			nf::Commands::OpenShortcutInExplorer(pPanel->get_hPlugin()
				, GetSelectedShortcut(pPanel, pi, sh), tstring());
			return TRUE;	
		}
	}
	if (pi.SelectedItemsNumber != 0) {	 //avoid  ".."
		if (! IsSelectedItemIsCatalog(pPanel, pi)) { //open selected directory in FAR
			if (PKF_CONTROL != ControlState) {
				nf::Commands::OpenShortcut(pPanel->get_hPlugin()
					, GetSelectedShortcut(pPanel, pi, sh), tstring());
				return TRUE;	//itsn't necessary to update active panel
			} else {//на неактивной панели
				nf::Commands::OpenShortcut(pPanel->get_hPlugin()
					, GetSelectedShortcut(pPanel, pi, sh), tstring(), false);
				return TRUE;	//itsn't necessary to update active panel
			}
		} 
	}	
	return FALSE; //let's FAR open calalog itself
}