/********************************************************************
	created:	1.07.2005   18:31
	filename: 	string_buffer.h
	Copyright 2005 by Victor Derevyanko
	wingspan@yandex.ru
	http://www.rammus.ru
	revision $Id$
*********************************************************************/

#pragma once 

#include <string>
#include <cassert>

//#include <stlsoft_pod_vector.h>
//#include <vector>
#include <stlsoft_auto_buffer.h>

namespace Ext
{
/************************************************************************/
// wrapper around C strings
/************************************************************************/
template<class T>
class string_bufferT
{
	//typedef std::vector<T> tbuffer;	//!TODO: replace by autobuffer
	typedef stlsoft::auto_buffer<T, std::allocator<T> > tbuffer;
	tbuffer m_buffer;
	typedef string_bufferT<T> this_class;
public:
	explicit string_bufferT(size_t Size) 
		: m_buffer(Size) 
	{
		memset(&m_buffer[0], 0, m_buffer.size());
	}

	explicit string_bufferT(string_bufferT<T> const& Buf) 
		: m_buffer(Buf.m_buffer.size()) 
	{ 
		assign(&Buf.m_buffer[0]);
	}

	explicit string_bufferT(std::basic_string<T> const& Str) 
		: m_buffer(Str.size()) 
	{ 
		assign(Str); 
	}

	explicit string_bufferT(T const* CStr) 
		: m_buffer(get_length(CStr)) 
	{ 
		assign(CStr); 
	}

public:	//size
	inline size_t GetStringSize() const { return std::basic_string<T>(&m_buffer[0]).size(); }
	inline DWORD GetBufferSize() const { return static_cast<DWORD>(get_buffer_size()); }

public:	//assignment/resize
	string_bufferT<T>& operator =(string_bufferT<T> const& Buf) { m_buffer = Buf.m_buffer; return *this;}
	string_bufferT<T>& operator=(std::basic_string<T> const& Str) {  assign(Str); return *this;}
	string_bufferT<T>& operator=(T const* CStr ) { assign(CStr); return *this; }
	inline void ResizeBuffer(size_t Size) {  resize_buffer(Size); }
	inline void swap(string_bufferT<T> & Buf) { swap_buffer(Buf.m_buffer); }

public: //convertion
	operator std::basic_string<T> () const {return &m_buffer[0]; }
	operator T*() { return &m_buffer[0]; };
	T const* c_str() const {return &m_buffer[0];}
	std::basic_string<T> str() const {return &m_buffer[0]; }

public:	//add
	string_bufferT& operator +=(std::basic_string<T> const& Str ) 
	{ 
		std::basic_string<T> new_str;
		new_str.reserve(get_buffer_size() + Str.size());
		new_str= &m_buffer[0];
		new_str += Str;
		assign(new_str);
		return *this;
	}
	string_bufferT& operator +=(T const* CStr)		
	{
		std::basic_string<T> new_str;
		new_str.reserve(get_buffer_size());
		new_str = &m_buffer[0];
		new_str += CStr;
		assign(new_str);
		return *this;
	}
	string_bufferT& operator +=(string_bufferT<T> const &Buf)		
	{
		std::basic_string<T> new_str;
		new_str.reserve(get_buffer_size() + Buf.get_buffer_size());
		new_str = &m_buffer[0];
		new_str += &Buf.m_buffer[0];
		assign(new_str.c_str());
		return *this;
	}
private:
	inline void assign(T const* str)
	{
		size_t str_size = get_length(str);
		if (get_buffer_size() < str_size + 1) resize_buffer(str_size + 1);
		lstrcpy(&m_buffer[0], str);
	}
	inline void assign(std::basic_string<T> const& str)
	{
		if (get_buffer_size() < str.size() + 1) resize_buffer(str.size() + 1);
		lstrcpy(&m_buffer[0], str.c_str());
	}
private: //shims
	inline size_t get_buffer_size() {return m_buffer.size();}
	inline void resize_buffer(size_t NewSize) {m_buffer.resize(NewSize);}
	inline void swap_buffer(tbuffer &Buf) {m_buffer.swap(Buf);}
	inline size_t get_length(T const* Str)
	{
		return std::basic_string<T>(Str).size();	//!TODO
	}
};

};