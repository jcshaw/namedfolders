/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once
#include <list>
#include <vector>
#include "header.h"

extern const wchar_t CHAR_LEADING_VALUE_ENVVAR;
extern const wchar_t CHAR_LEADING_VALUE_REGKEY;


namespace nf {

namespace Shell {
//функции для поиска всех вариантов каталогов/ярлыков/директорий
//на основе данных указанных в команде пользователя
//первыми должны идти точно совпавшие варианты
//вернуть количество точно совпавших вариантов
	size_t SelectShortcuts(tstring shortcut_pattern, tstring catalog, tshortcuts_list& list);	
//	size_t SelectCatalogsAndShortcuts(wchar_t const *shortcut_pattern, wchar_t const* catalog_pattern, tshortcuts_list& list);

//операции с псевдонимами
	bool InsertShortcut(tshortcut_info const&shortcut, tstring value, bool bOverride);		
	bool ModifyShortcut(tshortcut_info const& from
		, tshortcut_info const &to, tstring * pnew_value = 0);		//обновить свойства псевдонима
	bool DeleteShortcut(tshortcut_info const&shortcut);		//удалить псевдоним
	bool MoveShortcut(tshortcut_info const&shortcut
		, tstring const& new_shortcut_path
		, tshortcut_info &new_shortcut
	);

	bool PrepareMovingShortcut(tshortcut_info const &srcSh, tstring const &new_shortcut_path, tshortcut_info &destSh);
	bool CopyShortcut(tshortcut_info const&shortcut
		, tstring const& new_shortcut_path
		, tshortcut_info &new_shortcut
	);

//////////////////////////////////////////////////////////////////////////
//операции с каталогами	
	namespace Private {
		bool remove_catalog(tstring const& SourceCatalog, tstring const* pTargetCatalog, bool bDeleteSource);
	}

	bool InsertCatalog(tstring catalog, wchar_t const* Parent = L"");		
	inline bool DeleteCatalog(tstring const& SourceCatalog) {
		return Private::remove_catalog(SourceCatalog, 0, true);
	}

	inline bool MoveCatalog(tstring const& SourceCatalog, tstring const& TargetCatalog) {
		return Private::remove_catalog(SourceCatalog, &TargetCatalog, true);
	}

	inline bool CopyCatalog(tstring const& SourceCatalog, tstring const& TargetCatalog) {
		return Private::remove_catalog(SourceCatalog, &TargetCatalog, false);
	}

//получение пути для заданного псевдонима
	bool GetShortcutValue(tshortcut_info const& sh, tstring& DestValue);

} //Shell


}