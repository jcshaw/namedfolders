#pragma once

#include <Windows.h>

namespace nf {
	namespace Registry {
		inline bool IsSubkeyExist(HKEY srcHkey, wchar_t const* subKeyName) {
			HKEY hkey;
			if (ERROR_SUCCESS != RegOpenKeyEx (srcHkey
				, subKeyName
				, 0
				, KEY_READ 
				, &hkey)) {
					return false;
			} else {
				::RegCloseKey(hkey);
				return true;
			}
		}
		
	}
}