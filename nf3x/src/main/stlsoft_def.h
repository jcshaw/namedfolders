/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#pragma warning(disable: 4267 4290)
//используется библиотека STL_SOFT 
//http://www.stlsoft.com
#define _WINSTL_REG_STRING_USE_STD_STRING
#include <winstl/filesystem/findfile_sequence.hpp>
// #include <winstl/registry/reg_key_sequence.hpp>
// #include <winstl/registry/reg_traits.hpp>
// #include <winstl/registry/reg_key_sequence.hpp>
#include <winstl/registry/reg_value_sequence.hpp>
// #include <winstl/registry/reg_value.hpp>
#include <winstl/registry/reg_key.hpp>
#include <winstl/filesystem/findvolume_sequence.hpp>
#pragma warning(default: 4267 4290)

namespace WinSTL {
	typedef winstl::findfile_sequence_w findfile_sequence_t;
	//typedef winstl::findvolume_sequence_w findvolume_sequence_t;
	//typedef winstl::reg_key_sequence_w reg_key_sequence_t;
	typedef winstl::reg_key_w reg_key_t;
	typedef winstl::reg_value_sequence_w  reg_value_sequence_t;
	//typedef winstl::reg_value_w reg_value_t;
}

#include <boost/foreach.hpp>
namespace boost { 
	//possibility to use BOOST_FOREACH with WinSTL::findfile_sequence_t
	//WinSTL::findfile_sequence_t doesn't contain "iterator"
	// specialize range_mutable_iterator and range_const_iterator in namespace boost
	template<>
	struct range_mutable_iterator<WinSTL::findfile_sequence_t> {
		typedef WinSTL::findfile_sequence_t::const_iterator type;
	};
// 
// 	template<>
// 	struct range_const_iterator<WinSTL::findfile_sequence_t> {
// 		typedef WinSTL::findfile_sequence_t::const_iterator type;
// 	};
}

