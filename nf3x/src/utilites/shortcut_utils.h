/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once
#include "header.h"

namespace nf {
//encode two shortcut values (for both paneles) to single string
	tstring EncodeValues(tstring const& ValueFirstPanel
		, tstring const& ValueSecondPanel);

//decode value of shortcut, determine type of directory for each panel
	tshortcut_value_parsed_pair DecodeValues(tstring const& Value);

	nf::tshortcut_info MakeShortcut(tstring const& srcCatalog
		, tstring const& srcShortcut, bool bTemporary); 
}
