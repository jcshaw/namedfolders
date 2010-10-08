/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once
#include "stlsoft_def.h"
#include "Kernel.h"
#include "strings_utils.h"
#include "settings.h"
#include "registry_functions.h"

//������ � ���������� � ����� stl
namespace nf {
namespace sc {

typedef WinSTL::reg_value_sequence_t shortcuts_sequence;
typedef WinSTL::reg_key_sequence_t subcatalogs_sequence;

template<class V>
class sequence_item
{
	V m_value;
public:
	sequence_item(){}

	sequence_item(V const &value) : m_value(value){}
	sequence_item(sequence_item<V> const& s) : m_value(s.m_value) {}

	sequence_item& operator=(V const& value)	{m_value = value;}
	tstring GetValue() const { return m_value.value_sz().c_str();}	//!TODO: ������������� ������ ��� V = winstl::reg_value_t 
	tstring GetName() const { return m_value.name().c_str();}
};
typedef sequence_item<WinSTL::reg_value_t> shortcuts_sequence_item;
typedef sequence_item<WinSTL::reg_key_t> catalogs_sequence_item;
		
class CCatalog 
{
	typedef WinSTL::reg_key_t basic_class;
	typedef enum tregs_enum	{REG_STATIC_KEYS, REG_TEMP_KEYS, REG_SUB_CATALOGS, REG_B_SUB_CATALOGS_B};
public:
	CCatalog(); //default - root NF-catalog
	CCatalog(CCatalog const &catalog); 
	CCatalog(tstring SubCatalog, CCatalog const *pParent = 0, bool bCreateIfNotExists = true); //constructor for subcatalog of catalog

//����������� ��� �������� ������� ������ ��������� ������������������ ���������
	CCatalog(sc::catalogs_sequence_item &c, CCatalog const *pParent);
public:
	inline tstring const& CatalogPath() const {//���� � �������� ������������ ��������� ��������	
		return m_CatalogPath;
	}
	tstring GetCatalogRegkey() const;
	inline size_t GetNumberSubcatalogs() const {
		if (! nf::Registry::IsSubkeyExist(m_key.get_key_handle(), GetKeyName(REG_SUB_CATALOGS))) return 0;

		WinSTL::reg_key_t c(m_key.get_key_handle(), GetKeyName(REG_SUB_CATALOGS));
		return WinSTL::reg_key_sequence_t(c).size();
	}

	inline size_t GetNumberShortcuts() const {
		if (! nf::Registry::IsSubkeyExist(m_key.get_key_handle(), GetKeyName(REG_STATIC_KEYS))) return 0;

		WinSTL::reg_key_t c(m_key.get_key_handle(), GetKeyName(REG_STATIC_KEYS));
		return WinSTL::reg_value_sequence_t(c).size();
	}
	//size_t number_temporary_shortcuts() const;

public:	//�������� � ��������������������
	inline basic_class GetSequenceShortcuts(bool bTemporary) //������������������ ��������� � ������� �����������
	{
		wchar_t const* subkey = GetKeyName(bTemporary ? REG_TEMP_KEYS : REG_STATIC_KEYS);
		if (m_key.has_sub_key(subkey)) {
			return basic_class(m_key.get_key_handle(), subkey);
		} else {
			return m_key.create_sub_key(subkey);
		}
	}
	inline basic_class GetSequenceSubcatalogs()//������������������ ��������� � ������� ������������
	{
		wchar_t const* subkey = GetKeyName(REG_SUB_CATALOGS);
		if (m_key.has_sub_key(subkey)) {
			return basic_class(m_key.get_key_handle(), subkey);
		} else {
			return m_key.create_sub_key(subkey);
		}
	}

public: //�������� ��������/��������/��������� � ��������� �������� ���������
	bool SetShortcut(tstring const& Name, tstring const& Value, bool bTemporary);
	bool InsertSubcatalog(tstring const& Name); //�������� ����������
	bool DeleteShortcut(tstring const& Name, bool bTemporary);
	bool DeleteSubcatalog(tstring const& Name);
	bool GetShortcutInfo(tstring const& AnsiName, bool bTemporary, tstring &Value);
private: 
	tstring get_combined_path(wchar_t const* catalog, CCatalog const *parent = 0); //�������� ������ ���� � �������� ������������ ��������� ����
	inline static tstring const& get_far_reg_key() 
	{ 
		return nf::CSettings::GetInstance().get_NamedFolders_reg_key();
	}
	static wchar_t const* GetKeyName(tregs_enum Index)
	{
		static wchar_t const* regs[] = {L"keys", L"tempkeys", L"Catalogs", L"\\Catalogs\\"};
		return regs[static_cast<int>(Index)];
	};
private: //members
	basic_class m_key;
	tstring m_CatalogPath;
};
} //sc

} //nf