/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include <vector>
#include "plugin.hpp"

//���������� ���������� � ���������
namespace nf {
	typedef enum tsetting_strings {
		STS_PREFIXES
		, ST_SOFT_MASKS_TO_IGNORE_COMMA_SEPARETED	//������ ����� ������������ "cd: soft"
		, STS_PANELWIDTH //��������� ������ ������ � ���� N,Z,CO;10,0,10
		, ST_ASTERIX_MODE	//����� ��������������� ���������� ����� �����������: 0 - *a*, 1 - a*, 2 - a (�� ���������)

		, NUMBER_STRING_SETTINGS 
	} _tsetting_strings;

	typedef enum tsetting_flags {	//����� ��������
		ST_SHOW_ERRORS					//���������� ������
		, ST_SHOW_IN_PLUGINS_MENU		//���������� �������� ������� � ���� ��������
		, ST_SHOW_IN_DISK_MENU			//���������� �������� ������� � ���� ������
		, ST_SHOW_TEMPORARY_AS_HIDDEN	//���������� ��������� ������� ��� ������� �����
		, ST_CONFIRM_DELETE				//������������ �������� ����������� 
		, ST_CONFIRM_DELETE_CATALOGS	//������������ �������� ���������
		, ST_CONFIRM_DELETE_NOT_EMPTY_CATALOGS //������������ �������� �������� ���������
		, ST_CONFIRM_OVERRIDE			//������������ ���������� �������� ����������
		, ST_CONFIRM_IMPLICIT_CREATION	//������������ �������� ���������� �� ������� cd::, cd:+
		, ST_CONFIRM_IMPLICIT_DELETION	//������������ �������� ���������� �� ������� cd:-
		, ST_CONFIRM_GO_TO_NEAREST		//������������ ������� � ��������� ����������
		, ST_SHOW_KEYS_IN_MENU		//���������� � ���� ��������� ������
		, ST_SELECT_SH_MENU_MODE	//����� ����������� ���� �� ������� ��������� �����������
		, ST_PANEL_MODE				//����� ����������� ������
		, ST_SELECT_CATS_MENU_MODE	//����� ����������� ���� �� ������� ��������� ���������
		, ST_ALWAYS_EXPAND_SHORTCUTS	//������ ��������� ������������� ����������
		, ST_HISTORY_IN_DIALOG_APPLY_COMMAND	//������������ ������� � ������� ���������� �������
		, ST_SELECT_SH_MENU_MODE_EV		//����� ����������� ���� �� ������� ���������� �����
		, ST_SELECT_SH_MENU_SHOWCATALOGS_MODE // ����� ����������� ��������� � ���� ���������
		//, ST_EDIT_MENU_DISK_FAST_KEY // ������� �������� ������� � ���� ������
		, ST_FLAG_NETWORK_COMMANDS_THROUGH_COMMAND_LINE //���������� � ������� ���������� � ������� ������� Network
		, ST_SELECT_SOFT_MENU_SHOWCATALOGS_MODE // ����� ����������� ��������� � ���� ������� ��������
		, ST_USE_SINGLE_MENU_MODE
		, ST_SUBDIRECTORIES_AS_ALIASES

		//NF 3.0
		, ST_SHOW_CATALOGS_IN_DISK_MENU //allow to show list of catalogs in disk menu
		, ST_ALLOW_ABBREVIATED_SYNTAX_FOR_DEEP_SEARCH //disable possibility to use "\\\" instead of "\*\*\*", "...." instead of "\..\.." and use "cd:a.." instead of "cd:a\.."
		// ��� ���������� - �������� �������� �� ��������� � default_flags_values

		, NUMBER_FLAG_SETTINGS 
	} _tflag;

	class CSettings {
	private:
		CSettings(void);
		~CSettings(void);
	public:
		static CSettings& GetInstance() {
			static CSettings st;
			return st;
		}
		void ReloadSettings();
		void SaveSettings();
	public: //�������� �������
		tstring GetPrimaryPluginPrefix();
		tstring const& GetListPrefixes();
	public: //������� �������� ���������� ����� ���������
		DWORD GetValue(tsetting_flags fg) const;	
		tstring const& GetValue(tsetting_strings fg) const;
		tstring const& get_NamedFolders_reg_key() const { return m_nf_reg_key;}
	public: //������ �������� ���������� ����� ��������� (� ������������� �������)
		void SetValue(tsetting_flags fg, DWORD Value);	
		void SetValue(tsetting_strings fg, tstring Value);
	private:	//members
		nf::tautobuffer_byte m_FV;	//flag values
		nf::tvector_strings m_SV;	//string values
		tstring m_nf_reg_key;		//���� � ������� �������� ��������� NF
		tstring m_plugin_prefixes;	//�������� �������
		tstring m_FullListPrefixes;
	};

	inline const wchar_t *GetMsg(int MsgId) {
		return g_PluginInfo.GetMsg(g_PluginInfo.ModuleNumber, MsgId);
	}
	inline tstring GetRegistryKeyForCommandPatterns() {
		return CSettings::GetInstance().get_NamedFolders_reg_key() + tstring(L"\\CommandPatterns");
	}};
