/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

namespace nf 
{
	HANDLE OpenFromPluginsMenu();
	std::pair<HANDLE, bool>  OpenFromCommandLine(wchar_t const* pCmd );
}