#include "stdafx.h"
#include ".\dialogeditpattern.h"

#include "strings_utils.h"

using namespace nf;
using namespace Patterns;

namespace 
{
	wchar_t const* HELP_TOPIC_ID = L"DialogPatterns";
}

DialogEditPattern::DialogEditPattern(void)
: dialogT(nf::NF_DIALOG_EDIT_PATTERN, DIALOG_WIDTH, DIALOG_HEIGHT, g_PluginInfo, 10, HELP_TOPIC_ID)
, m_bCreateNewPattern(true)
{
	SetDialogItems();
}

DialogEditPattern::DialogEditPattern(tstring const& Prefix, tstring const& Pattern)
: dialogT(nf::NF_DIALOG_EDIT_PATTERN, DIALOG_WIDTH, DIALOG_HEIGHT, g_PluginInfo, 10, HELP_TOPIC_ID)
, m_bCreateNewPattern(false)
, m_Prefix(Prefix)
, m_Pattern(Pattern)
{
	SetDialogItems();
}


DialogEditPattern::~DialogEditPattern(void) {
}

UINT DialogEditPattern::ShowModal() {
	//вызываем диалог редактирования имени каталога
	GetDialogItemsRef().SetFarDialogItemData(ID_EDIT_PREFIX, m_Prefix.c_str());
	GetDialogItemsRef().SetFarDialogItemData(ID_EDIT_PATTERN, m_Pattern.c_str());
	//GetDialogItemsRef()[ID_EDIT_PREFIX].Focus = 1; //!TODO

	int nChoosedItem;
	if (Execute(nChoosedItem))
		if (nChoosedItem != ID_CANCEL)
		{	//редактируем имя каталога
			m_Prefix =  this->GetDialogItemValue(ID_EDIT_PREFIX);
			m_Pattern =  this->GetDialogItemValue(ID_EDIT_PATTERN);

			//префикс всегда должен заканчиваться двоеточием
			//если пользователь забыл его указать - доставляем : автоматически
			if (m_Prefix[m_Prefix.size()-1] != L':') m_Prefix += L":";
			return 1;
		}

		return 0;
}