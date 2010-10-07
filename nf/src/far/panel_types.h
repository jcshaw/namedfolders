/*
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once
#include <vector>
#include <boost/scoped_ptr.hpp>

#include "plugin.hpp"
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
			g_PluginInfo.Control(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, reinterpret_cast<LONG_PTR>(&pi));
			for (int i = 0; i < pi.ItemsNumber; ++i) {
				std::vector<unsigned char> buffer; //!TODO: use another buffer
				buffer.resize(g_PluginInfo.Control(PANEL_ACTIVE, FCTL_GETPANELITEM, i, NULL));
				PluginPanelItem *ppi = reinterpret_cast<PluginPanelItem*>(&buffer[0]);
				g_PluginInfo.Control(PANEL_ACTIVE, FCTL_GETPANELITEM, i, reinterpret_cast<LONG_PTR>(&buffer[0]));

				if (m_ItemName == tstring(ppi->FindData.lpwszFileName)) {
					bool bIsCatalog = (ppi->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0; 
					if (((m_ItemsType & FG_CATALOGS) && bIsCatalog) || 
						((m_ItemsType & FG_SHORTCUTS) && (!bIsCatalog))) {
							return i;
					}
				}
			}
			return 0;		
		}
	};

}
}
