/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/

#pragma once

/************************************************************************/
// поиск директорий по заданным паттернам
// паттерны могут содержать стандартные метасимволы Far
// *, ? и [a-z]
/************************************************************************/
#include "strings_utils.h"

namespace nf
{
	namespace SearchPathsPolices
	{

		struct CSearchPolice
		{
			CSearchPolice(twhat_to_search_t WhatToSearch) :  m_WhatToSearch(WhatToSearch) {}
		//выполнить поиск поддиректории по заданному шаблону имени директории
			virtual void  SearchSubdir(tstring const& RootDir
				, tstring const& Name
				, std::list<tstring> &list) const = 0;
			twhat_to_search_t GetWhatToSearch() const {return m_WhatToSearch;}
		private:
			twhat_to_search_t m_WhatToSearch;
		};

		class CSearchSystemPolice : public CSearchPolice
		{	//поиск системными средствами; метасимволы [] не поддерживаются
			//все найденные имена автоматически считаются совпавшими с шаблоном
		public:
			CSearchSystemPolice(twhat_to_search_t WhatToSearch) : CSearchPolice(WhatToSearch) {}
			virtual void SearchSubdir(tstring const& RootDir
				, tstring const& Name
				, std::list<tstring> &list) const;
		};

		class CSearchFarPolice : public CSearchPolice
		{	//находим все директорий, а затем проверяем совпадение имен с шаблоном
			//средствами Far
		public:
			CSearchFarPolice(twhat_to_search_t WhatToSearch) 
				: CSearchPolice(WhatToSearch) {}
			virtual void SearchSubdir(tstring const& RootDir
				, tstring const& Name
				, std::list<tstring> &list) const;
		};
	};

	//поиск директорий ведется системными функциям
	//системные функции позволяют задать маску поиска с использованием метасимволов * и ?
	//если требуется поддержка дополнительного метасимвола [], объявленного в FAR
	//необходимо вначале найти все директории, затем проверить их совпадение с шаблоном средствами FAR
	//в итоге два варианта поиска для класса CSearcherPaths задаются в SearchPathsPolices

	class CSearcherPaths
	{	
	public:
		typedef std::list<tstring> tlist;

		CSearcherPaths(tlist& Results
			, SearchPathsPolices::CSearchPolice &sp
			, twhat_to_search_t WhatToSearch = WTS_DIRECTORIES) 
			: m_Results(Results)
			, m_Sp(sp)
			, m_WhatToSearch(WhatToSearch)
		{}
		// поиск директории, вложенной в текущую, по шаблону {[\dir]|[\\dir]|[\\\dir]|..}+
		// вариант поиска имени в текущей директории определяется кол-вом слешей.
		// "\dir"  - поиск dir только в текущей директории
		// "\\dir" - поиск dir во всех вложенных директориях и в директориях непосредственно в них вложенных 
		// "\\\dir" и т.д.
		//? "\*\dir" - поиск имени dir во всех вложенных директориях
		//? ".dir" - переход dir начиная с директории на один уровень выше, "..dir" на два уровня выше и т.д.
		inline bool SearchByPattern(TCHAR const* PatternOEM, tstring const &RootDirOEM)
		{
				tstring RootDirANSI = Utils::GetInternalToOem(RootDirOEM);
				tstring PatternANSI = Utils::GetInternalToOem(PatternOEM);
				return SearchByPatternOEM(PatternANSI.c_str(), RootDirANSI);
		}

		//найти все директории, удовлетворяющие паттерну
		//PathPattern должен содержать полный локальный путь вида
		//C:\path1\path2\...\pathN
		//причем C - любая одна буква или один из метасимволов ?, * или [a-z]
		//В токенах pathXXX могут встречаться метасимволы ?, * или [a,b-z]
		inline bool SearchMatched(tstring PathPatternOEM)
		{
			tstring PatternANSI = Utils::GetInternalToOem(PathPatternOEM);
			return SearchMatchedOEM(PatternANSI);
		}

	private: //operations
		inline void insert_result(tstring const& dir_oem)
		{
			m_Results.push_back(dir_oem);
		}
		bool search_multisubdir(TCHAR const* RootDir, TCHAR const* Name, int level, tlist &list);
		TCHAR const* extract_name(TCHAR const* Pattern, tstring &Name, int &Level);

		bool SearchByPatternOEM(TCHAR const* Pattern, tstring const &RootDir);
		bool SearchMatchedOEM(tstring PathPattern);

	private: //members
		tlist& m_Results;
		SearchPathsPolices::CSearchPolice const &m_Sp;
		twhat_to_search_t m_WhatToSearch;
	}; //CSearcherPaths

};//nf