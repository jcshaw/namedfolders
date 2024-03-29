/*
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"

#include <algorithm>
#include <ShLwApi.h>
#include <boost/foreach.hpp>

#include "panel.h"
#include "panel_types.h"
#include "panel_functions.h"
#include "PanelUpdater.h"

#include "stlcatalogs.h"
#include "lang.h"
#include "Executor.h"
#include "strings_utils.h"
#include "Parser.h"
#include "catalog_names.h"

#include "far/CatalogSequences.h"

using namespace nf;
using namespace Panel;
namespace {
	enum tshortcut_state {
		STATE_UNKNOWN		//��������� �� ��������
		, STATE_NO_STATE	//��������� ����������� ��� ����� :)
		, STATE_EXISTS		//����������� ���������� ������� �� �����
		, STATE_PARTLY_EXISTS	//��� ������� ���� �� ����������� ����������, 
		//�� ������� ��������� ���������, ������� �� �����
		, STATE_NOT_FOUND	//����������� ���������� �� �������
		, NUMBER_STATES
	};

	ULONG get_state(nf::tshortcut_value_parsed const &p) {	
		if (! p.bValueEnabled) return 0;
		if (p.ValueType != nf::VAL_TYPE_LOCAL_DIRECTORY) return 0;

		return 2 << (nf::Parser::ContainsMetachars(p.value) 
			? STATE_UNKNOWN
			: static_cast<ULONG>(::PathFileExists(p.value.c_str()) 
				? STATE_EXISTS
				: STATE_NOT_FOUND
			)
		);
	}

	inline tshortcut_state get_state(tstring const& Value) {
		nf::tshortcut_value_parsed_pair vp = nf::DecodeValues(Value);
		ULONG res = get_state(vp.first) | get_state(vp.second);
		tshortcut_state state = STATE_NO_STATE;
		switch (res) {
		case 2 << STATE_EXISTS: state = STATE_EXISTS; break;
		case 2 << STATE_NOT_FOUND: state = STATE_NOT_FOUND; break;
		case (2 << STATE_NOT_FOUND) | (2 << STATE_EXISTS):  state = STATE_PARTLY_EXISTS; break;
		};
		return state;
	}
	
	inline tstring get_panel_title(tstring const& currentCatalog) {
		return tstring(GetMsg(lg::PANEL_TITLE)) + currentCatalog; 
	}
}

CPanel::CPanel(tstring catalog)
: m_PanelTitle(catalog)
, m_PreviousDir(CPanelInfoWrap(INVALID_HANDLE_VALUE).GetPanelCurDir(true))
	//get_hPlugin().GetPanelInfo(true).CurDir;
	//get_hPlugin �� ��������, �.�. ��������� ������ ��� �� ������
{
	m_CurrentCatalog = SLASH_CATS + catalog;
	Utils::RemoveTrailingCharsOnPlace(m_CurrentCatalog, SLASH_CATS_CHAR);
	UpdateListItems();
}

CPanel::~CPanel(void)
{
	tmap_panelitems::iterator p = m_FindCache.begin();
	while (p != m_FindCache.end()) {
		delete p->second;
	}
}

namespace {
	void set_keybar_label(KeyBarTitles &kb, int index, wchar_t const* pMsg) {
		// 	KeyBar.Titles[3-1] = L"";
		// 	KeyBar.Titles[5-1] = (wchar_t*)nf::GetMsg(lg::F5);
		// 	KeyBar.Titles[6-1] = (wchar_t*)nf::GetMsg(lg::F6);
		// 	KeyBar.Titles[8-1] = (wchar_t*)nf::GetMsg(lg::F8);
		// 	KeyBar.Titles[7-1] = (wchar_t*)nf::GetMsg(lg::F7);
		memset(&kb.Labels[index], 0, sizeof(KeyBarLabel));
		if (pMsg != nullptr) {
			kb.Labels[index].Text = pMsg;
		}
	}
}


void CPanel::GetOpenPanelInfo(struct OpenPanelInfo *pi) {
	memset(pi, 0, sizeof(OpenPanelInfo));
	pi->StructSize = sizeof(*pi);
	pi->Flags = OPIF_ADDDOTS 
		//OPIF_USEFILTER  // = !OPIF_DISABLEFILTER
		| OPIF_SHOWNAMESONLY 
		| OPIF_SHOWRIGHTALIGNNAMES
		| OPIF_RAWSELECTION
		| OPIF_SHOWPRESERVECASE
		//| OPIF_USEHIGHLIGHTING //OPIF_DISABLEHIGHLIGHTING
		;
	pi->HostFile = 0;
	pi->CurDir = const_cast<wchar_t*>(m_CurrentCatalog.c_str());
	pi->Format = (wchar_t*)nf::GetMsg(lg::NAMEDFOLDERS);
	m_PanelTitle = get_panel_title(m_CurrentCatalog).c_str();
	pi->PanelTitle = const_cast<wchar_t*>(m_PanelTitle.c_str());

	pi->InfoLines = 0;
	pi->InfoLinesNumber = 0;

	pi->DescrFiles = 0;
	pi->DescrFilesNumber = 0;

	int panel_mode_id = CSettings::GetInstance().GetValue(nf::ST_PANEL_MODE);	
	if ((panel_mode_id > 10) || (panel_mode_id < 0)) panel_mode_id = 6;

	const int NUM_PANEL_MODES = 10;
	static struct PanelMode PanelModesArray[NUM_PANEL_MODES];
	PanelMode& panel_mode = PanelModesArray[panel_mode_id];
	memset(&panel_mode, 0, sizeof(PanelMode) );
	panel_mode.ColumnTypes = NULL;
	panel_mode.ColumnWidths = NULL; //"8,0";	

	static wchar_t *ColumnTitles[3] = {
		(wchar_t*)nf::GetMsg(lg::NF_PANEL_NAME),
		(wchar_t*)nf::GetMsg(lg::NF_PANEL_DESCRIPTION),
		(wchar_t*)nf::GetMsg(lg::NF_PANEL_STATE)
	};
	PanelModesArray[6].ColumnTypes=L"N,Z";
//	PanelModesArray[6].ColumnWidths="10,0";	//������ ��������������� ������������� �������� ���������� ������������

	tstring types = CSettings::GetInstance().GetValue(nf::STS_PANELWIDTH);	
	
	std::size_t npos =  types.find(L';', 0);
	tstring widths;
	widths.assign(types.begin() + npos + 1, types.end());
	types.erase(types.begin() + npos, types.end());

	static wchar_t buf_types[20] = L"N,Z,C0";
	static wchar_t buf_widths[20] = L"10,0,5";

	lstrcpy(&buf_types[0], types.c_str());
	lstrcpy(&buf_widths[0], widths.c_str());
	
	PanelModesArray[8].ColumnTypes = &buf_types[0];
	PanelModesArray[8].ColumnWidths = &buf_widths[0];

	PanelModesArray[8].ColumnTitles = &ColumnTitles[0];

	pi->PanelModesArray = PanelModesArray;
	pi->PanelModesNumber = NUM_PANEL_MODES;
	pi->StartPanelMode='0' + panel_mode_id;

	
// 	static struct KeyBarTitles kb; 
// 	memset(&kb, 0, sizeof(KeyBarTitles));
// 	set_keybar_label(kb, 3-1, nullptr);
// 	set_keybar_label(kb, 5-1, (wchar_t*)nf::GetMsg(lg::F5));
// 	set_keybar_label(kb, 6-1, (wchar_t*)nf::GetMsg(lg::F6));
// 	set_keybar_label(kb, 7-1, (wchar_t*)nf::GetMsg(lg::F7));
// 	set_keybar_label(kb, 8-1, (wchar_t*)nf::GetMsg(lg::F8));
	pi->KeyBar = 0;//!todo &kb;
}


int CPanel::SetDirectory(const struct SetDirectoryInfo *pInfo)
{	//������� ��������� ���������� - ������� � ����������, �� ������� ��������� �������
	//�� �������� ������
	CPanelUpdater pu(this, 0, pInfo->OpMode);
	tstring sdir = Utils::ReplaceStringAll(pInfo->Dir, SLASH_DIRS, SLASH_CATS);

	if (tstring(LEVEL_UP_TWO_POINTS) == sdir || sdir == SLASH_CATS) {
		return go_to_up_folder(pInfo->OpMode);
	} else {
		pu.SetCursorOnItem(m_CurrentCatalog, FG_CATALOGS);
		if (sdir != SLASH_CATS) { //bug #39
			m_CurrentCatalog = m_CurrentCatalog + tstring(SLASH_CATS) + tstring(sdir);
		}
	}

	pu.UpdateActivePanel();
	return TRUE;
}

int CPanel::ProcessPanelInputW(INPUT_RECORD const& inputRecord) {	
	if (inputRecord.EventType != KEY_EVENT) {
		return 0;
	}
	KEY_EVENT_RECORD const& ker = inputRecord.Event.KeyEvent;

	switch(ker.wVirtualKeyCode) {	//let's FAR handle cursor movements 
		case VK_LEFT: 
		case VK_UP: 
		case VK_RIGHT: 
		case VK_DOWN: 
			return FALSE;
	};
	PanelInfo const& pi = get_hPlugin().GetPanelInfo(true); 

	switch (ker.wVirtualKeyCode) {
	case VK_RETURN:
	case VK_LBUTTON: //open
		return OpenSelectedItem(this, ker.dwControlKeyState, pi);
	case VK_PRIOR:	//go to up level - Ctrp + PgUp = BACKSPACE 		
		if (LEFT_CTRL_PRESSED == ker.dwControlKeyState) { 
			return go_to_up_folder(0);
		}
		return FALSE;
	case VK_DELETE:  //delete
		if (SHIFT_PRESSED != ker.dwControlKeyState) return FALSE; 
		//no break here!
	case VK_F8:  
		DeleteSelectedCatalogsAndShortcuts(this, pi);
		return TRUE;
	case 'R': //Ctrl + R update state
		if ((ker.dwControlKeyState & LEFT_CTRL_PRESSED) == 0) return FALSE; 
		UpdateCursorPositionOnFarPanel(this, pi);
		return TRUE;
	case VK_F5:	 //copy
		if (pi.SelectedItemsNumber) CopyItems(this, pi);	// ��������� ".."
		return TRUE;
	case VK_F6: //move
		if (pi.SelectedItemsNumber) {
			MoveItems(this, pi, (SHIFT_PRESSED != ker.dwControlKeyState) != 0 ? ID_CM_MOVE : ID_CM_RENAME);	// ��������� ".."
		}
		return TRUE;
	case VK_F4: //edit/create shortcut name
		if (SHIFT_PRESSED == ker.dwControlKeyState) {
			InsertShortcut(this, pi);
		} else if (IsSelectedItemIsCatalog(this, pi)) {
			if (pi.SelectedItemsNumber) { // ��������� ".."
				EditCatalog(this, GetSelectedCatalog(this, pi));
			}
		} else {
			nf::tshortcut_info sh;
			EditShortcut(this, GetSelectedShortcut(this, pi, sh), pi);
		}
		return TRUE;
	case VK_F9: //save settings by F9
		if (SHIFT_PRESSED == ker.dwControlKeyState) {
			SaveSetup(this);
			return TRUE;
		} 
		return FALSE;
	case VK_F3:  //forbid handling by FAR
		return TRUE;	
	default:
		return FALSE;	//let's FAR handle it
	}; 

	assert(false); //UpdateCursorPositionOnFarPanel(this, pi);
	return TRUE;
}

int CPanel::MakeDirectory(struct MakeDirectoryInfo *pInfo)
{	//������� ����� �������
	CPanelUpdater pu(this);
	nf::tcatalog_info c;
	if (! CreateCatalog(this, m_CurrentCatalog, c)) return -1;	//�������� �������������

//����� ����������������� �� ����� ��������� �������, ������ ��� Update //!TODO
	tstring filename = Utils::ExtractCatalogName(c);
	pu.UpdateActivePanel(FG_CATALOGS);
	pu.SetCursorOnItem(filename, FG_CATALOGS);
	pu.UpdateActivePanel(FG_CATALOGS);	
	return TRUE;
}

int CPanel::PutFiles(const struct PutFilesInfo *pInfo) {	
	//create shortcuts for all directories that are being placed on the panel 
	if (pInfo->OpMode & OPM_SILENT) return FALSE;

	for (unsigned int i = 0; i < pInfo->ItemsNumber; ++i) {
		tstring cur_dir = get_hPlugin().GetPanelCurDir(true);	
		if (pInfo->PanelItem[i].FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			nf::tshortcut_info sht = nf::MakeShortcut(m_CurrentCatalog, pInfo->PanelItem[i].FileName, false);
			if (sht.shortcut == LEVEL_UP_TWO_POINTS) continue;
			tstring value = Utils::CombinePath(cur_dir, sht.shortcut, SLASH_DIRS);
			InsertShortcut(this, get_hPlugin().GetPanelInfo(false), sht, value);
		};
	}

	CPanelUpdater pu(this, 0, pInfo->OpMode);
	pu.UpdateActivePanel();

	return TRUE;
}

int CPanel::GetFindData(struct GetFindDataInfo *pInfo) {	
	static struct PanelInfo PInfo;
	if (pInfo->OpMode & OPM_FIND)
	{	//�������� ������ ������ ��� ������ (�.�. ��������� ������������ 
		//��������� � ������ ���������� ���� ���������)
		tpanelitems* pm = new tpanelitems();	//������� ������ � FreeFindData
		*pm = m_PanelItems; //copy(!) all items
		if (! pm->first.empty()) {
			m_FindCache.push_back(std::make_pair(&(pm->first)[0], pm));
			pInfo->PanelItem = &(pm->first)[0];
			pInfo->ItemsNumber = static_cast<int>(pm->first.size());
		} else {
			pInfo->PanelItem = 0;
			pInfo->ItemsNumber = 0;
			delete pm;
			return FALSE;
		}
	} else {
		pInfo->PanelItem = m_PanelItems.first.size() == 0 
			? 0
			: &m_PanelItems.first[0];
		pInfo->ItemsNumber = static_cast<int>(m_PanelItems.first.size());
	}
	return TRUE;
}

namespace {
	inline bool equal_panel_item(std::pair<PluginPanelItem*, CPanel::tpanelitems*> const& item, struct PluginPanelItem *panelItem) {
		return item.first == panelItem;
	}
}
void CPanel::FreeFindData(const struct FreeFindDataInfo *pInfo) {	
	tmap_panelitems::iterator p = std::find_if(m_FindCache.begin(), m_FindCache.end()
		, boost::bind(&equal_panel_item, _1,  pInfo->PanelItem) );
	if (p != m_FindCache.end()) {
		tpanelitems *pm = p->second;
		m_FindCache.erase(p);
		delete pm;
	}
}

int CPanel::ProcessEvent(const struct ProcessPanelEventInfo *pInfo) {
	//��� ����� ������ ������� FE_CHANGEVIEWMODE ���������� ��� ���� ������
	//������ - �� ����; �������
	static bool bexclude_double = false;
	if (FE_CHANGEVIEWMODE != pInfo->Event) bexclude_double = false;
	
	if (FE_CHANGEVIEWMODE == pInfo->Event) {
		if (! bexclude_double) {
			CPanelUpdater pu(this, 0);
			pu.UpdateActivePanel();
		}
		bexclude_double = true;
	} else if (FE_IDLE == pInfo->Event) {
//!TODO:FAR3: is it possible to know about changes in panel content?
// 		if (m_RegNotify.Check()) {
// 			//���������, ���� ������ ������� �� �������... ???
// 			PanelInfo const& pi = get_hPlugin().GetPanelInfo(true);
// 			if ((pi.Flags & PFLAGS_PLUGIN) != 0) {
// 				CPanelUpdater pu(this, pi.CurrentItem);
// 				pu.UpdateActivePanel();
// 				pu.UpdateInactivePanel();
// 			}
// 			m_RegNotify.Start();
//		};
	}

	return FALSE;
}

void set_panel_item(PluginPanelItem& item
					, tstring_buffer& itemBuffer
					, tshortcut_state State
					, tstring const& name
					, tstring const& folder
					, bool bIsTemporary
					, bool bIsHidden
					, bool bIsDirectory) {
	memset(&item, 0, sizeof(item));
	item.Description = folder.empty() ? 0 : folder.c_str();
	item.Flags = 0;
	itemBuffer.reset(Utils::Str2Buffer(name));
	item.FileName = &(*itemBuffer)[0];
	item.FileAttributes =  (bIsHidden) ? FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM : 0;
	if (bIsDirectory) item.FileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
	item.Owner = NULL;
	static wchar_t const* AddColumns[NUMBER_STATES] = {
		nf::GetMsg(lg::STATE_UNKNOWN)
		, nf::GetMsg(lg::STATE_NO)
		, nf::GetMsg(lg::STATE_OK)
		, nf::GetMsg(lg::STATE_PARTLY_OK)
		, nf::GetMsg(lg::STATE_NOT_FOUND)
	};
	item.CustomColumnData = &AddColumns[State];
	item.CustomColumnNumber = 1;
	item.CRC32 = bIsTemporary ? 1 : 0;
}

void CPanel::read_list_panelitems(DWORD flags) {
	sc::CCatalog c(m_CurrentCatalog);
	if (flags & FG_CATALOGS) { //load list of NF-catalogs
		m_ListCatalogs.clear();
		auto cseq = c.GetSequenceSubcatalogs();
		m_ListCatalogs.reserve(cseq->getItems().size());
		BOOST_FOREACH(auto const& csi, cseq->getItems()) {
			m_ListCatalogs.push_back(nf::get_catalog_item_name(csi));
		}
	}
	if (flags & FG_SHORTCUTS) { //load list of temporal and not-temporal shortcuts
		m_ListShortcuts.clear();
		auto seq_shtemp = c.GetSequenceShortcuts(true);
		auto seq_sh = c.GetSequenceShortcuts(false);
		
		BOOST_FOREACH(auto const& si, seq_shtemp->getItems()) { //temporal shortcuts
			m_ListShortcuts.push_back(std::make_pair(nf::get_ssi_name(si), nf::get_ssi_value(si)));
		}
		m_NumberTemporaryShortcuts = static_cast<int>(seq_shtemp->getItems().size());
		BOOST_FOREACH(auto const& si, seq_sh->getItems()) { //not-temporal shortcuts
			m_ListShortcuts.push_back(std::make_pair(nf::get_ssi_name(si), nf::get_ssi_value(si)));
		}
	}
}

void CPanel::UpdateListItems(DWORD flags) {
	BOOL btemporary_is_hidden = CSettings::GetInstance().GetValue(nf::ST_SHOW_TEMPORARY_AS_HIDDEN);
	int bshow_state = 8 == CPanelInfoWrap(INVALID_HANDLE_VALUE).GetPanelInfo(true).ViewMode;

	read_list_panelitems(flags);

	m_PanelItems.first.resize(m_ListCatalogs.size() + m_ListShortcuts.size());
	m_PanelItems.second.resize(m_PanelItems.first.size());

	for (unsigned int i = 0; i < m_ListCatalogs.size(); ++i) {
		set_panel_item(m_PanelItems.first[i]
			, m_PanelItems.second[i] 
			, STATE_NO_STATE
			, m_ListCatalogs[i]
			, L""
			, false
			, false
			, true
		);
	}

	unsigned int index = 0;
	BOOST_FOREACH(tpair_strings const& pair, m_ListShortcuts) {
		set_panel_item(m_PanelItems.first[m_ListCatalogs.size() + index]
			, m_PanelItems.second[m_ListCatalogs.size() + index]
			, bshow_state 
				? get_state(pair.second)
				: STATE_NO_STATE
			, pair.first
			, pair.second
			, index < m_NumberTemporaryShortcuts
			, (index < m_NumberTemporaryShortcuts) && btemporary_is_hidden
			, false
		);
		++index;
	}
}

int CPanel::go_to_up_folder(OPERATION_MODES OpMode) {
	if (m_CurrentCatalog.empty()) {
		if (OpMode & OPM_FIND) return FALSE;
		get_hPlugin().ClosePlugin(m_PreviousDir);
		return TRUE;
	} else { 
		tstring CurrentCatalogName;
		int nCurrentItem = -1;

		size_t npos = m_CurrentCatalog.find_last_of(SLASH_CATS_CHAR);
		if (npos == tstring::npos) {
			m_CurrentCatalog = L"";
			nCurrentItem = 0;
		} else {	//������������� ������ �� ��� �������� �� �������� �����
			CurrentCatalogName.assign(m_CurrentCatalog.begin()+npos+1, m_CurrentCatalog.end()); 
			m_CurrentCatalog.erase(npos, m_CurrentCatalog.size()-npos);
		}

		CPanelUpdater pu(this, 0, OpMode);
		pu.UpdateActivePanel();
		pu.SetCursorOnItem(CurrentCatalogName, FG_CATALOGS);
		pu.UpdateActivePanel();

		return TRUE;
	}
}


