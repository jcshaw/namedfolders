/*
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#pragma once
#include "plugin.hpp"
#include <map>
#include <vector>
#include "kernel.h"
#include "regnotify.h"
#include "PanelInfoWrap.h"
#include "lang.h"
#include "settings.h"

namespace nf {

class CPanelUpdater;

class CPanel
{
	typedef std::vector<tstring> tvecstr;
	typedef std::vector<PluginPanelItem> tpanelitems;
	typedef std::map<PluginPanelItem*, tpanelitems*> tmap_panelitems;

public:
	CPanel(tstring catalog = _T(""));
	~CPanel(void);

	enum {
		FG_SHORTCUTS = 0x1
		,FG_CATALOGS = 0x2
		, FG_ALL = FG_SHORTCUTS | FG_CATALOGS
		, FG_ANY = FG_ALL
	};

	void GetOpenPluginInfo(struct OpenPluginInfo *Info);
	int GetFindData(PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode);	
	void FreeFindData(struct PluginPanelItem *PanelItem, int ItemsNumber);
	int SetDirectory(const char *Dir, int OpMode);
	int PutFiles(struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode);
	int ProcessKey(int Key, unsigned int ControlState);
	int MakeDirectory (char *Name, int OpMode);
	int ProcessEvent(int Event, void *Param);

public:
	void UpdateListItems(DWORD flags = FG_ALL);

private:
	bool CreateCatalog(tstring const& ParentCatalog, tstring &CreatedCatalogName);
	BOOL MoveItems(PanelInfo const &pi, bool bCopy = false);
	BOOL CopyItems(PanelInfo const &pi) { return MoveItems(pi, true); }
	bool EditShortcut(nf::tshortcut_info const &sh, PanelInfo const &pi);
	bool InsertShortcut(PanelInfo const &pi);
	void save_setup();

private:
	tpanelitems m_PanelItems;			//список всех элементов панели 
//загрузить в память список всех именованных директорий и каталогов отображаемых на панели
//подготовить m_PanelItems
	friend CPanelUpdater;

	bool IsSelectedItemIsCatalog(PanelInfo const &pi, int nSelectedItem = 0) const;
	nf::tshortcut_info& GetSelectedShortcut(PanelInfo const &pi, nf::tshortcut_info& sh, int nSelectedItem = 0);
	tstring GetSelectedCatalog(PanelInfo const &pi, int nSelectedItem = 0);
	int go_to_up_folder(int OpMode);
	tstring get_panel_title() const
	{
		return tstring(GetMsg(lg::PANEL_TITLE)) + m_CurrentCatalog; 
	}
	void get_selected_shortcut_info(nf::tshortcut_info &sh);
	tstring get_inactive_panel_current_catalog();

	CPanelInfoWrap get_hPlugin() {return  CPanelInfoWrap(this); }
	bool insert_shortcut(PanelInfo const &Pi, tshortcut_info const &Sh, tstring const &Value);

private: //members
	tstring m_PanelTitle;				//заголовок панели
	tstring m_CurrentCatalog;			//текущий каталог открытый на панели

	tvecstr m_ListShortcutsValues;		//список значений всех именованных директорий
	tvecstr m_ListShortcutsNames;		//список названий всех именованных директорий
	tvecstr m_ListCatalogsNames;		//список названий всех каталогов
	unsigned int NumberTemporaryShortcuts;		//кол-во временных псевдонимов (первые в списке m_ListShortcutsNames)

	tstring const m_PreviousDir;	//директори, открытая на панели плагина до открытия панели
	tmap_panelitems m_find_cash;	//при поиске выделям память для хранения содержимого всех каталогов

	CRegNotify m_RegNotify;
};


class CPanelItemFinder
{	//реализует отложенный поиск позиции элемента на панели
	tstring m_ItemName;
	ULONG m_ItemsType;
public:
	CPanelItemFinder(TCHAR const* ItemName, ULONG ItemsType = CPanel::FG_ANY)
		: m_ItemName(ItemName), m_ItemsType(ItemsType)
	{ }
	int operator()()
	{
		PanelInfo pi;
		g_PluginInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &pi);
		for (int i = 0; i < pi.ItemsNumber; ++i)
		{
			if (m_ItemName == tstring(pi.PanelItems[i].FindData.cFileName))
			{
				bool bIsCatalog = (pi.PanelItems[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0; 
				if (((m_ItemsType & CPanel::FG_CATALOGS) && bIsCatalog) || 
					((m_ItemsType & CPanel::FG_SHORTCUTS) && (!bIsCatalog)))
				{
					return i;
				}
			}
		}
		return 0;		
	}
};


};	//fp