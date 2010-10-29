/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#pragma once 

//far dialog item's templates

#include <cassert>
#include <vector>
#include <Windows.h>
#include <algorithm>

namespace nf 
{
extern const char* GetMsg(int MsgId);

struct far_di_item
{
	FarDialogItem m_data;
	unsigned int m_ItemIndex;
	far_di_item(unsigned int ItemIndex
		, int ItemType
		, int X1, int Y1, int X2, int Y2
		, int Focus
		, int Selected, 
		DWORD Flags
		, int DefaultButton
		, char const* pData)
		: m_ItemIndex(ItemIndex)
	{
		m_data.Type = ItemType;
		m_data.X1 = X1;
		m_data.Y1 = Y1;
		m_data.X2 = X2;
		m_data.Y2 = Y2;
		m_data.Focus = Focus;
		m_data.Selected = Selected;
		m_data.Flags = Flags;
		m_data.DefaultButton = DefaultButton;
		assert(::lstrlen(pData) < 512);	//size of FarDialogItem.data[] is 512 bytes
		::lstrcpy(m_data.Data, pData);			
	};
};

class far_list_di_items
{	//array of FAR dialog items
	typedef std::vector<FarDialogItem>  tvec;
	tvec m_items;	
public:
	far_list_di_items(int InitNumItems = 10)
		: m_items(InitNumItems)
	{};
	far_list_di_items& operator << (far_di_item *pT)
	{
		if (m_items.size() < pT->m_ItemIndex+1) m_items.resize(pT->m_ItemIndex+1);
		m_items[pT->m_ItemIndex] = pT->m_data;
		return *this;
	};
	far_list_di_items& operator<<(UINT)
	{	//модификатор счетчика высоты
		return *this;
	}
	size_t GetNumberItems() const { return m_items.size(); }
	FarDialogItem* GetItemsPtr() {return &m_items[0];}
	FarDialogItem&  operator[](int ItemIndex) {return m_items[ItemIndex];}
};

class dialogT
{
	PluginStartupInfo &m_PluginInfo;
	tstring m_HelpTopic;
	const int m_height;
	const int m_width;
public:
	far_list_di_items m_DialogItems;
	dialogT(int width
		, int height
		, PluginStartupInfo &PluginInfo
		, int InitNumberItems
		, tstring HelpTopic = _T("")) 
		: m_DialogItems(InitNumberItems)
		, m_PluginInfo(PluginInfo)
		, m_HelpTopic(HelpTopic)
		, m_height(height)
		, m_width(width)
	{
	}
	bool Execute(int &nChoosenItem)
	{
		nChoosenItem = g_PluginInfo.Dialog(
				m_PluginInfo.ModuleNumber 
				,-1 
				,-1 
				,m_width
				,m_height
				,m_HelpTopic.empty()
					? NULL
					: m_HelpTopic.c_str()
				,m_DialogItems.GetItemsPtr()
				,static_cast<int>(m_DialogItems.GetNumberItems())
			);
		return nChoosenItem != -1;
	}
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
		, int Y2 = 0
		)
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
		, int Y2 = 0
		)
{
	return far_di_item(ItemIndex, DI_CHECKBOX, X1, Y1, X2, Y2, Focus, Selected, Flags, DefaultButton, nf::GetMsg(msgID));
};

struct far_di_edit : public far_di_item
{
	far_di_edit(unsigned int ItemIndex, int X1, int Y1, int X2, const char* sEditText
		, DWORD Flags = 0
		, int Focus = FALSE
		, char const* History = NULL
		, int DefaultButton = 0
		, int Y2 = 0
		)
		:  far_di_item(ItemIndex, DI_EDIT, X1, Y1, X2, Y2, Focus
			, 0
			, Flags, DefaultButton, sEditText)
	{
		m_data.History = History;	//directly specify "history" instead of Selected == 0
	}
};

template<int DI_ID>
struct far_di_boxT : public far_di_item
{
	far_di_boxT(unsigned int ItemIndex, int msgID, int X1, int Y1, int X2, int Y2 
		, DWORD Flags = 0
		, int Focus = 0
		, int Selected = 0
		, int DefaultButton = 0
		)
		:  far_di_item(ItemIndex, DI_ID, X1, Y1, X2, Y2, Focus, Selected, Flags, DefaultButton, 
			msgID ? nf::GetMsg(msgID) : "")
	{}
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


typedef far_di_boxT<DI_DOUBLEBOX> far_di_doublebox;
typedef far_di_boxT<DI_SINGLEBOX> far_di_singlebox;

};