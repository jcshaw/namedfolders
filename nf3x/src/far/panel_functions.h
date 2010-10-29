/*
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include "header.h"
#include "plugin.hpp"
#include "PanelInfoWrap.h"

namespace nf {
	namespace Panel {
		class CPanel;
	
		bool CreateCatalog(CPanel* pPanel, tstring const& ParentCatalog, tstring &CreatedCatalogName);
		BOOL MoveItems(CPanel* pPanel, PanelInfo const &pi, bool bCopy = false);
		inline BOOL CopyItems(CPanel* pPanel, PanelInfo const &pi) { 
			return MoveItems(pPanel, pi, true); }
		bool EditShortcut(CPanel* pPanel, nf::tshortcut_info const &sh, PanelInfo const &pi);
		bool InsertShortcut(CPanel* pPanel, PanelInfo const &pi);
		bool InsertShortcut(CPanel* pPanel, PanelInfo const &Pi, tshortcut_info const &Sh, tstring const &Value);
		void SaveSetup(CPanel* pPanel);
		void DeleteSelectedCatalogsAndShortcuts(CPanel *pPanel, PanelInfo const &pi);
		void UpdateCursorPositionOnFarPanel(CPanel *pPanel, PanelInfo const& pi);
		int OpenSelectedItem(CPanel *pPanel, unsigned int ControlState, PanelInfo const &pi);
	
	//helper functions
		bool IsSelectedItemIsCatalog(CPanel* pPanel, PanelInfo const &pi, int nSelectedItem = 0);
		tstring GetSelectedCatalog(CPanel* pPanel, PanelInfo const&pi, int nSelectedItem = 0);
		nf::tshortcut_info& GetSelectedShortcut(CPanel* pPanel, PanelInfo const &pi, nf::tshortcut_info& sh, int nSelectedItem = 0);

		inline PluginPanelItem* allocate_PluginPanelItem(CPanel const* pSrcPanel, int Command, int nSelectedItem) {
			HANDLE hplugin = static_cast<HANDLE>(const_cast<CPanel*>(pSrcPanel));
			PluginPanelItem* ppi = reinterpret_cast<PluginPanelItem*>(malloc(g_PluginInfo.Control(hplugin, Command, nSelectedItem, NULL)));

			g_PluginInfo.Control(hplugin, Command, nSelectedItem, reinterpret_cast<LONG_PTR>(ppi));
			return ppi;
		}

		inline void deallocate_PluginPanelItem(PluginPanelItem* p) {
			free(p);
		}

	}
}