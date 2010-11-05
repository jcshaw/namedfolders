#pragma once

//������� ������ Far.PanelInfo
class CPanelInfoWrap {
	HANDLE m_hPlugin;
	PanelInfo m_Pi;
public:
	CPanelInfoWrap(HANDLE hPlugin) : m_hPlugin(hPlugin) {}

	PanelInfo const& GetPanelInfo(bool bActivePanel) {
		g_PluginInfo.Control( (bActivePanel ? PANEL_ACTIVE : PANEL_PASSIVE)
			, FCTL_GETPANELINFO
			, 0
			, reinterpret_cast<LONG_PTR>(&m_Pi));
		return m_Pi;
	}

	tstring GetPanelCurDir(bool bActivePanel) {
		int buffer_size = g_PluginInfo.Control( (bActivePanel ? PANEL_ACTIVE : PANEL_PASSIVE), FCTL_GETPANELDIR, 0, 0); 
		nf::tautobuffer_char buffer(buffer_size);
		if (! g_PluginInfo.Control( (bActivePanel ? PANEL_ACTIVE : PANEL_PASSIVE)
			, FCTL_GETPANELDIR
			, buffer_size
			, reinterpret_cast<LONG_PTR>(&buffer[0])
		)) return L"";

		return &buffer[0];
	}

	void SetPanelDir(bool bActivePanel, tstring const& srcDir) {
		g_PluginInfo.Control( (bActivePanel ? PANEL_ACTIVE : PANEL_PASSIVE)
			, FCTL_SETPANELDIR
			, 0 
			, reinterpret_cast<LONG_PTR>(srcDir.c_str()));
	}

	void RedrawPanel(bool bActivePanel, PanelRedrawInfo *pri = 0) {
		g_PluginInfo.Control( (bActivePanel ? PANEL_ACTIVE : PANEL_PASSIVE)
			, FCTL_REDRAWPANEL
			, 0
			, reinterpret_cast<LONG_PTR>(pri));
	}

	void ClosePlugin(tstring const& srcDir) {
		g_PluginInfo.Control(m_hPlugin
			, FCTL_CLOSEPLUGIN
			, 0
			, reinterpret_cast<LONG_PTR>(srcDir.c_str()));
	}

	void UpdatePanel(bool bActivePanel) {
		g_PluginInfo.Control( (bActivePanel ? PANEL_ACTIVE : PANEL_PASSIVE)
			, FCTL_UPDATEPANEL 
			, 1 //don't remove selection; we can use any non-NULL pointer here
			, NULL
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