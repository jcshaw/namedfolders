/*
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include "header.h"
//#include "far2/plugin.hpp"
#include "PanelInfoWrap.h"

namespace nf {
	namespace Panel {
		class CPanel;

		typedef enum tcopy_mode {
			ID_CM_COPY
			, ID_CM_MOVE
			, ID_CM_RENAME
		};
	
		bool CreateCatalog(CPanel* pPanel, tstring const& ParentCatalog, tstring &CreatedCatalogName);
		bool EditCatalog(CPanel* pPanel, tstring const& CatalogName);
		BOOL MoveItems(CPanel* pPanel, PanelInfo const &pi, tcopy_mode copyMode);
		inline BOOL CopyItems(CPanel* pPanel, PanelInfo const &pi) { 
			return MoveItems(pPanel, pi, ID_CM_COPY); }
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

		inline PluginPanelItem* allocate_PluginPanelItem(HANDLE hPlugin, FILE_CONTROL_COMMANDS Command, int nSelectedItem) {
			PluginPanelItem* ppi = reinterpret_cast<PluginPanelItem*>(malloc(g_PluginInfo.PanelControl(hPlugin, Command, nSelectedItem, nullptr)));

			g_PluginInfo.PanelControl(hPlugin, Command, nSelectedItem, reinterpret_cast<void*>(ppi));
			return ppi;
		}
		inline PluginPanelItem* allocate_PluginPanelItem(CPanel const* pSrcPanel, FILE_CONTROL_COMMANDS Command, int nSelectedItem) {
			HANDLE hplugin = static_cast<HANDLE>(const_cast<CPanel*>(pSrcPanel));
			return allocate_PluginPanelItem(hplugin, Command, nSelectedItem);
		}

		inline void deallocate_PluginPanelItem(PluginPanelItem* p) {
			free(p);
		}

	}
}