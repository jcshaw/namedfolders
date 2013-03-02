#pragma once

//#include "far2/plugin.hpp"
//#include "far2/farkeys.hpp"

//обертка вокруг Far.PanelInfo
class CPanelInfoWrap {
	HANDLE m_hPlugin;
	PanelInfo m_Pi;
public:
	CPanelInfoWrap(HANDLE hPlugin) : m_hPlugin(hPlugin) {}

	PanelInfo const& GetPanelInfo(bool bActivePanel) {
		g_PluginInfo.PanelControl( (bActivePanel ? PANEL_ACTIVE : PANEL_PASSIVE)
			, FCTL_GETPANELINFO
			, 0
			, reinterpret_cast<void*>(&m_Pi));
		return m_Pi;
	}

	tstring GetPanelCurDir(bool bActivePanel) {
		int buffer_size = g_PluginInfo.PanelControl( (bActivePanel ? PANEL_ACTIVE : PANEL_PASSIVE), FCTL_GETPANELDIRECTORY, 0, 0); 
		nf::tautobuffer_char buffer(buffer_size);
		if (! g_PluginInfo.PanelControl( (bActivePanel ? PANEL_ACTIVE : PANEL_PASSIVE)
			, FCTL_GETPANELDIRECTORY
			, buffer_size
			, reinterpret_cast<void*>(&buffer[0])
		)) return L"";
		return reinterpret_cast<FarPanelDirectory*>(&buffer[0])->File;
	}

	tstring GetPanelActiveFileName(bool bActivePanel) {
		int buffer_size = g_PluginInfo.PanelControl( (bActivePanel ? PANEL_ACTIVE : PANEL_PASSIVE), FCTL_GETCURRENTPANELITEM, 0, 0); 
		nf::tautobuffer_char buffer(buffer_size);
		FarGetPluginPanelItem fgppi = {sizeof(FarGetPluginPanelItem), buffer_size, reinterpret_cast<PluginPanelItem*>(&buffer[0])};

		PluginPanelItem ppi;
		memset(&ppi, 0, sizeof(PluginPanelItem));
		if (! g_PluginInfo.PanelControl( (bActivePanel ? PANEL_ACTIVE : PANEL_PASSIVE)
			, FCTL_GETCURRENTPANELITEM
			, 0
			, &fgppi
			)) return L"";

		return fgppi.Item->FileName;
	}

	inline void SetPanelDir(bool bActivePanel, tstring const& srcDir) {
		FarPanelDirectory fpd = {sizeof(FarPanelDirectory), srcDir.c_str(), NULL, {0}, NULL};
		g_PluginInfo.PanelControl( (bActivePanel ? PANEL_ACTIVE : PANEL_PASSIVE)
			, FCTL_SETPANELDIRECTORY
			, 0 
			, &fpd);
	}

	inline void RedrawPanel(bool bActivePanel, PanelRedrawInfo *pri = 0) {
		g_PluginInfo.PanelControl( (bActivePanel ? PANEL_ACTIVE : PANEL_PASSIVE)
			, FCTL_REDRAWPANEL
			, 0
			, reinterpret_cast<void*>(pri));
	}

	inline void ClosePlugin(tstring const& srcDir) {
		g_PluginInfo.PanelControl(m_hPlugin
			, FCTL_CLOSEPANEL
			, 0
			, reinterpret_cast<void*>(const_cast<LPTSTR>(srcDir.c_str())));
	}

	inline void UpdatePanel(bool bActivePanel) {
		g_PluginInfo.PanelControl( (bActivePanel ? PANEL_ACTIVE : PANEL_PASSIVE)
			, FCTL_UPDATEPANEL 
			, 1 //don't remove selection; we can use any non-NULL pointer here
			, 0
		);
	}

	void UpdateAndRedraw(bool bActivePanel, int nCurrentItem = 0, int nTopPanelItem = 0) {
		PanelRedrawInfo pri; 
		pri.CurrentItem = nCurrentItem;
		pri.TopPanelItem = nTopPanelItem; 
		UpdatePanel(bActivePanel);
		RedrawPanel(bActivePanel, &pri);
	}

	operator HANDLE() {return m_hPlugin;}
};
