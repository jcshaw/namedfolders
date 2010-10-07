#pragma once

#include <map>
#include <list>
#include "header.h"
#include "Registry.h"
#include "strings_utils.h"

namespace nf 
{

namespace Patterns
{

typedef std::pair<tstring, tstring> tcommand_pattern;
typedef std::list<tcommand_pattern> tlist_command_patterns;
/************************************************************************/
// ѕредоставл€ет интерфейс дл€ работы с шаблонами команд
// —читывание списка команд, добавление, удаление, редактирование
// –екурсивна€ раскрутка команды
/************************************************************************/
class CommandsManager
{
	nf::CRegistry m_Key;
public:
	CommandsManager(tstring const& RegKey);
	~CommandsManager();

	bool SetCommand(tstring const& Prefix, tstring const& Pattern);
	bool CheckIfPrefixIsFree(tstring const& Prefix);
	bool RemoveCommand(tstring const& Prefix);
	bool TransformCommandRecursively(tstring const &SrcCmd, tstring &DestCmd) const;
	void GetListRegisteredCommands(tlist_command_patterns &ListPatterns) const;
	tstring GetListCommandPrefixes() const; //список префиксов зарегистрированных дл€ шаблонных команд
};	//CommandsManager


/************************************************************************/
// Ўаблоны команд
// ¬ общем виде, кажда€ команда представл€етс€ так
// [Prefix][Command][Catalog][CDelim][Name][PDelim][Path][Space][Params]
// ƒл€ шаблона задаетс€: префикс + список значений переменных (через точку с зап€той)
// Ќапример: $Name=$Prefix;$Catalog=abc;$Command=--f 
/************************************************************************/
class CommandPatterns
{
	typedef std::map<tstring, tstring, Utils::CmpStringLessCI> tmap;	//prefix - patterns
	tmap m_PP;
public:
	CommandPatterns(tlist_command_patterns const&ListPatterns);
	~CommandPatterns(void);

	bool TransformCommand(nf::tparsed_command const &SrcCmd
		, tstring &DestCmd) const;
	bool TransformCommand(tstring const& Pattern
		, nf::tparsed_command const &SrcCmd
		, tstring &DestCmd) const;
};	//CommandPatterns

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	namespace Private
	{	//вытащил этот класс наружу только дл€ UnitTesting 
		class DetailedCommand
		{
			typedef std::vector<tstring> tlistparts;
			tlistparts const m_SrcParts;
			tlistparts m_DestParts;
		public:
			DetailedCommand(nf::tparsed_command const &SrcCmd);
			void ApplyPattern(tstring const& Pattern);
			bool GetResultCommand(nf::tparsed_command &DestCmd) const;
			tstring GetResultString() const;
		private:
			tlistparts commandToListParts(nf::tparsed_command const &SrcCmd) const;
			void listPartsToCommand(tlistparts const& SrcListParts, nf::tparsed_command &DestCmd) const;
		};	//DetailedCommand
	}

} //Patterns

} //nf
