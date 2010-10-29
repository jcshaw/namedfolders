/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#pragma once
#include "header.h"

//кодирование/декодирование значений псевдонимов
namespace nf 
{
//закодировать два значения (для обоих панелей) в одну строку
	tstring EncodeValues(tstring const& ValueFirstPanel
		, tstring const& ValueSecondPanel);

//декодировать значение псевдонима, определить типы директорий для каждой из панелей
	tshortcut_value_parsed_pair DecodeValues(tstring const& Value);

}
