/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

namespace nf 
{
	HANDLE OpenFromPluginsMenu();
	std::pair<HANDLE, bool>  OpenFromCommandLine(TCHAR const* pCmd );
}