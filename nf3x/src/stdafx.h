/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _FAR_USE_FARFINDDATA

#define _CRT_SECURE_NO_DEPRECATE //Currently, STLSoft does not work with the "Safe String Library". 
#define _SCL_SECURE_NO_WARNINGS

#pragma warning (disable: 4290) //C++ exception specification ignored except to indicate a function is not __declspec(nothrow)

#include "plugin.hpp" // этот файл хочет 1-byte alignment
#include "farkeys.hpp"
#include <windows.h>
#include <wtypes.h>
#include <WinDef.h>
#include "header.h"

#include <crtdbg.h>
