#pragma once


//обертка вокруг Far.PanelInfo
class CPanelInfoWrap
{
	HANDLE m_hPlugin;
	PanelInfo m_Pi;
public:
	CPanelInfoWrap(HANDLE hPlugin) : m_hPlugin(hPlugin) {}

	PanelInfo const& GetPanelInfo(bool bActivePanel) 
	{
		g_PluginInfo.Control(m_hPlugin, 
			bActivePanel 
				? FCTL_GETPANELINFO
				: FCTL_GETANOTHERPANELINFO
			, &m_Pi);
		return m_Pi;
	}

	PanelInfo const& GetPanelShortInfo(bool bActivePanel) 
	{
		g_PluginInfo.Control(m_hPlugin, 
			bActivePanel 
			? FCTL_GETPANELSHORTINFO
			: FCTL_GETANOTHERPANELSHORTINFO
			, &m_Pi);
		return m_Pi;
	}

	void SetPanelDir(bool bActivePanel, tstring const& dir_oem)
	{
		g_PluginInfo.Control(m_hPlugin
			, bActivePanel
				? FCTL_SETPANELDIR
				: FCTL_SETANOTHERPANELDIR
			, (void*)dir_oem.c_str());
	}

	void RedrawPanel(bool bActivePanel, PanelRedrawInfo *pri = 0)
	{
		g_PluginInfo.Control(m_hPlugin
			, bActivePanel
				? FCTL_REDRAWPANEL
				: FCTL_REDRAWANOTHERPANEL
			, pri);
	}

	void ClosePlugin(tstring const& dir_oem)
	{
		g_PluginInfo.Control(m_hPlugin
			, FCTL_CLOSEPLUGIN
			, (void*)dir_oem.c_str());
	}

	void UpdatePanel(bool bActivePanel)
	{
		g_PluginInfo.Control(m_hPlugin
			, bActivePanel
			? FCTL_UPDATEPANEL
			: FCTL_UPDATEANOTHERPANEL
			, (void*)1);
	}

	void UpdateAndRedraw(bool bActivePanel, int nCurrentItem = 0, int nTopPanelItem = 0)
	{
		PanelRedrawInfo pri; 
		pri.CurrentItem = nCurrentItem;
		pri.TopPanelItem = nTopPanelItem; 
		UpdatePanel(bActivePanel);
		RedrawPanel(bActivePanel, &pri);
	}

	operator HANDLE() {return m_hPlugin;}
};
