/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
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

//#define USE_BOOST_XPRESSIVE
//#define USE_STLPORT

//stlport
#ifdef USE_STLPORT
#define _STLP_HAS_NO_NEW_C_HEADERS 
#define _STLP_VENDOR_TERMINATE_STD
#endif

#define NF_FAR_API_3
//#define NF_FAR_API_2

#ifdef NF_FAR_API_3
#include "far3/plugin.hpp" 
#else 
#include "far2/plugin.hpp" // этот файл хочет 1-byte alignment
#include "far2/farkeys.hpp"
#endif


#pragma warning (disable: 4290) //C++ exception specification ignored except to indicate a function is not __declspec(nothrow)

#include <windows.h>
#include <wtypes.h>
#include <WinDef.h>
#include "header.h"
#include "far_utils.h" //!TODO: remove

#include <crtdbg.h>
