/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once 

//far dialog item's templates
#include <cassert>
#include <vector>
#include <boost/utility.hpp>
#include <Windows.h>
#include <algorithm>

namespace nf {

extern const wchar_t* GetMsg(int MsgId);

class far_di_item : boost::noncopyable {
public:
	far_di_item(unsigned int ItemIndex
		, int ItemType
		, int X1, int Y1, int X2, int Y2
		, int Focus
		, int Selected
		, DWORD Flags
		, int DefaultButton
		, wchar_t const* pData)
		: m_ItemIndex(ItemIndex)
	{
		m_pBuffer.reset(new nf::tautobuffer_char(1)); //stlsoft-autobuffer doesn't allow zero size
		m_FarDialogItem.Type = ItemType;
		m_FarDialogItem.X1 = X1;
		m_FarDialogItem.Y1 = Y1;
		m_FarDialogItem.X2 = X2;
		m_FarDialogItem.Y2 = Y2;
		m_FarDialogItem.Focus = Focus;
		m_FarDialogItem.Selected = Selected;
		m_FarDialogItem.Flags = Flags;
		m_FarDialogItem.DefaultButton = DefaultButton;
		m_FarDialogItem.MaxLen = 0;
		far_di_item::SetFarDialogItemBuffer(m_pBuffer, pData, m_FarDialogItem);		
	};
// 	tstring_buffer const& GetBufferPtr() const {
// 		return m_pBuffer;
// 	}
	FarDialogItem& GetFarDialogItemRef() {
		return m_FarDialogItem;
	}
	unsigned int GetItemIndex() const {
		return m_ItemIndex;
	}
	static void SetFarDialogItemBuffer(tstring_buffer& destBuffer, wchar_t const* srcData, FarDialogItem& destFarDialogItem) {
		destBuffer.reset(new nf::tautobuffer_char(lstrlenW(srcData) + 1));
		lstrcpyW(&(*destBuffer)[0], srcData);
		destFarDialogItem.PtrData = &(*destBuffer)[0];
	}
private:
	tstring_buffer m_pBuffer; 
	FarDialogItem m_FarDialogItem;
	unsigned int m_ItemIndex;
};

class far_list_di_items : boost::noncopyable {	//array of FAR dialog items
public:
	far_list_di_items(int InitNumItems = 10) {
		m_Items.reserve(InitNumItems);
		m_Buffers.reserve(InitNumItems);
	};

	far_list_di_items& operator << (far_di_item *pT) {
		if (m_Items.size() < pT->GetItemIndex()+1) {
			m_Items.resize(pT->GetItemIndex()+1);
			m_Buffers.resize(m_Items.size());
		}
		m_Items[pT->GetItemIndex()] = pT->GetFarDialogItemRef();
		far_di_item::SetFarDialogItemBuffer(m_Buffers[pT->GetItemIndex()]
			, pT->GetFarDialogItemRef().PtrData
			, m_Items[pT->GetItemIndex()]);
		return *this;
	};
	far_list_di_items& operator<<(UINT) {	//модификатор счетчика высоты
		return *this;
	}
	size_t GetNumberItems() const { 
		return m_Items.size(); }
	FarDialogItem* GetItemsPtr() {
		return &m_Items[0];}
	FarDialogItem&  operator[](int ItemIndex) {
		return m_Items[ItemIndex];}
	void SetFarDialogItemData(int itemIndex, wchar_t const* pData) {
		far_di_item::SetFarDialogItemBuffer(m_Buffers[itemIndex]
			, pData
			, m_Items[itemIndex]);		
	}
private:
	std::vector<FarDialogItem> m_Items;	
	tlist_buffers m_Buffers;
};

class dialogT : boost::noncopyable {
public:
	dialogT(int width
		, int height
		, PluginStartupInfo &PluginInfo
		, int InitNumberItems
		, tstring HelpTopic = L"") 
		: m_DialogItems(InitNumberItems)
		, m_PluginInfo(PluginInfo)
		, m_HelpTopic(HelpTopic)
		, m_Height(height)
		, m_Width(width)
		, m_DialogHandle(0)
	{
	}
	virtual ~dialogT() {
		if (m_DialogHandle != 0) {
			g_PluginInfo.DialogFree(m_DialogHandle);
		}
	}
	bool Execute(int &nChoosenItem) {
		if (m_DialogHandle == 0) {
			g_PluginInfo.DialogFree(m_DialogHandle);
		}
		m_DialogHandle = g_PluginInfo.DialogInit(
			m_PluginInfo.ModuleNumber 
			, -1 
			, -1 
			, m_Width
			, m_Height
			, m_HelpTopic.empty()
				? NULL
				: m_HelpTopic.c_str()
			, m_DialogItems.GetItemsPtr()
			, static_cast<int>(m_DialogItems.GetNumberItems())
			, 0 //reserved
			, 0 //flags
			, &dialogT::dlg_proc
			, reinterpret_cast<LONG_PTR>(this) //param
		);		
		if (m_DialogHandle != INVALID_HANDLE_VALUE) {
			nChoosenItem = g_PluginInfo.DialogRun(m_DialogHandle); //free in destructor
			return nChoosenItem != -1;
		} return false;
	}
protected:
	far_list_di_items& GetDialogItemsRef() {
		return m_DialogItems;
	}
public: //get modified data from dialog after dialog closing
	tstring GetDialogItemValue(int dialogItemId) {
		assert(m_DialogHandle != 0);
		FarDialogItemData fdi;
		fdi.PtrLength = g_PluginInfo.SendDlgMessage(m_DialogHandle, DM_GETTEXTLENGTH, dialogItemId, 0);
		nf::tautobuffer_char buffer(static_cast<nf::tautobuffer_char::size_type>(fdi.PtrLength) + 1);		
		fdi.PtrData = &buffer[0];
		fdi.PtrData[0] = L'0';

		g_PluginInfo.SendDlgMessage(m_DialogHandle, DM_GETTEXT, dialogItemId, reinterpret_cast<LONG_PTR>(&fdi));
		return fdi.PtrData;
	}
	bool IsDialogItemSelected(int dialogItemId) {
		assert(m_DialogHandle != 0);
		return g_PluginInfo.SendDlgMessage(m_DialogHandle, DM_GETCHECK, dialogItemId, 0) == BSTATE_CHECKED;
		//return GetDialogItemsRef()[dialogItemId].Selected != 0;
	}
private:
	static LONG_PTR WINAPI dlg_proc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2) {
		return g_PluginInfo.DefDlgProc(hDlg, Msg, Param1, Param2);
	}
private:
	PluginStartupInfo &m_PluginInfo;
	tstring m_HelpTopic;
	const int m_Height;
	const int m_Width;
	HANDLE m_DialogHandle;

	far_list_di_items m_DialogItems;
};

////////////////////////////////////////////////////////////
// wrappers for different FAR dialog items
////////////////////////////////////////////////////////////
inline far_di_item 
far_di_text(unsigned int ItemIndex, int msgID, int X1, int Y1, int X2
			, DWORD Flags = 0
			, int Focus = 0
			, int Selected = 0
			, int DefaultButton = 0
			, int Y2 = 0) 
{
	return far_di_item(ItemIndex, DI_TEXT, X1, Y1, X2, Y2, Focus, Selected, Flags, DefaultButton, nf::GetMsg(msgID));
};

inline far_di_item
far_di_checkbox(unsigned int ItemIndex, int msgID, int X1, int Y1
				, DWORD Flags = 0
				, int Focus = FALSE
				, int Selected = FALSE
				, int DefaultButton = 0
				, int X2 = 0
				, int Y2 = 0)
{
	return far_di_item(ItemIndex, DI_CHECKBOX, X1, Y1, X2, Y2, Focus, Selected, Flags, DefaultButton, nf::GetMsg(msgID));
};

struct far_di_edit : public far_di_item {
	far_di_edit(unsigned int ItemIndex, int X1, int Y1, int X2
		, const wchar_t* sEditText
		, DWORD Flags = 0
		, int Focus = FALSE
		, wchar_t const* History = NULL
		, int DefaultButton = 0
		, int Y2 = 0
		)
		:  far_di_item(ItemIndex, DI_EDIT, X1, Y1, X2, Y2, Focus
			, 0
			, Flags, DefaultButton, sEditText)
	{
		this->GetFarDialogItemRef().History = History;	//directly specify "history" instead of Selected == 0
	}
};

struct far_di_box : public far_di_item {
	far_di_box(DialogItemTypes dialogItemType, unsigned int ItemIndex, int msgID, int X1, int Y1, int X2, int Y2 
		, DWORD Flags = 0
		, int Focus = 0
		, int Selected = 0
		, int DefaultButton = 0)
		:  far_di_item(ItemIndex, dialogItemType, X1, Y1, X2, Y2, Focus, Selected, Flags, DefaultButton
		, msgID 
		? nf::GetMsg(msgID) 
		: L""
		){}
};

inline far_di_item far_di_button(unsigned int ItemIndex, int msgID, int X1, int Y1
		, DWORD Flags = 0
		, int Focus = FALSE
		, int Selected = FALSE
		, int DefaultButton = FALSE
		, int X2 = 0
		, int Y2 = 0) 
{
	return far_di_item(ItemIndex, DI_BUTTON, X1, Y1, X2, Y2, Focus, Selected, Flags, DefaultButton, nf::GetMsg(msgID));
};


// typedef far_di_boxT<DI_DOUBLEBOX> far_di_doublebox;
// typedef far_di_boxT<DI_SINGLEBOX> far_di_singlebox;
};