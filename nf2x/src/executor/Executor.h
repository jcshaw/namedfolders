/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include "Kernel.h"
#include "codec_for_values.h"
#include "PanelInfoWrap.h"

namespace nf 
{
//��������� ������� (��� ������� ����� �������� ������)
	bool ExecuteCommand(nf::tparsed_command &cmd);

	namespace Commands
	{
	//������� � FAR
		bool OpenShortcut(HANDLE hPlugin
			, nf::tshortcut_info const& sh
			, tstring const &path
			, bool bOpenOnActivePanel = true
			, nf::twhat_to_search_t WhatToSearch = nf::WTS_DIRECTORIES);	//!TODO: remove

		bool OpenShortcut(HANDLE hPlugin
			, nf::tshortcuts_list const& SrcList
			, tstring const &path
			, bool bOpenOnActivePanel = true
			, nf::twhat_to_search_t WhatToSearch = nf::WTS_DIRECTORIES);

		bool OpenShortcutInExplorer(HANDLE hPlugin
			, nf::tshortcut_info const& sh
			, tstring const &path);
		inline bool OpenFile(HANDLE hPlugin
			, nf::tshortcut_info const& sh
			, tstring const &path
			, bool bOpenOnActivePanel = true)
		{
			//��� �������� ���������� ��������� - �� ���� �� ���
			return OpenShortcut(hPlugin, sh, path, bOpenOnActivePanel, nf::WTS_FILES);
		}

	//������� � Explorer
		bool OpenPath(HANDLE hPlugin, tstring const& path);
		void OpenPathInExplorer(tstring const& s);

	//������� ����� ��������� / �������
		bool AddShortcut(HANDLE hPlugin, nf::tshortcut_info const &sh, tstring const& Value, bool bImplicit);
		bool AddShortcut(HANDLE hPlugin, nf::tcatalog_info const &cat, tstring const& sh, bool bTemporary, bool bImplicit);
		bool AddShortcutForBothPanels(HANDLE hPlugin, nf::tcatalog_info const &cat, tstring const& sh, bool bTemporary, bool bImplicit);
		bool AddCatalog(HANDLE hPlugin, nf::tcatalog_info const &cat);


		std::pair<tstring, tstring>
		get_implicit_name_and_value(HANDLE hPlugin = INVALID_HANDLE_VALUE
			, bool bGetDataFromActivePanel = true	//� �������� ��� ���������� ������ ����� ������
		);
		bool IsCatalogIsEmpty(nf::tcatalog_info const& cat);

	//����� ��� �������� ������/���������� ��������� �/��� �����������
		struct Deleter
		{
			Deleter(std::list<nf::tshortcut_info> const& Shortcuts
				, std::list<nf::tcatalog_info> const& Catalogs
				, bool bImplicit = false);
			static int DeleteShortcut(nf::tshortcut_info const&Shortcut, bool bImplicit = false);
			static int DeleteCatalog(nf::tcatalog_info const&Catalog, bool bImplicit = false);

			int Del();
		private:
			std::list<nf::tshortcut_info> const& m_Shortcuts;
			std::list<nf::tcatalog_info> const& m_Catalogs;
			bool m_bImplicit;
		};
	} 
};