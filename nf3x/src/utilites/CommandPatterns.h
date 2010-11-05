#pragma once

#include <map>
#include <vector>
#include <list>
#include "header.h"
#include "Registry.h"
#include "strings_utils.h"

namespace nf {
namespace Patterns {

typedef std::pair<tstring, tstring> tcommand_pattern;
typedef tlist_pairs_strings tlist_command_patterns;
/************************************************************************/
// ѕредоставл€ет интерфейс дл€ работы с шаблонами команд
// —читывание списка команд, добавление, удаление, редактирование
// –екурсивна€ раскрутка команды
/************************************************************************/
class CommandsManager {
	nf::CRegistry m_Key;
public:
	explicit CommandsManager(tstring const& RegKey);
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
class CommandPatterns {
	typedef tlist_pairs_strings tprefix_pattern_vector; 
	//typedef std::map<tstring, tstring, Utils::CmpStringLessCI> tmap;	//prefix - patterns
	tprefix_pattern_vector m_PP;
public:
	explicit CommandPatterns(tlist_command_patterns const&ListPatterns);
	~CommandPatterns(void);

	bool TransformCommand(nf::tparsed_command const &SrcCmd
		, tstring &DestCmd) const;
	bool TransformCommand(tstring const& Pattern
		, nf::tparsed_command const &SrcCmd
		, tstring &DestCmd) const;
};	//CommandPatterns

	namespace Private {	 //Private namespace is visible for unit testing
		class DetailedCommand {
			typedef nf::tvector_strings tlistparts;
			tlistparts const m_SrcParts;
			tlistparts m_DestParts;
		public:
			explicit DetailedCommand(nf::tparsed_command const &SrcCmd);
			void ApplyPattern(tstring const& Pattern);
			tstring GetResultString() const;
		private:
			tlistparts commandToListParts(nf::tparsed_command const &SrcCmd) const;
		};	//DetailedCommand
	}

} //Patterns
} //nf
