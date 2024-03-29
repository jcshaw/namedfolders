/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "registry_remover.h"
#include <list>
#include <shlwapi.h>
#include "stlsoft_def.h"


//����������� �������  SHCopyKeyA ���������� � ������, ����� ���������
//������������� � IE 5.0
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
STDAPI_(DWORD) SHCopyKeyA_c(HKEY hkeySrc, LPCSTR pszSrcSubKey, HKEY hkeyDest, DWORD fReserved)
{
	HKEY hkeyFrom;
	DWORD dwRet;

	if (pszSrcSubKey) dwRet = RegOpenKeyExA(hkeySrc, pszSrcSubKey, 0, MAXIMUM_ALLOWED, &hkeyFrom);
	else if (hkeySrc) {
		dwRet = ERROR_SUCCESS;
		hkeyFrom = hkeySrc;
	} else dwRet = ERROR_INVALID_PARAMETER;

	if (dwRet == ERROR_SUCCESS)
	{
		DWORD dwIndex;
		DWORD cchValueSize;
		DWORD cchClassSize;
		DWORD dwType;
		CHAR  szValue[MAX_PATH]; 
		CHAR  szClass[MAX_PATH];

		cchValueSize = ARRAYSIZE(szValue);
		cchClassSize = ARRAYSIZE(szClass);
		for (dwIndex=0; 
			dwRet == ERROR_SUCCESS && (dwRet = RegEnumKeyExA(hkeyFrom, dwIndex, szValue, &cchValueSize, NULL, szClass, &cchClassSize, NULL)) == ERROR_SUCCESS; 
			dwIndex++, cchValueSize = ARRAYSIZE(szValue), cchClassSize = ARRAYSIZE(szClass))
		{
			HKEY  hkeyTo;
			DWORD dwDisp;

				dwRet = RegCreateKeyExA(hkeyDest, szValue, 0, szClass, REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hkeyTo, &dwDisp);
			if (dwRet != ERROR_SUCCESS) break;

			dwRet = SHCopyKeyA_c(hkeyFrom, szValue, hkeyTo, fReserved); //if not error_success we break out
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
				if (lpbyBuffer) cbBufferSize = cb;
				else dwRet = ERROR_OUTOFMEMORY;

				cchValueSize = ARRAYSIZE(szValue);
				for (dwIndex=0;
					dwRet == ERROR_SUCCESS && (dwRet = RegEnumValueA(hkeyFrom, dwIndex, szValue, &cchValueSize, NULL, &dwType, lpbyBuffer, &cb)) == ERROR_SUCCESS;
					dwIndex++, cchValueSize = ARRAYSIZE(szValue), cb = cbBufferSize)
				{
					// cb has the size of the value so use it rather than cbBufferSize which is just max size
					dwRet = RegSetValueExA(hkeyDest, szValue, 0, dwType, lpbyBuffer, cb);
					if (dwRet != ERROR_SUCCESS) break;
				}

				if (lpbyBuffer != NULL) LocalFree(lpbyBuffer);
			}
		}

		if (dwRet == ERROR_NO_MORE_ITEMS) dwRet = ERROR_SUCCESS;
		if (pszSrcSubKey) RegCloseKey(hkeyFrom);
	}

	return dwRet;
}


namespace
{
	bool recursive_delete_key(WinSTL::reg_key_t &Key, TCHAR const *SubKey)
	{	//���������� ������� ���������� ����� �������
		bool bRet = true;
		std::list<tstring> sub_keys;
		WinSTL::reg_key_t rk(Key.get_key_handle(), SubKey);
		WinSTL::reg_key_sequence_t seq(rk);
		WinSTL::reg_key_sequence_t::const_iterator p = seq.begin();
		while (p != seq.end())
		{
			sub_keys.push_back((*p).name().c_str());
			++p;
		};
		while(! sub_keys.empty())
		{
			bRet &= recursive_delete_key(rk,  (*sub_keys.begin()).c_str());
			sub_keys.erase(sub_keys.begin());
		}

		bRet &= ERROR_SUCCESS == ::RegDeleteKey(Key.get_key_handle(), SubKey);
		return bRet;
	}
	bool copy_key(TCHAR const* SrcKey, TCHAR const* TargetKey)
	{	//����������� ���������� ����� �������
		//��������� ���������� � ���� (� ������)
		//� ��������������� � ����� ����� �� �����
		HKEY hKeyTarget;
		if (ERROR_SUCCESS != 
			RegCreateKeyEx(HKEY_CURRENT_USER, TargetKey, 0, NULL, 0
			, KEY_ALL_ACCESS, 0, &hKeyTarget, 0)) 
				return false;

		bool bRet = 
			ERROR_SUCCESS == SHCopyKeyA_c(HKEY_CURRENT_USER, SrcKey, hKeyTarget, 0);

		::RegCloseKey(hKeyTarget);
		return bRet;
	}

}

using namespace nf;

inline bool registry_remover::IsKeysOverloaded(tstring const &SrcKey, tstring const &TargetKey)
{	//���������, ����� ������� ���� �� ��� ������ � �������� ����
	//!TODO
	return false;
}

bool registry_remover::remove(tstring const &SrcKey
							  , tstring const*const pTargetKey
							  , bool bDeleteSrc)
{
	if (pTargetKey)
	{
		tstring target_key = *pTargetKey;
	//����� �� ������������
		if (IsKeysOverloaded(SrcKey, target_key)) return false;	
		if (! ::copy_key(SrcKey.c_str(), target_key.c_str())) return false;
	}

//������� �������� ����
	if (bDeleteSrc)
	{
		tstring ParentKey;
		tstring KeyName;

		tstring::size_type npos =  SrcKey.find_last_of(_T("\\"));
		if (npos == tstring::npos)
		{
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

