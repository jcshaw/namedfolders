/*
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once
#include <vector>
#include <boost/utility.hpp>
//#include "far2/plugin.hpp"
#include "kernel.h"
#include "PanelInfoWrap.h"
#include "lang.h"
#include "settings.h"
#include "panel_types.h"

namespace nf {
namespace Panel {

class CPanel : boost::noncopyable {
	typedef tpair_strings tname_value; //name and value of shortcut
	typedef std::vector<PluginPanelItem> tpanelitems0;
public:
	typedef std::pair<tpanelitems0, tlist_buffers> tpanelitems;
	typedef std::list<std::pair<PluginPanelItem*, tpanelitems*> > tmap_panelitems;

	CPanel(tstring catalog = L"");
	~CPanel(void);

	void GetOpenPanelInfo(struct OpenPanelInfo *Info);
	int GetFindData(struct GetFindDataInfo *pInfo);
	void FreeFindData(const struct FreeFindDataInfo *pInfo);
	int SetDirectory(const struct SetDirectoryInfo *pInfo);
	int PutFiles(const struct PutFilesInfo *pInfo);
	int ProcessPanelInputW(INPUT_RECORD const& inputRecord);
	int MakeDirectory (struct MakeDirectoryInfo *pInfo);
	int ProcessEvent(const struct ProcessPanelEventInfo *pInfo);
	void UpdateListItems(DWORD flags = FG_ALL);
public:
	CPanelInfoWrap get_hPlugin() {
		return  CPanelInfoWrap(this); }
	tstring const& GetCurrentCatalog() const {
		return m_CurrentCatalog;
	}
private: 
	tpanelitems m_PanelItems;			//list of all panel items

	tstring m_PanelTitle;				
	tstring m_CurrentCatalog;			//current NF-catalog 

	nf::tlist_pairs_strings m_ListShortcuts; //list of all shortcuts - names and values
	nf::tvector_strings m_ListCatalogs;	  //list of all NF-catalogs on panel - only names

	unsigned int m_NumberTemporaryShortcuts;		//���-�� ��������� ����������� (������ � ������ m_ListShortcuts)

	tstring const m_PreviousDir;	//����������, �������� �� ������ ������� �� �������� ������
	tmap_panelitems m_FindCache;	//��� ������ ������� ������ ��� �������� ����������� ���� ���������

	int go_to_up_folder(OPERATION_MODES OpMode);
	void read_list_panelitems(DWORD flags);
};
} //Panel
}	//nf