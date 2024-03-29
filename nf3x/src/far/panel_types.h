/*
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once
#include <vector>

//#include "far2/plugin.hpp"
#include "PanelInfoWrap.h"

namespace nf {
namespace Panel {

	enum { 
		FG_SHORTCUTS = 0x1
		,FG_CATALOGS = 0x2
		, FG_ALL = FG_SHORTCUTS | FG_CATALOGS
		, FG_ANY = FG_ALL
	};

//implements lazy search of item position on the panel
	class CPanelItemFinder { 
		tstring m_ItemName;
		ULONG m_ItemsType;
	public:
		CPanelItemFinder(wchar_t const* ItemName, ULONG ItemsType = FG_ANY)
			: m_ItemName(ItemName), m_ItemsType(ItemsType)
		{ }
		int operator()() {
			PanelInfo pi;
			pi.StructSize = sizeof(PanelInfo);

			g_PluginInfo.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, reinterpret_cast<void*>(&pi));
			for (unsigned int i = 0; i < pi.ItemsNumber; ++i) {
				nf::tautobuffer_byte buffer(g_PluginInfo.PanelControl(PANEL_ACTIVE, FCTL_GETPANELITEM, i, NULL));
				PluginPanelItem *ppi = reinterpret_cast<PluginPanelItem*>(&buffer[0]);
				if (ppi != nullptr) {
					FarGetPluginPanelItem fgppi = {sizeof(FarGetPluginPanelItem), buffer.size(), ppi};
					g_PluginInfo.PanelControl(PANEL_ACTIVE, FCTL_GETPANELITEM, i, &fgppi);
					tstring s = ppi->FileName;
					if (m_ItemName == s) {
						bool bIsCatalog = (ppi->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0; 
						if (((m_ItemsType & FG_CATALOGS) && bIsCatalog) || 
							((m_ItemsType & FG_SHORTCUTS) && (!bIsCatalog))) {
								return i;
						}
					}
				}

			}
			return 0;		
		}
	};

}
}
