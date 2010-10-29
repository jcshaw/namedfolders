/*
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "panel.h"
#include <boost/scoped_ptr.hpp>
#include "stlcatalogs.h"
#include "lang.h"
#include "Executor.h"
#include "DialogEditCatalog.h"
#include "DialogEditShortcut.h"
#include "DialogMove.h"
#include <algorithm>
#include <ShLwApi.h>
#include "strings_utils.h"
#include "Parser.h"

using namespace nf;
namespace 
{
	tstring extract_catalog_name(tstring const& src)
	{
		tstring path;
		tstring filename;
		Utils::DividePathFilename(src, path, filename, SLASH_CATS_CHAR, true);
		Utils::RemoveLeadingChars(filename, SLASH_CATS_CHAR);
		return filename;
	}

	enum tshortcut_state
	{
		STATE_UNKNOWN		//состояние не известно
		, STATE_NO_STATE	//состояние отсутствует как класс :)
		, STATE_EXISTS		//именованная директория найдена на диске
		, STATE_PARTLY_EXISTS	//как минимум одна из именованных директорий, 
		//на которые ссылается псевдоним, найдена на диске
		, STATE_NOT_FOUND	//именованная директория не найдена
		, NUMBER_STATES
	};

	ULONG get_state(nf::tshortcut_value_parsed const &p)
	{	
		if (! p.bValueEnabled) return 0;
		if (p.ValueType != nf::VAL_TYPE_LOCAL_DIRECTORY) return 0;

		return 2 << (nf::Parser::IsContainsMetachars(p.value) 
			? STATE_UNKNOWN
			: static_cast<ULONG>(::PathFileExists(p.value.c_str()) 
				? STATE_EXISTS
				: STATE_NOT_FOUND
			)
		);
	}

	inline tshortcut_state get_state(tstring const& Value)
	{
		nf::tshortcut_value_parsed_pair vp = nf::DecodeValues(Value);
		ULONG res = get_state(vp.first) | get_state(vp.second);
		tshortcut_state state = STATE_NO_STATE;
		switch (res)
		{
		case 2 << STATE_EXISTS: state = STATE_EXISTS; break;
		case 2 << STATE_NOT_FOUND: state = STATE_NOT_FOUND; break;
		case (2 << STATE_NOT_FOUND) | (2 << STATE_EXISTS):  state = STATE_PARTLY_EXISTS; break;
		};
		return state;
	}
}

namespace nf
{
	class CPanelUpdater
	{
		ULONG m_nCurrentItem;
		nf::CPanel * const m_pPanel;
		int m_OpMode;
		boost::scoped_ptr<CPanelItemFinder> m_pif;
		CPanelInfoWrap m_hPlugin;
	public:
		CPanelUpdater(nf::CPanel * const p
			, ULONG nCurrentItem = 0
			, int OpMode = 0	//возможный режим обновления панели
			) 
			: m_nCurrentItem(nCurrentItem)
			, m_pPanel(p)
			, m_OpMode(OpMode)
			, m_pif(0)
			, m_hPlugin(m_pPanel)
		{ }
		void SetCursorOnItem(tstring const&PanelItemName, ULONG ItemTypes = CPanel::FG_ANY)
		{	//после обновления панели необходимо найти в обновленном списке элементов 
			//элемент с заданным именем и установить на него курсор
			m_pif.reset(new CPanelItemFinder(PanelItemName.c_str(), ItemTypes));
		}
		void Update(ULONG fgUpdate = CPanel::FG_ALL)
		{	//обновить список элементов и обновит активную панель FAR
			m_pPanel->UpdateListItems(fgUpdate);
			if (m_OpMode & OPM_FIND)  m_hPlugin.UpdatePanel(true);
			else {
				if (m_pif.get()) m_nCurrentItem = (*m_pif)();	//ищем элемент по имени, указанному ранее
				int nTopPanelItem = (static_cast<LONG>(m_nCurrentItem) > 
					m_hPlugin.GetPanelShortInfo(true).PanelRect.bottom)
						? m_nCurrentItem 
						: 0;
				m_hPlugin.UpdateAndRedraw(true, m_nCurrentItem, nTopPanelItem);
			}
		}
		inline void UpdateAnother()
		{	//обновить неактивную панель FAR
			m_hPlugin.UpdateAndRedraw(false, m_hPlugin.GetPanelInfo(false).CurrentItem, 0);
		}
	}; //CPanelUpdater
}

CPanel::CPanel(tstring catalog)
: m_PanelTitle(catalog)
, m_RegNotify(CSettings::GetInstance().get_NamedFolders_reg_key().c_str())
, m_PreviousDir(CPanelInfoWrap(INVALID_HANDLE_VALUE).GetPanelInfo(true).CurDir)
	//get_hPlugin().GetPanelInfo(true).CurDir;
	//get_hPlugin не подходит, т.к. экземпляр панели еще не создан
{
	m_CurrentCatalog = SLASH_CATS + catalog;
	Utils::RemoveTrailingChars(m_CurrentCatalog, SLASH_CATS_CHAR);
	UpdateListItems();
}

CPanel::~CPanel(void)
{
	tmap_panelitems::iterator p = m_find_cash.begin();
	while (p != m_find_cash.end())
	{
		delete p->second;
	}
}


void CPanel::GetOpenPluginInfo(struct OpenPluginInfo *pi)
{
	pi->StructSize = sizeof(*pi);
	pi->Flags = OPIF_USEFILTER 
		| OPIF_ADDDOTS 
		| OPIF_SHOWNAMESONLY 
		| OPIF_SHOWRIGHTALIGNNAMES
		| OPIF_RAWSELECTION
		| OPIF_SHOWPRESERVECASE
		| OPIF_USEHIGHLIGHTING;
	pi->HostFile = 0;
	pi->CurDir = const_cast<char*>(m_CurrentCatalog.c_str());
	pi->Format = (char*)nf::GetMsg(lg::NAMEDFOLDERS);
	m_PanelTitle = get_panel_title().c_str();
	pi->PanelTitle = const_cast<char*>(m_PanelTitle.c_str());

	pi->InfoLines = 0;
	pi->InfoLinesNumber = 0;

	pi->DescrFiles = 0;
	pi->DescrFilesNumber = 0;

	int panel_mode_id = CSettings::GetInstance().GetValue(nf::ST_PANEL_MODE);	
	if ((panel_mode_id > 10) || (panel_mode_id < 0)) panel_mode_id = 6;

	const int NUM_PANEL_MODES = 10;
	static struct PanelMode PanelModesArray[NUM_PANEL_MODES];
	PanelMode& panel_mode = PanelModesArray[panel_mode_id];
	panel_mode.ColumnTypes = NULL;
	panel_mode.ColumnWidths = NULL; //"8,0";
	panel_mode.FullScreen = FALSE;
	panel_mode.Reserved[0] = 0;
	panel_mode.Reserved[1] = 0;

	static char *ColumnTitles[3] = 
	{
		(char*)nf::GetMsg(lg::NF_PANEL_NAME),
		(char*)nf::GetMsg(lg::NF_PANEL_DESCRIPTION),
		(char*)nf::GetMsg(lg::NF_PANEL_STATE)
	};
	PanelModesArray[6].ColumnTypes="N,Z";
//	PanelModesArray[6].ColumnWidths="10,0";	//ширина устанавливается автоматически согласно настройкам пользователя

	tstring types = CSettings::GetInstance().GetValue(nf::STS_PANELWIDTH);	
	
	std::size_t npos =  types.find(_T(';'), 0);
	tstring widths;
	widths.assign(types.begin() + npos + 1, types.end());
	types.erase(types.begin() + npos, types.end());

	static char buf_types[20] = "N,Z,C0";
	static char buf_widths[20] = "10,0,5";

	lstrcpy(&buf_types[0], types.c_str());
	lstrcpy(&buf_widths[0], widths.c_str());
	
	PanelModesArray[8].ColumnTypes = &buf_types[0];
	PanelModesArray[8].ColumnWidths = &buf_widths[0];

	PanelModesArray[8].ColumnTitles = &ColumnTitles[0];
	PanelModesArray[8].FullScreen=FALSE;

	pi->PanelModesArray = PanelModesArray;
	pi->PanelModesNumber = NUM_PANEL_MODES;
	pi->StartPanelMode='0' + panel_mode_id;

	static struct KeyBarTitles KeyBar;
	memset(&KeyBar, 0, sizeof(KeyBar));
	KeyBar.Titles[3-1] = "";
	KeyBar.Titles[5-1] = (char*)nf::GetMsg(lg::F5);
	KeyBar.Titles[6-1] = (char*)nf::GetMsg(lg::F6);
	KeyBar.Titles[8-1] = (char*)nf::GetMsg(lg::F8);
	KeyBar.Titles[7-1] = (char*)nf::GetMsg(lg::F7);
	pi->KeyBar = &KeyBar;
}


int CPanel::SetDirectory(const char *Dir, int OpMode)
{	//открыть указанную директорию - перейти в директорию, на которую указывает ярлычок
	//на активной панели
	CPanelUpdater pu(this, 0, OpMode);

	if (tstring(LEVEL_UP_TWO_POINTS) == Dir)
	{
		return go_to_up_folder(OpMode);
	} else {
		pu.SetCursorOnItem(m_CurrentCatalog, nf::CPanel::FG_CATALOGS);
		m_CurrentCatalog = m_CurrentCatalog + tstring(SLASH_CATS) + tstring(Dir);
	}

	pu.Update();
	return TRUE;
}


bool CPanel::IsSelectedItemIsCatalog(PanelInfo const &pi, int nSelectedItem) const
{
	return ((pi.SelectedItems[nSelectedItem].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

inline nf::tshortcut_info& CPanel::GetSelectedShortcut(PanelInfo const &pi
													   , nf::tshortcut_info& sh
													   , int nSelectedItem) 
{
	sh.catalog = Utils::GetOemToInternal(m_CurrentCatalog);
	sh.shortcut = Utils::GetOemToInternal(pi.SelectedItems[nSelectedItem].FindData.cFileName);
	sh.bIsTemporary = 0 != pi.SelectedItems[nSelectedItem].CRC32;
	return sh;
}

inline tstring CPanel::GetSelectedCatalog(PanelInfo const&pi, int nSelectedItem) 
{
	return Utils::CombinePath(Utils::GetOemToInternal(
		m_CurrentCatalog)
		, Utils::GetOemToInternal(tstring(pi.SelectedItems[nSelectedItem].FindData.cFileName))
		, SLASH_CATS);
}

int CPanel::ProcessKey(int Key, unsigned int ControlState)
{	//обработать нажатие клавиши в FAR 

	switch(Key) {	//курсор игнорируем, исключаем лишние инициализации pi (ниже)
	case VK_LEFT:
	case VK_UP:
	case VK_RIGHT:
	case VK_DOWN:
		return FALSE;
	};

	CPanelInfoWrap hPlugin = get_hPlugin();
	PanelInfo const&pi = hPlugin.GetPanelInfo(true); 
	nf::tshortcut_info sh;

	switch (Key)
	{
	case VK_RETURN:
	case VK_LBUTTON: 
		if (PKF_SHIFT == ControlState)
		{	//открыть в системе
			if (! IsSelectedItemIsCatalog(pi)) 
				nf::Commands::OpenShortcutInExplorer(get_hPlugin(), GetSelectedShortcut(pi, sh), tstring());
			return TRUE;	
		}
		if (! pi.SelectedItemsNumber) return FALSE;	// ".."

		//перейти в директорию соответствующую псевдониму / открыть каталог
		if (! IsSelectedItemIsCatalog(pi))	
		{	//перейти в выбранную директорию
			if (PKF_CONTROL != ControlState)
			{
				nf::Commands::OpenShortcut(get_hPlugin(), GetSelectedShortcut(pi, sh), tstring());
				return TRUE;	//itsn't necessary to update active panel
			}
			else {//на неактивной панели
				nf::Commands::OpenShortcut(get_hPlugin(), GetSelectedShortcut(pi, sh), tstring(), false);
				return TRUE;	//itsn't necessary to update active panel
			}
		} else return FALSE;	
	break;

//Ctrp + PgUp = BACKSPACE
	case VK_PRIOR:	
		if (PKF_CONTROL == ControlState) 
			return go_to_up_folder(0);
		return FALSE;

//удалить псевдоним(ы)/каталог(и)
	case VK_DELETE: if (PKF_SHIFT != ControlState) return FALSE;
	case VK_F8:  
		{
			if (! pi.SelectedItemsNumber) return TRUE;	// ".."

			CPanelUpdater pu(this, pi.CurrentItem);
			
		//составляем полный список выделенных псевдонимов и каталогов
			std::list<nf::tshortcut_info> list_sh;
			std::list<nf::tcatalog_info> list_c;
			for (int i = 0; i < pi.SelectedItemsNumber; ++i)
			{
				if (IsSelectedItemIsCatalog(pi, i)) 
				{
					list_c.push_back(GetSelectedCatalog(pi, i));
				} else {
					list_sh.push_back(GetSelectedShortcut(pi, sh, i));
				}
			}
		//удаляем их и, если хотя бы один из них был удален, обновляем панель
			if (nf::Commands::Deleter(list_sh, list_c, false).Del()) pu.Update();
		} 

		return TRUE;
//обновление и состояние
	case 'R': //Ctrl + Shift + R - получить состояние всех элементов
		if ((ControlState & PKF_CONTROL) == 0) 
			return FALSE; //Ctrl + R - обновить содержимое панели 
		break;

//копирование и перемещение
	case VK_F5:	
		if (pi.SelectedItemsNumber) CopyItems(pi);	// исключаем ".."
		return TRUE;
	case VK_F6: 
		if (pi.SelectedItemsNumber) MoveItems(pi, false);	// исключаем ".."
		return TRUE;

//редактирование
	case VK_F4:
		if (PKF_SHIFT == ControlState) InsertShortcut(pi);
		else if (! IsSelectedItemIsCatalog(pi)) EditShortcut(GetSelectedShortcut(pi, sh), pi);
		return TRUE;

//сохранение настроек
	case VK_F9:
		if (PKF_SHIFT == ControlState) 
		{
			save_setup();
			return TRUE;
		} else return FALSE;
	case VK_F3: 
		return TRUE;	//запретить обработку клавиш FAR'ом
	default:
		return FALSE;	//пусть обрабатываем FAR
	}; //switch

	CPanelUpdater pu(this);
	if (IsSelectedItemIsCatalog(pi))
	{
		nf::tcatalog_info c = GetSelectedCatalog(pi);		
		pu.SetCursorOnItem(Utils::GetInternalToOem(extract_catalog_name(c)), nf::CPanel::FG_CATALOGS);
	} else {
		nf::tshortcut_info sh;
		GetSelectedShortcut(pi, sh);
		pu.SetCursorOnItem(Utils::GetInternalToOem(extract_catalog_name(sh.shortcut)), FG_SHORTCUTS);
	}
	pu.Update();
	
	return TRUE;
}

int CPanel::MakeDirectory (char *Name, int OpMode)
{	//создать новый каталог
	CPanelUpdater pu(this);
	nf::tcatalog_info c;
	if (! CreateCatalog(Utils::GetOemToInternal(m_CurrentCatalog), c)) return -1;	//прервана пользователем

//чтобы позиционироваться на вновь созданный элемент, делаем два Update //!TODO
	tstring filename = extract_catalog_name(c);
	pu.Update(FG_CATALOGS);
	pu.SetCursorOnItem(Utils::GetInternalToOem(filename), FG_CATALOGS);
	pu.Update(FG_CATALOGS);	
	return TRUE;
}

int CPanel::PutFiles(struct PluginPanelItem *PanelItem
					 , int ItemsNumber
					 , int Move
					 , int OpMode)
{	//поместить файлы на панель - создать ярлыки для всех каталогов, перемещаемых на панель
	//!разобрать xml-файл и создать на его основе именованные директории
	if (OpMode & OPM_SILENT) return FALSE;

	for (int i = 0; i < ItemsNumber; ++i)
	{
		tstring cur_dir = get_hPlugin().GetPanelInfo(false).CurDir;	
		cur_dir = Utils::GetOemToInternal(cur_dir);
		if (PanelItem[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)	
		{
			nf::tshortcut_info sht;
			sht.bIsTemporary = false;
			sht.catalog = Utils::GetOemToInternal(m_CurrentCatalog);
			sht.shortcut = Utils::GetOemToInternal(PanelItem[i].FindData.cFileName); 
			if (sht.shortcut == LEVEL_UP_TWO_POINTS) continue;
			tstring value = Utils::CombinePath(cur_dir, sht.shortcut, SLASH_DIRS);
			insert_shortcut(get_hPlugin().GetPanelInfo(false), sht, value);
// 			nf::Commands::AddShortcut(get_hPlugin()
// 				, sht
// 				, value
// 				, true //implicit
// 			);
		};
	}

	CPanelUpdater pu(this, 0, OpMode);
	pu.Update();

	return TRUE;
}

int CPanel::GetFindData(PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode)
{	
	static struct PanelInfo PInfo;
	if (OpMode & OPM_FIND)
	{	//выделяем память только при поиске (т.е. требуется одновременно 
		//загружать в память содержимое всех каталогов)
		tpanelitems *pm = new tpanelitems();	//очистка памяти в FreeFindData
		*pm = m_PanelItems;
		m_find_cash.insert(std::make_pair(&(*pm)[0], pm));
		*pPanelItem = &(*pm)[0];
		*pItemsNumber = static_cast<int>(pm->size());
	} else {
		*pPanelItem = &m_PanelItems[0];
		*pItemsNumber = static_cast<int>(m_PanelItems.size());
	}
	return TRUE;
}

void CPanel::FreeFindData(struct PluginPanelItem *PanelItem, int ItemsNumber)
{	
	tmap_panelitems::iterator p = m_find_cash.find(PanelItem);
	if (p != m_find_cash.end())
	{
		tpanelitems *pm = p->second;
		m_find_cash.erase(p);
		delete pm;
	}
}

int CPanel::ProcessEvent(int Event, void *Param)
{
	//при смене панели событие FE_CHANGEVIEWMODE посылается два раза подряд
	//почему - не ясно; обходим
	static bool bexclude_double = false;
	if (FE_CHANGEVIEWMODE != Event) bexclude_double = false;
	
	if (FE_CHANGEVIEWMODE == Event)
	{
		if (! bexclude_double) 
		{
			CPanelUpdater pu(this, 0);
			pu.Update();
		}
		bexclude_double = true;
	} else if (FE_IDLE == Event) 
	{
		if (m_RegNotify.check())
		{
			//обновляем, если панель плагина не активна... ???
			PanelInfo const& pi = get_hPlugin().GetPanelShortInfo(true);
			if (pi.Plugin)
			{
				CPanelUpdater pu(this, pi.CurrentItem);
				pu.Update();
			}
			m_RegNotify.start();
		};
	} 

	return FALSE;
}

// -----
BOOL CPanel::MoveItems(PanelInfo const &pi
					   , bool bCopy)
{
	bool bSeveralCatalogs = 1 != pi.SelectedItemsNumber;
	bool bFirstItemIsCatalog = IsSelectedItemIsCatalog(pi, 0);
	tstring name;
	if (bFirstItemIsCatalog)
	{
		name = GetSelectedCatalog(pi, 0);
	} else {
		nf::tshortcut_info sh;
		GetSelectedShortcut(pi, sh, 0);
		name = sh.catalog + tstring(SLASH_CATS) + sh.shortcut;			
	};

//куда копируем? проверяем, не открыта ли директория на другой панели...
	tstring default_path = get_inactive_panel_current_catalog();
	bool bPluginOpenedOnInactivePanel = ! default_path.empty();
	
	nf::CDialogMove dlg(name.c_str()
		, default_path
		, bFirstItemIsCatalog
		, bSeveralCatalogs
		, bCopy);
	if (dlg.ShowModal())
	{
		CPanelUpdater pu(this, pi.CurrentItem);
		bool bSingleItem = pi.SelectedItemsNumber == 1;
		
		bool bRet = false;
		for (int i = 0; i < pi.SelectedItemsNumber; ++i)
		{
			tstring new_path = dlg.GetItemName();

		//плагин предлагает переименовать "/11", указываем "22" получаем каталог, вместо псевдонима.
			if (! new_path.empty() && new_path[0] != SLASH_CATS_CHAR) {
				new_path = SLASH_CATS_CHAR + new_path;
			}
			
			if (IsSelectedItemIsCatalog(pi, i))
			{
				tstring new_catalog_name;
				tstring catalog_name = GetSelectedCatalog(pi, i); 
				if (bCopy)
				{
					bRet |= Shell::CopyCatalog(catalog_name.c_str(), new_path);
				} else {
					bRet |= Shell::MoveCatalog(catalog_name.c_str(), new_path);
				}
				if (bSingleItem)
				{
			//если был переименован только один элемент и каталог его не изменился, то
			//позиционируем курсор на этом элементе.
					if (catalog_name == new_path) 
						pu.SetCursorOnItem(new_catalog_name, FG_CATALOGS);
					//!!!! сравнивать только имена каталогов, не полные пути
				}
			} else {
				nf::tshortcut_info new_sh;
				nf::tshortcut_info sh;
				GetSelectedShortcut(pi, sh, i);
				if (bCopy)
				{
					bRet |= Shell::CopyShortcut(sh, new_path, new_sh);
				} else {
					bRet |= Shell::MoveShortcut(sh, new_path, new_sh);
				}
				if (bSingleItem)
				{
			//если был переименован только один элемент и каталог его не изменился, то
			//позиционируем курсор на этом элементе.
					if (sh.catalog == new_sh.catalog) pu.SetCursorOnItem(new_sh.shortcut, FG_SHORTCUTS);
				}
			}
		}

		pu.Update();
		pu.UpdateAnother();

		return static_cast<BOOL>(bRet);
	}

	return FALSE;
}

bool CPanel::EditShortcut(nf::tshortcut_info const &shortcut_name,
						  PanelInfo const &pi)
{
	nf::tshortcut_info sh;
	get_selected_shortcut_info(sh);
	tstring value;
	Shell::GetShortcutValue(sh, value);
	nf::tshortcut_value_parsed_pair vp = nf::DecodeValues(value);

	nf::CDialogEditShortcut dlg(sh, vp.first.value, vp.second.value, false);
	if (dlg.ShowModal())
	{
		CPanelUpdater pu(this, pi.CurrentItem);
		nf::tshortcut_info const& sh2 = dlg.GetShortcutInfo();
		tstring value2 = nf::EncodeValues(dlg.GetValueForActivePanel(), dlg.GetValueForInActivePanel());
		if (Shell::ModifyShortcut(sh, sh2, &value2))
		{
			if (sh.catalog == sh2.catalog) pu.SetCursorOnItem(sh2.shortcut, FG_SHORTCUTS);
			pu.Update();
			return true;
		}
	}
	return false;
}

bool CPanel::insert_shortcut(PanelInfo const &pi, tshortcut_info const &Sh, tstring const &Value)
{
// 	nf::tshortcut_info default_sh;
// 	nf::tshortcut_value_parsed apv;
// 
// 	std::pair<tstring, tstring> &pair_name_value = NameValuePair;
// 	default_sh.shortcut.swap(pair_name_value.first);
// 	apv.value.swap(pair_name_value.second);
// 
// 	default_sh.bIsTemporary = false;
// 	default_sh.catalog = Utils::GetOemToInternal(m_CurrentCatalog);

	nf::CDialogEditShortcut dlg(Sh, Value, tstring(), true);
	if (dlg.ShowModal())
	{
		CPanelUpdater pu(this, pi.CurrentItem);
		nf::tshortcut_info const& sh = dlg.GetShortcutInfo();
		tstring value = nf::EncodeValues(dlg.GetValueForActivePanel(), dlg.GetValueForInActivePanel());

		if (nf::Commands::AddShortcut(get_hPlugin(), sh, value.c_str(), false))
		{
			//делаем update два раза, чтобы корректно позиционироваться на вновь созданный псевдоним
			pu.Update();	//!TODO
			pu.SetCursorOnItem(sh.shortcut, FG_SHORTCUTS);
			pu.Update();	
			return true;	
		}
	}
	return false;
}

bool CPanel::InsertShortcut(PanelInfo const &pi)
{
	nf::tshortcut_info default_sh;
	nf::tshortcut_value_parsed apv;
	
	std::pair<tstring, tstring> pair_name_value = nf::Commands::get_implicit_name_and_value(get_hPlugin(), false);
 	default_sh.shortcut.swap(pair_name_value.first);
 	apv.value.swap(pair_name_value.second);
 
	default_sh.bIsTemporary = false;
	default_sh.catalog = Utils::GetOemToInternal(m_CurrentCatalog);

	return insert_shortcut(pi, default_sh, apv.value);

// 	nf::CDialogEditShortcut dlg(default_sh, apv.value, tstring(), true);
// 	if (dlg.ShowModal())
// 	{
// 		CPanelUpdater pu(this, pi.CurrentItem);
// 		nf::tshortcut_info const& sh = dlg.GetShortcutInfo();
// 		tstring value = nf::EncodeValues(dlg.GetValueForActivePanel(), dlg.GetValueForInActivePanel());
// 
// 		if (nf::Commands::AddShortcut(get_hPlugin(), sh, value.c_str(), false))
// 		{
// 			//делаем update два раза, чтобы корректно позиционироваться на вновь созданный псевдоним
// 			pu.Update();	//!TODO
// 			pu.SetCursorOnItem(sh.shortcut, FG_SHORTCUTS);
// 			pu.Update();	
// 			return true;	
// 		}
// 	}
// 	return false;
}

bool CPanel::CreateCatalog(tstring const& ParentCatalog, tstring &CreatedCatalogName)
{
	nf::CDialogEditCatalog dlg(ParentCatalog.c_str(), _T(""), true);
	if (dlg.ShowModal())
	{
		CreatedCatalogName = dlg.GetCatalogName(true);
		return nf::Commands::AddCatalog(get_hPlugin(), CreatedCatalogName);
	}
	return false;
}

// ----- private
inline void SetPanelItem(PluginPanelItem& item
						 ,tshortcut_state State
						 , char const* name
						 , char const* folder
						 , bool bIsTemporary
						 , bool bIsHidden
						 , bool bIsDirectory)
{
	memset(&item, 0, sizeof(item));
	item.Description = const_cast<char*>(folder);
	item.Flags = 0;
	lstrcpy(item.FindData.cFileName, name);
	item.FindData.dwFileAttributes =  (bIsHidden) ? FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM : 0;
	if (bIsDirectory) item.FindData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
	item.Owner = NULL;
	static char* AddColumns[NUMBER_STATES] = 
	{
		(char*)nf::GetMsg(lg::STATE_UNKNOWN)
		,(char*)nf::GetMsg(lg::STATE_NO)
		,(char*)nf::GetMsg(lg::STATE_OK)
		,(char*)nf::GetMsg(lg::STATE_PARTLY_OK)
		,(char*)nf::GetMsg(lg::STATE_NOT_FOUND)
	};
	item.CustomColumnData = &AddColumns[State];
	item.CustomColumnNumber = 1;
	item.CRC32 = bIsTemporary ? 1 : 0;
}

void CPanel::UpdateListItems(DWORD flags)
{
	int viewmode = CPanelInfoWrap(INVALID_HANDLE_VALUE).GetPanelInfo(true).ViewMode;
	bool bShowState = 8 == viewmode;

	m_PanelItems.clear();

	sc::CCatalog c(Utils::GetOemToInternal(m_CurrentCatalog));
	sc::subcatalogs_sequence cseq(c.GetSequenceSubcatalogs());
	sc::shortcuts_sequence tseq(c.GetSequenceShortcuts(true));
	sc::shortcuts_sequence shseq(c.GetSequenceShortcuts(false));

	//загрузить список каталогов
	if (flags & FG_CATALOGS)
	{
		m_ListCatalogsNames.clear();
		m_ListCatalogsNames.reserve(cseq.size());
		sc::subcatalogs_sequence::const_iterator pc = cseq.begin();
		while (pc != cseq.end())
		{
			sc::catalogs_sequence_item csi(*pc);
			m_ListCatalogsNames.push_back(csi.GetName());
			++pc;
		}
	}

	//загрузить список псевдонимов
	if (flags & FG_SHORTCUTS)
	{
		m_ListShortcutsNames.clear();
		m_ListShortcutsValues.clear();

		m_ListShortcutsNames.reserve(tseq.size() + shseq.size());
		m_ListShortcutsValues.reserve(m_ListShortcutsNames.size());
		//временных
		sc::shortcuts_sequence::const_iterator pt = tseq.begin();
		while (pt != tseq.end())
		{
			sc::shortcuts_sequence_item si(*pt);
			m_ListShortcutsNames.push_back(si.GetName());
			m_ListShortcutsValues.push_back(si.GetValue());
			++pt;
		}
		NumberTemporaryShortcuts = static_cast<int>(tseq.size());
		//постоянных
		sc::shortcuts_sequence::const_iterator psh = shseq.begin();
		while (psh != shseq.end())
		{
			sc::shortcuts_sequence_item si(*psh);
			m_ListShortcutsNames.push_back(si.GetName());
			m_ListShortcutsValues.push_back(si.GetValue());
			++psh;
		}
	}

	BOOL bTemporaryAsHidden = CSettings::GetInstance().GetValue(nf::ST_SHOW_TEMPORARY_AS_HIDDEN);
	m_PanelItems.resize(m_ListCatalogsNames.size() + m_ListShortcutsNames.size());

	for (unsigned int i = 0; i < m_ListCatalogsNames.size(); ++i)
		SetPanelItem(m_PanelItems[i]
			, STATE_NO_STATE
			, m_ListCatalogsNames[i].c_str()
			, _T("")	//value
			, false
			, false
			, true
		);

	for (unsigned int i = 0; i < m_ListShortcutsNames.size(); ++i)
	{
		SetPanelItem(m_PanelItems[m_ListCatalogsNames.size() + i]
			, bShowState 
				? get_state(m_ListShortcutsValues[i])
				: STATE_NO_STATE
			, m_ListShortcutsNames[i].c_str()
			, m_ListShortcutsValues[i].c_str()
			, i < NumberTemporaryShortcuts
			, (i < NumberTemporaryShortcuts) && bTemporaryAsHidden
			, false
		);
	}

}

int CPanel::go_to_up_folder(int OpMode)
{
	if (m_CurrentCatalog.empty())
	{
		if (OpMode & OPM_FIND) return FALSE;
		get_hPlugin().ClosePlugin(m_PreviousDir);
		return TRUE;
	}
	else 
	{ 
		tstring CurrentCatalogName;
		int nCurrentItem = -1;

		size_t npos = m_CurrentCatalog.find_last_of(SLASH_CATS_CHAR);
		if (npos == tstring::npos)
		{
			m_CurrentCatalog = _T("");
			nCurrentItem = 0;
		} else 
		{	//позиционируем курсор на том каталоге из которого вышли
			CurrentCatalogName.assign(m_CurrentCatalog.begin()+npos+1, m_CurrentCatalog.end()); 
			m_CurrentCatalog.erase(npos, m_CurrentCatalog.size()-npos);
		}

		nf::CPanelUpdater pu(this, 0, OpMode);
		pu.SetCursorOnItem(CurrentCatalogName, nf::CPanel::FG_CATALOGS);
		pu.Update();

		return TRUE;
	}

}

void CPanel::get_selected_shortcut_info(nf::tshortcut_info &sh)
{
	PanelInfo const &pi = get_hPlugin().GetPanelInfo(true);
	assert(! IsSelectedItemIsCatalog(pi));
	
	sh.bIsTemporary = pi.SelectedItems[0].FindData.dwFileAttributes && FILE_ATTRIBUTE_TEMPORARY;
	sh.catalog = Utils::GetOemToInternal(m_CurrentCatalog);
	sh.shortcut = Utils::GetOemToInternal(pi.SelectedItems[0].FindData.cFileName);
}

tstring CPanel::get_inactive_panel_current_catalog()
{
	tstring s = Utils::GetOemToInternal(get_hPlugin().GetPanelInfo(false).CurDir);
	if (s.find_first_of(_T(":")) != tstring::npos) return _T("");
	else return s;
}

void CPanel::save_setup()
{
	//запоминаем состояние панели
	int ViewMode = get_hPlugin().GetPanelInfo(true).ViewMode;
	const char *Msg = GetMsg(lg::MSG_SAVE_SETUP_MESSAGE);      

	if (g_PluginInfo.Message(g_PluginInfo.ModuleNumber
		, FMSG_MB_OKCANCEL | FMSG_ALLINONE
		, 0
		, (char const * const*) Msg
		, 0, 0) == 0) 
	{
		CSettings::GetInstance().SetValue(nf::ST_PANEL_MODE, ViewMode);	
		CSettings::GetInstance().SaveSettings();
	}
}