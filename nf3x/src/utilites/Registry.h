/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once
#pragma warning(disable: 4290)

#include <cassert>
#include <vector>
#include <boost/utility.hpp>

namespace nf {

class CRegistry : boost::noncopyable {	
	HKEY m_hKey;		
public:
	CRegistry(HKEY hkey
		, wchar_t const * pszSubKey
		, bool bNonVolatile = true
		, bool bCreateIFNotExisted = true
		) throw (DWORD)
	{
		if (ERROR_SUCCESS != RegCreateKeyEx(hkey
			, pszSubKey
			, 0
			, NULL
			, (bNonVolatile) ? REG_OPTION_NON_VOLATILE : REG_OPTION_VOLATILE
			, KEY_ALL_ACCESS
			, NULL
			, &m_hKey
			, NULL)
		) throw 0;
	}
	~CRegistry() {
		RegCloseKey(m_hKey);
	}

	operator HKEY() const {
		return m_hKey;}

public: //get
	DWORD GetValue(tstring const& Key, tstring &str) {
		DWORD size = MAX_PATH * sizeof(wchar_t);	
		DWORD type;
		DWORD ErrorCode; 
		nf::tautobuffer_byte buf(size);

		do {
			buf.resize(size);
			ErrorCode = RegQueryValueEx(m_hKey, Key.c_str(), 0, &type, (LPBYTE)&buf[0], &size);
		} while (ErrorCode == ERROR_MORE_DATA);

		if (! ((ErrorCode == ERROR_SUCCESS) && (type == REG_SZ))) return 0;
		str.assign(reinterpret_cast<wchar_t*>(&buf[0]), size);
		return type;
	}
	bool GetValue(tstring const& Key, DWORD &Value) {
		DWORD type;
		DWORD size = sizeof(Value);
		bool b = (ERROR_SUCCESS == RegQueryValueEx(m_hKey, Key.c_str(), 0, &type, (LPBYTE)&Value, &size));
		return (type == REG_DWORD) && b;
	}

public: //set 
	bool SetValue(wchar_t const* Key, wchar_t const* Value) {
		long error_code = RegSetValueExW(m_hKey
			, Key
			, 0 //reserved
			, REG_SZ
			, reinterpret_cast<BYTE const*>(Value)
			, (lstrlenW(Value) + 1) * sizeof(wchar_t)
		);
		return error_code == ERROR_SUCCESS;
// 		if (b) {
// 			MessageBox(0, L"OK", 0, MB_OK);
// 		} else {
// 			MessageBox(0, L"FALSE", 0, MB_OK);
// 		}
// 		return b;
	}

	bool SetValue(wchar_t const* Key, DWORD Value) {
		return RegSetValueEx(m_hKey
			, Key
			, 0 //reserved
			, REG_DWORD
			, reinterpret_cast<BYTE const*>(&Value)
			, sizeof(Value)
			) == ERROR_SUCCESS;
	}

	bool DeleteValue(tstring const& Key) {
		return RegDeleteValue(m_hKey, Key.c_str()) == ERROR_SUCCESS;
	}
};

} //nf
#pragma warning(default: 4290)