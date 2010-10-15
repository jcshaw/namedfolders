/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "codec_for_values.h"
#include "strings_utils.h"

using namespace nf;
namespace 
{
	wchar_t const* DEVCHAR_DEPRECATED = L"\n";	
		//первый разделитель значений для двух панелей
		//оставлен для совместимости
	wchar_t const* DEVCHAR = L"|";	
		//новый разделитель значений для двух панелей
	wchar_t const* STR_AMP = L"&";
	wchar_t const* STR_AMP_SCREENED = L"&&";


//заменить спецсимволы экранированными значениями 
	inline tstring get_screened(tstring s) {		
		return Utils::ReplaceStringAll(s, STR_AMP, STR_AMP_SCREENED);
	};
//заменить экранированные символы спец символами
	inline tstring get_unscreened(tstring s) {
		return Utils::ReplaceStringAll(s, STR_AMP_SCREENED, STR_AMP); //!TODO: UT
	}

	nf::tshortcutvalue_type get_value_type(tstring const& value)
	{	//определяем по префиксу тип именованной директории
		if (! value.size()) return nf::VAL_TYPE_LOCAL_DIRECTORY;
		if (value.size() > 2) {
			switch(value[0]) {
			case L'\\':
				if (value[1] == SLASH_DIRS_CHAR) return nf::VAL_TYPE_NET_DIRECTORY;
				break;
			case L'%'://CHAR_LEADING_VALUE_ENVVAR: 
				return nf::VAL_ENVIRONMENT_VARIABLE;
			case L'$': //CHAR_LEADING_VALUE_REGKEY:
				return nf::VAL_REGISTRY_KEY;
			}
		};

		tstring::size_type npos = value.find_first_of(L':');
		if (npos != tstring::npos &&  npos > 1) {
			if (*value.begin() != L'[')	//метасимвол для имени диска... //!TODO: все ли учтено?
				return nf::VAL_TYPE_PLUGIN_DIRECTORY;
		}

		return nf::VAL_TYPE_LOCAL_DIRECTORY;
	}
} 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
tstring nf::EncodeValues(tstring const& FirstPanelValue, tstring const& SecondPanelValue)
{
	if (SecondPanelValue.empty()) return get_screened(FirstPanelValue);
	return get_screened(FirstPanelValue) + DEVCHAR + get_screened(SecondPanelValue);
}

tshortcut_value_parsed_pair nf::DecodeValues(tstring const& Value) {
	tshortcut_value_parsed_pair result;
	size_t n = Value.find(DEVCHAR);
	if (n == tstring::npos) n = Value.find(DEVCHAR_DEPRECATED);	 //support of shortcuts created in 2.0 beta 1.
	if (n == tstring::npos) { 
		result.first.bValueEnabled = true;
		result.first.value = get_unscreened(Value);
		result.first.ValueType = get_value_type(Value);
		result.second.bValueEnabled = false;
	} else {
		tstring v2 = get_unscreened(Value);
		result.first.bValueEnabled = true;
		result.first.value.assign(v2.c_str(), 0, n);
		result.first.ValueType = get_value_type(result.first.value);

		result.second.bValueEnabled = true;
		result.second.value.assign(v2.c_str(), n+1, v2.size());
		result.second.ValueType = get_value_type(result.second.value);
	}
	return result;
}


