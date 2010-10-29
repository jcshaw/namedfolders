/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/


#pragma once
#pragma warning(disable: 4800)

#include "header.h"
#include "Kernel.h"
#include "Parser.h"

namespace Start 
{
	class CStartSoftShortcut
	{
		tstring m_RootPathOem;
		tstring m_Pattern;
		tstring m_PatternDir;
		nf::tshortcuts_list& m_Data;
		int m_bTemporaryValue;	//по значению Temporary различаем пути, к которым принадлежат €рлыки
		
	public:
		CStartSoftShortcut(TCHAR const* RootPathInt
			, TCHAR const* Pattern
			, TCHAR const* PatternDir
			, nf::tshortcuts_list& Data
			, int bTemporaryValue);
		~CStartSoftShortcut(void);
	private:
		void load_data();
		void load_sub_catalogs(tstring const& RelatedPathOem);
		void load_shortcuts(tstring const& RelatedPathOem);

		inline bool is_matched_to_pattern(tstring const& FileName)
		{
			return nf::Parser::IsTokenMatchedToPattern(FileName, m_Pattern, false);
		}
		inline bool is_dir_matched_to_pattern(tstring const& DirName)
		{
			return nf::Parser::IsTokenMatchedToPattern(DirName, m_PatternDir, false);
		}
	};

	bool OpenSoftShortcut(HANDLE hPlugin, nf::tparsed_command const &cmd);
	bool GetShortcutProgramPath(tstring const& PathToShortcut
		, tstring &TargetProgramDirectory);
};


#pragma warning(default: 4800)