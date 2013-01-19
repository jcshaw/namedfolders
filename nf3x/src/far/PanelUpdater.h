/*
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once
#include <vector>
#include <boost/scoped_ptr.hpp>

//#include "far2/plugin.hpp"
#include "PanelInfoWrap.h"
#include "panel_types.h"
#include "Panel.h"

namespace nf {
namespace Panel {

class CPanelUpdater {
public:
	CPanelUpdater(CPanel* const p, ULONG nCurrentItem = 0, int OpMode = 0) 
		: m_nCurrentItem(nCurrentItem)
		, m_pPanel(p)
		, m_OpMode(OpMode) //mode of update panel
		, m_pif(0)
		, m_hPlugin(m_pPanel)
	{ }
	void SetCursorOnItem(tstring const&PanelItemName, ULONG ItemTypes = FG_ANY) 
	{	//find item with specified item on panel after panel update and set cursor on it
		m_pif.reset(new CPanelItemFinder(PanelItemName.c_str(), ItemTypes));
	}
	void UpdateActivePanel(ULONG fgUpdate = FG_ALL)
	{	//update list of items and update active FAR panel
		m_pPanel->UpdateListItems(fgUpdate);
		if (m_OpMode & OPM_FIND) {
			m_hPlugin.UpdatePanel(true);
		} else {
			if (m_pif.get()) {
				m_nCurrentItem = (*m_pif)();	//ищем элемент по имени, указанному ранее
			}
			int nTopPanelItem = (static_cast<LONG>(m_nCurrentItem) > 
				m_hPlugin.GetPanelInfo(true).PanelRect.bottom)
				? m_nCurrentItem 
				: 0;
			m_hPlugin.UpdateAndRedraw(true, m_nCurrentItem, nTopPanelItem);
		}
	}
	void UpdateInactivePanel() {	
		m_hPlugin.UpdateAndRedraw(false, m_hPlugin.GetPanelInfo(false).CurrentItem, 0);
	}
private:
	ULONG m_nCurrentItem;
	CPanel* const m_pPanel;
	int m_OpMode;
	boost::scoped_ptr<CPanelItemFinder> m_pif;
	CPanelInfoWrap m_hPlugin;
}; 

}
}