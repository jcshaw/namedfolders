/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "executor_addons.h"
#include "stlsoft_def.h"

#include <cassert>
#include <vector>
#include <list>
#include <functional>
#include <shlwapi.h>

#include "header.h"
#include "strings_utils.h"

#include "executor.h"
#include "Kernel.h"
#include "stlcatalogs.h"
#include "menu2.h"
#include "lang.h"
#include "codec_for_values.h"
#include "confirmations.h"
#include "Parser.h"
#include "select_variants.h"

using namespace nf;

namespace 
{
	void GetListPairsForEnvVar(tstring const &VarPattern, std::list<std::pair<tstring, tstring> > &ListPairs
		, tstring &DestLocalPath)
	{	//�������� ������ <��� ���������� �����, ����>, ��������������� ������� VarPattern
		//���� VarPattern ������ � ���� %VarPattern% ��� %VarPattern �� 
		//��������� VarPattern � ���������� ����� ���������� NF

		tstring variable_name;

		//���������� ��� ����������, ��������� ����
		//� ��� ��, ������ �� ��� ���������� ���������, ��� ��� �����
		//���� ��� ���������� ������ ��� ��������� % �� ��� ����� (��� ���������� ������� ParseEnvVarPath)
		//���� �������� % ����, �� � ����� ���� ����������� - ��� ���� �����, ������ �������� ��������� � ��� ��������� �� ����
		//����� ��� ������ ��� ����������
		bool bIncompleteVarPatternPattern  = ! nf::Parser::ParseEnvVarPath(VarPattern, variable_name, DestLocalPath);
		bool bMetachars = nf::Parser::IsContainsMetachars(VarPattern); //� ����� ���� �����������

		tstring& mask = (bIncompleteVarPatternPattern && (! bMetachars))
			? //tstring(L"*") +  //��� ������������� ������ �� ������� "*�����*", ������ "�����*" ������� �������������
			variable_name + tstring(L"*")
			: variable_name;

		//�������� �������� ���� ���������� �����, ������� ������������� ������� VarPattern
		//���������� ������ ���� ���������� �� ������� ��������� ��������� ���������� �����
		wchar_t **penv = _wenviron;
		while (*penv != NULL) {
			tstring senv = *penv;
			std::size_t pos = senv.find(L"=");
			if (pos != tstring::npos) {
				assert(pos+1 < senv.size());
				tstring env_name(senv.c_str(), pos);
				tstring pe(senv.c_str(), pos+1, senv.size() - pos);

				if (nf::Parser::IsTokenMatchedToPattern(env_name, mask, false)) {
					std::list<tstring> list_tokens;
					Utils::SplitStringByRegex(pe, list_tokens, L";");
					for (std::list<tstring>::iterator beg = list_tokens.begin(); beg != list_tokens.end(); ++beg)
						if (::PathFileExists(beg->c_str()) && ::PathIsDirectory(beg->c_str())) {
							ListPairs.push_back(std::make_pair(env_name, *beg));	//CHAR_LEADING_VALUE_ENVVAR
						}
				} else 
					if (bIncompleteVarPatternPattern) 
					{	//��� ���������� ������ �� ���������
						//������� ���� ����� �� ������ � ����� ����������, 
						//�� � � �����������, �� ������� ��� ���������			
						std::list<tstring> list_tokens;
						Utils::SplitStringByRegex(pe, list_tokens, L";");
						for (std::list<tstring>::iterator beg = list_tokens.begin(); beg != list_tokens.end(); ++beg)
							if (nf::Parser::IsTokenMatchedToPattern(*beg, mask, false))
								if (::PathFileExists(beg->c_str()) && ::PathIsDirectory(beg->c_str())) {
									ListPairs.push_back(std::make_pair(env_name, *beg));
								}
					}
			}
			++penv;
		}
	};

	bool decode_reg_key(tstring const& path
		, HKEY &hkey
		, tstring &SubKeyPath)
	{
		//$HKEY_CURRENT_USER\Software ->  hkey=HKEY_CURRENT_USER, SubKeyPath=Software
		hkey = 0;
		size_t npos = min(static_cast<unsigned int>(path.find_first_of(SLASH_DIRS)), 
			static_cast<unsigned int>(path.find_first_of(SLASH_CATS)));
		if (npos == tstring::npos) return false;
		SubKeyPath.assign(path, npos+1, path.size() - npos);

		tstring hkey_path(path, 0, npos);
		assert(hkey_path.size());
		if (*hkey_path.begin() == CHAR_LEADING_VALUE_REGKEY) hkey_path.erase(hkey_path.begin());
		if (! lstrcmpi(hkey_path.c_str(), L"HKEY_CURRENT_USER")) hkey = HKEY_CURRENT_USER;
		else if (! lstrcmpi(hkey_path.c_str(), L"HKEY_LOCAL_MACHINE")) hkey = HKEY_LOCAL_MACHINE;

		return 0 != hkey;
	}

	int append_if(std::list<std::pair<tstring, tstring> > &DestList, tstring SubkeyPatternName, WinSTL::reg_value_sequence_t::value_type const& Value)
	{	//���� ������� � ����: ��� �����, ����
		//���� ������ SubkeyPatternName, ��������� ������ �� ����������, ���� ������� 
		//������������� ������� SubkeyPatternName*
		if (! SubkeyPatternName.empty()) SubkeyPatternName += L"*";
		if (SubkeyPatternName.empty() || nf::Parser::IsTokenMatchedToPattern(Value.name(), SubkeyPatternName, true))
		{
			DestList.push_back(std::make_pair(Value.name(), Value.value_expand_sz())); //CHAR_LEADING_VALUE_ENVVAR
		}
		return 0;	
	}

	void GetListPairsForRegKey(tstring const &RegKeyName
		, tstring const& SubkeyPatternName
		, std::list<std::pair<tstring, tstring> > &DestListPaths)
	{	//�������� ������ <���� �������, ����>, ��������������� ������� KeyPattern
		tstring subkey;
		HKEY hkey;
		if (! decode_reg_key(RegKeyName, hkey, subkey)) return;		//���-�� ������������ �� �� �������...

		WinSTL::reg_key_t c(hkey, subkey.c_str());
		WinSTL::reg_value_sequence_t s(c);
		std::for_each(s.begin(), s.end(), boost::bind(append_if, boost::ref(DestListPaths), boost::ref(SubkeyPatternName),_1) );
	}

	void copy_list_paths(std::list<std::pair<tstring, tstring> > const& SrcList
		, std::list<tstring> &DestList
		, tstring (*f)(std::pair<tstring, tstring> const& ))
	{
		for (std::list<std::pair<tstring, tstring> >::const_iterator p = SrcList.begin(); p != SrcList.end(); ++p)
		{	
			DestList.push_back(f(*p));
		}
	}

}	//namespace

namespace
{
	tstring get_pair_second(std::pair<tstring, tstring> const& t) {return t.second;}
	tstring get_evar_packed_string(std::pair<tstring, tstring> const& t)
	{
		return t.first + CHAR_LEADING_VALUE_ENVVAR + t.second;
	}
	tstring get_regkey_packed_string(std::pair<tstring, tstring> const& t)
	{
		return t.first + CHAR_LEADING_VALUE_ENVVAR//CHAR_LEADING_VALUE_REGKEY 
			+ t.second;
	}
}


void nf::Selectors::GetAllPathForEnvvar(HANDLE hPlugins
										, tstring const &VarName
										, std::list<tstring> &DestListPaths)
{	//�������� ������ ������ ��������� ����������� ���������� 
	std::list<std::pair<tstring, tstring> > list_var_paths;
	tstring additional_local_path; //!TODO: ��� ������ � ���� ���������?
	::GetListPairsForEnvVar(VarName, list_var_paths, additional_local_path);

	//�������� ���������� ��� �� �����, ������ ����
	copy_list_paths(list_var_paths, DestListPaths, get_pair_second);

	//�� ������ ����� ��������� ������������� �������� 
	DestListPaths.sort(Utils::CmpStringEqualCI());
	DestListPaths.unique(Utils::CmpStringEqualCI());
}


void nf::Selectors::GetAllPathForRegKey(HANDLE hPlugins
										, tstring const &RegKeyName
										, tstring const &VarName
										, std::list<tstring> &DestListPaths)
{	//�������� ������ ������ ��������� ����������� ���������� 	
	//	Utils::RemoveLeadingChars(LocalPath, SLASH_DIRS_CHAR);
	//	if (LocalPath.size()) LocalPath = L"*" + LocalPath;
	std::list<std::pair<tstring, tstring> > list_var_paths;
	::GetListPairsForRegKey(RegKeyName, VarName, list_var_paths);

	copy_list_paths(list_var_paths, DestListPaths, get_pair_second);
}



//////////////////////////////////////////////////////////////////////////

bool nf::Selectors::OpenEnvVar(HANDLE hPlugin
							   , tstring const &VarName
							   , tstring const &LocalPath)
{
	tstring result_path;
	if (! nf::Selectors::GetPathByEnvvarPattern(hPlugin, VarName, LocalPath, result_path)) return false;
	//��������� ���������� �� ������ �������
	return nf::Commands::OpenPath(hPlugin, result_path);			
};

//////////////////////////////////////////////////////////////////////////
bool nf::Selectors::GetPathByEnvvarPattern(HANDLE hPlugin
										   , tstring const &VarName
										   , tstring const &LocalPath
										   , tstring &DestPath)
{
	std::list<std::pair<tstring, tstring> > list_var_paths;
	tstring additional_local_path;
	::GetListPairsForEnvVar(VarName, list_var_paths, additional_local_path);
	if (! LocalPath.empty()) {
		additional_local_path = additional_local_path.empty()
			? LocalPath
			: Utils::CombinePath(additional_local_path, LocalPath, L"\\");
		}

	if (list_var_paths.size() == 1) {
		DestPath = (*list_var_paths.begin()).second;
	} else {
		std::pair<tstring, tstring> result;
		if (! Menu::SelectEnvVar(list_var_paths, result)) return false;
		DestPath = result.second;
	}
	if (! additional_local_path.empty()) DestPath = Utils::CombinePath(DestPath, additional_local_path, L"\\");
	return true;
};

bool nf::Selectors::GetPathByRegKey(HANDLE hPlugins
									, tstring const &RegkeyName //���� �������, ���������� ����������, ���������� ��������� ����
									, tstring LocalPath
									, tstring &result_path	//��������� ������������� ���������� �� ���� ��������� ����������
									)
{
	std::list<std::pair<tstring, tstring> > list_var_paths;
	::GetListPairsForRegKey(RegkeyName
		, LocalPath	//!TODO: ����� ������ ������ ��� �� localpath
		, list_var_paths);

	std::pair<tstring, tstring> result;
	if (! Menu::SelectPathByRegKey(list_var_paths, result)) return false;
	result_path = result.second;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//������� �������� ���������� ��������� �� ������ ���������
//����� �� ������ ����������, �� ������� ��������� ���������...
bool nf::Selectors::GetShortcutByPathPattern(HANDLE hPlugin
											 , tstring const &current_path
											 , nf::tparsed_command const &cmd
											 , nf::tshortcut_info& sh)
{
	//!TODO
	while (true)
	{
		nf::tshortcuts_list list;

		switch(cmd.kind) {
		case nf::QK_DELETE_SHORTCUT_IMPLICIT:
			Shell::SelectShortcutsByPath(L""
				, current_path.c_str()
				, list
				, true);
			break;
		case nf::QK_OPEN_BY_PATH:
			Shell::SelectShortcutsByPath(cmd.catalog.c_str()
				, cmd.shortcut.c_str()
				, list
				, false);
			break;
		default:
			return false;
		} 
		int nCodeRet = Menu::SelectShortcut(list, sh);
		if (nCodeRet == -1)
		{
			//������� �������� ���������..
			nf::Commands::Deleter::DeleteShortcut(sh, false);
			continue;
		} else return nCodeRet != 0;
	}; //while

}

