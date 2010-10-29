#include "stdafx.h"
#include ".\dialogeditpattern.h"

#include "strings_utils.h"

using namespace nf;
using namespace Patterns;

namespace 
{
	TCHAR const* HELP_TOPIC_ID = _T("DialogPatterns");
}

DialogEditPattern::DialogEditPattern(void)
: dialogT(DIALOG_WIDTH, DIALOG_HEIGHT, g_PluginInfo, 10, HELP_TOPIC_ID)
, m_bCreateNewPattern(true)
{
	SetDialogItems();
}

DialogEditPattern::DialogEditPattern(tstring const& Prefix, tstring const& Pattern)
: dialogT(DIALOG_WIDTH, DIALOG_HEIGHT, g_PluginInfo, 10, HELP_TOPIC_ID)
, m_bCreateNewPattern(false)
, m_Prefix(Prefix)
, m_Pattern(Pattern)
{
	SetDialogItems();
}


DialogEditPattern::~DialogEditPattern(void)
{
}

UINT DialogEditPattern::ShowModal()
{
	//вызываем диалог редактированик имени каталога
	int nChoosedItem;

	::lstrcpy(m_DialogItems[ID_EDIT_PREFIX].Data, Utils::GetInternalToOem(m_Prefix).c_str());
	::lstrcpy(m_DialogItems[ID_EDIT_PATTERN].Data, Utils::GetInternalToOem(m_Pattern).c_str());
	m_DialogItems[ID_EDIT_PREFIX].Focus = 1;

	if (Execute(nChoosedItem))
		if (nChoosedItem != ID_CANCEL)
		{	//редактируем имя каталога
			m_Prefix = Utils::GetOemToInternal(m_DialogItems[ID_EDIT_PREFIX].Data);
			m_Pattern = Utils::GetOemToInternal(m_DialogItems[ID_EDIT_PATTERN].Data);

			//префикс всегда должен заканчиваться двоеточием
			//если пользователь забыл его указать - доставляем : автоматически
			if (m_Prefix[m_Prefix.size()-1] != _T(':')) m_Prefix += _T(":");
			return 1;
		}

		return 0;
}