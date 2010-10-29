/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#pragma warning(disable: 4267 4290)
//используетс€ библиотека STL_SOFT 
//http://www.stlsoft.com
#define _WINSTL_REG_STRING_USE_STD_STRING
#include <winstl/filesystem/findfile_sequence.hpp>
#include <winstl/registry/reg_key_sequence.hpp>
#include <winstl/registry/reg_traits.hpp>
#include <winstl/registry/reg_key_sequence.hpp>
#include <winstl/registry/reg_value_sequence.hpp>
#include <winstl/registry/reg_value.hpp>
#include <winstl/registry/reg_key.hpp>
#include <winstl/filesystem/findvolume_sequence.hpp>
#pragma warning(default: 4267 4290)

namespace WinSTL
{
// объ€вл€ем типы winstl дл€ TCHAR
#ifdef UNICODE
	typedef winstl::findfile_sequence_w findfile_sequence_t;
	typedef winstl::findvolume_sequence_w findvolume_sequence_t;
	typedef winstl::reg_key_sequence_w reg_key_sequence_t;
	typedef winstl::reg_key_w reg_key_t;
	typedef winstl::reg_value_sequence_w  reg_value_sequence_t;
	typedef winstl::reg_value_w reg_value_t;
#else
	typedef winstl::findfile_sequence_a findfile_sequence_t;
	typedef winstl::findvolume_sequence_a findvolume_sequence_t;
	typedef winstl::reg_key_sequence_a reg_key_sequence_t;
	typedef winstl::reg_key_a reg_key_t;
	typedef winstl::reg_value_sequence_a  reg_value_sequence_t;
	typedef winstl::reg_value_a reg_value_t;
#endif
}

