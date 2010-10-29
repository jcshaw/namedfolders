/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "registry_remover.h"
#include <list>
#include <boost/foreach.hpp>
#include <shlwapi.h>

#include "stlsoft_def.h"


//Стандартная функция  SHCopyKeyA перенесена в проект, чтобы исключить
//необходимость в IE 5.0
#define ARRAYSIZE_local(a)    (sizeof(a)/sizeof(a[0]))
STDAPI_(DWORD) SHCopyKeyA_c(HKEY hkeySrc, LPCSTR pszSrcSubKey, HKEY hkeyDest, DWORD fReserved)
{
	HKEY hkeyFrom;
	DWORD dwRet;

	if (pszSrcSubKey) {		
		dwRet = RegOpenKeyExA(hkeySrc, pszSrcSubKey, 0, MAXIMUM_ALLOWED, &hkeyFrom);
	} else if (hkeySrc) {
		dwRet = ERROR_SUCCESS;
		hkeyFrom = hkeySrc;
	} else {
		dwRet = ERROR_INVALID_PARAMETER;
	}

	if (dwRet == ERROR_SUCCESS) {
		DWORD dwIndex;
		DWORD cchValueSize;
		DWORD cchClassSize;
		DWORD dwType;
		CHAR  szValue[MAX_PATH]; //NOTE:szValue is also used to store subkey name when enumerating keys
		CHAR  szClass[MAX_PATH];

		cchValueSize = ARRAYSIZE_local(szValue);
		cchClassSize = ARRAYSIZE_local(szClass);
		for (dwIndex=0; 
			dwRet == ERROR_SUCCESS && (dwRet = RegEnumKeyExA(hkeyFrom, dwIndex, szValue, &cchValueSize, NULL, szClass, &cchClassSize, NULL)) == ERROR_SUCCESS; 
			dwIndex++, cchValueSize = ARRAYSIZE_local(szValue), cchClassSize = ARRAYSIZE_local(szClass))
		{
			HKEY  hkeyTo;
			DWORD dwDisp;

			// create new key
			dwRet = RegCreateKeyExA(hkeyDest, szValue, 0, szClass, REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hkeyTo, &dwDisp);
			if (dwRet != ERROR_SUCCESS)
				break;

			dwRet = SHCopyKeyA(hkeyFrom, szValue, hkeyTo, fReserved); //if not error_success we break out
			RegCloseKey(hkeyTo);
		}

		// copied all the sub keys, now copy all the values
		if (dwRet == ERROR_NO_MORE_ITEMS)
		{
			DWORD  cb;
			DWORD  cbBufferSize;
			LPBYTE lpbyBuffer;

			// get the max value size
			if ((dwRet = RegQueryInfoKey(hkeyFrom, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &cb, NULL, NULL)) == ERROR_SUCCESS)
			{
				// allocate buffer
				cb++; // add 1 just in case of a string
				lpbyBuffer = (LPBYTE)LocalAlloc(LPTR, cb);
				if (lpbyBuffer)
					cbBufferSize = cb;
				else
					dwRet = ERROR_OUTOFMEMORY;

				cchValueSize = ARRAYSIZE_local(szValue);
				for (dwIndex=0;
					dwRet == ERROR_SUCCESS && (dwRet = RegEnumValueA(hkeyFrom, dwIndex, szValue, &cchValueSize, NULL, &dwType, lpbyBuffer, &cb)) == ERROR_SUCCESS;
					dwIndex++, cchValueSize = ARRAYSIZE_local(szValue), cb = cbBufferSize)
				{
					// cb has the size of the value so use it rather than cbBufferSize which is just max size
					dwRet = RegSetValueExA(hkeyDest, szValue, 0, dwType, lpbyBuffer, cb);
					if (dwRet != ERROR_SUCCESS)
						break;
				}

				if (lpbyBuffer != NULL)
					LocalFree(lpbyBuffer);
			}
		}

		if (dwRet == ERROR_NO_MORE_ITEMS)
			dwRet = ERROR_SUCCESS;

		if (pszSrcSubKey)
			RegCloseKey(hkeyFrom);
	}

	return dwRet;
}

STDAPI_(DWORD) SHCopyKeyW_c(HKEY hkeySrc, LPCWSTR pwszSrcSubKey, HKEY hkeyDest, DWORD fReserved) {
	CHAR sz[MAX_PATH];
	if (pwszSrcSubKey) {
		WideCharToMultiByte(CP_ACP, 0, pwszSrcSubKey, -1, sz, ARRAYSIZE_local(sz), NULL, NULL);
	}
	return SHCopyKeyA_c(hkeySrc, pwszSrcSubKey ? sz : NULL, hkeyDest, fReserved);
}

namespace
{
	bool recursive_delete_key(WinSTL::reg_key_t &Key, wchar_t const *SubKey)
	{	//рекурсивно удалить содержимое ключа реестра
		bool bRet = true;
		std::list<tstring> sub_keys;
		WinSTL::reg_key_t rk(Key.get_key_handle(), SubKey);
		WinSTL::reg_key_sequence_t seq(rk);
		BOOST_FOREACH(WinSTL::reg_key_sequence_t::value_type const& key, seq) {
			sub_keys.push_back(key.name().c_str());
		};
		while(! sub_keys.empty()) {
			bRet &= recursive_delete_key(rk,  (*sub_keys.begin()).c_str());
			sub_keys.erase(sub_keys.begin());
		}
		bRet &= ERROR_SUCCESS == ::RegDeleteKey(Key.get_key_handle(), SubKey);
		return bRet;
	}

	bool copy_key(wchar_t const* SrcKey, wchar_t const* TargetKey)
	{	//copy registry key content to memory, store it to memory file and restore in new registry key from the file
		HKEY hKeyTarget;
		if (ERROR_SUCCESS != 
			RegCreateKeyEx(HKEY_CURRENT_USER, TargetKey, 0, NULL, 0
			, KEY_ALL_ACCESS, 0, &hKeyTarget, 0)) 
				return false;
		bool bRet = ERROR_SUCCESS == SHCopyKeyW_c(HKEY_CURRENT_USER, SrcKey, hKeyTarget, 0);

		::RegCloseKey(hKeyTarget);
		return bRet;
	}}

using namespace nf;

inline bool registry_remover::IsKeysOverloaded(tstring const &srcKey, tstring const &targetKey)
{	//ensure that targetKey is not embedded to srcKey
	return false; //!TODO
}

bool registry_remover::remove(tstring const &SrcKey, tstring const*const pTargetKey, bool bDeleteSrc) {
	if (pTargetKey) {
		tstring target_key = *pTargetKey;
		if (IsKeysOverloaded(SrcKey, target_key)) return false;	
		if (! ::copy_key(SrcKey.c_str(), target_key.c_str())) return false;
	}
	if (bDeleteSrc) { //delete source key
		tstring ParentKey;
		tstring KeyName;

		tstring::size_type npos =  SrcKey.find_last_of(L"\\");
		if (npos == tstring::npos) {
			KeyName = SrcKey;
		} else {
			ParentKey.assign(SrcKey.begin(), SrcKey.begin() + npos);
			KeyName.assign(SrcKey.begin() + npos + 1, SrcKey.end());
		}
		WinSTL::reg_key_t Key(HKEY_CURRENT_USER, ParentKey.c_str());	
		if (! ::recursive_delete_key(Key, KeyName.c_str())) return false;
	}
	return true;
}

