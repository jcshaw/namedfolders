/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "parser.h"
#include "far_impl.h"
#include "strings_utils.h"
#include "settings.h"

namespace re {	//регулярные выражения
//парсинг типа команды
//массив LIST_COMMANDS задает последовательность команд
//в массиве RE_LIST_COMMANDS в соответствующей последовательности
//перечислены регулярные выражения, позволяющие определить тип команды
//в регулярном выражении не указан префикс (он задается отдельным регулярным выражением RE_PREFIX)
//каждое регулярное выражение раскладывает строку на две части - команду (один или несколько спецсимовлов)
//и путь к ярлыку и директорию
	const int NUM_COMMANDS = 21;
	nf::tcommands_kinds LIST_COMMANDS[NUM_COMMANDS] = {
		nf::QK_OPEN_DIRECTORY_DIRECTLY
		, nf::QK_OPEN_SHORTCUT		
		, nf::QK_INSERT_CATALOG
		, nf::QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT	
		, nf::QK_INSERT_SHORTCUT_IMPLICIT	
		, nf::QK_SEARCH_FILE
		, nf::QK_SEARCH_DIRECTORIES_AND_FILES
		, nf::QK_DELETE_CATALOG
		, nf::QK_DELETE_SHORTCUT_IMPLICIT	
		, nf::QK_INSERT_SHORTCUT
		, nf::QK_INSERT_SHORTCUT_TEMPORARY
		, nf::QK_INSERT_BOTH_TEMPORARY_IMPLICIT
		, nf::QK_INSERT_BOTH_TEMPORARY
		, nf::QK_INSERT_BOTH_IMPLICIT
		, nf::QK_INSERT_BOTH
		, nf::QK_DELETE_SHORTCUT
		, nf::QK_OPEN_NETWORK
		, nf::QK_OPEN_PANEL
		, nf::QK_OPEN_BY_PATH
		, nf::QK_OPEN_ENVIRONMENT_VARIABLE
		, nf::QK_START_SOFT_SHORTCUT

	};	//последовательность команд в списке RE_LIST_COMMANDS
	wchar_t const* LIST_RE[NUM_COMMANDS] = {
		L"()(\\w:.*)" //L"()(\\w:\\\\.*)" - заменено, чтобы работало cd:z:
		, L"()([\\*\\?\\w\\d_\\.@#\\(\\)].*)"
		, L"(:)([^:\\+].*?/)$" //QK_INSERT_CATALOG
		, L"(\\+)((?:[^:\\+]+\\/)?)$" //QK_INSERT_SHORTCUT_TEMPORARY_IMPLICIT
		, L"(:)((?:[^:\\+]+\\/)?)$" //QK_INSERT_SHORTCUT_IMPLICIT
		, L"(\\-\\-f\\s+)(.*)" //QK_SEARCH_FILE
		, L"(\\-\\-df\\s+)(.*)" //QK_SEARCH_DIRECTORIES_AND_FILES
		, L"(\\-)(.*\\/\\s*)" //QK_DELETE_CATALOG
		, L"(\\-)((?:.+\\/)?)$" //QK_DELETE_SHORTCUT_IMPLICIT
		, L"(:)([^:\\+].*)" //QK_INSERT_SHORTCUT
		, L"(\\+)([^:\\+].*)" //QK_INSERT_SHORTCUT_TEMPORARY

		, L"(:\\+)((?:.+\\/)?)"	//QK_INSERT_BOTH_TEMPORARY_IMPLICIT
		, L"(:\\+)(.*)"	//QK_INSERT_BOTH_TEMPORARY
		, L"(::)((?:.+\\/)?)" //QK_INSERT_BOTH_IMPLICIT
		, L"(::)(.*)" //QK_INSERT_BOTH

		, L"(\\-)(.*[^\\/]\\s*)" //QK_DELETE_SHORTCUT
		, L"(\\\\)(.*)" //QK_OPEN_NETWORK
		, L"(\\s*)()$" //QK_OPEN_PANEL
		, L"(~)(.*)" //QK_OPEN_BY_PATH
		, L"(%)(.*)" //QK_OPEN_ENVIRONMENT_VARIABLE
		, L"( )(.*)" //QK_START_SOFT_SHORTCUT
	};
//префикс команды
	wchar_t const * RE_PREFIX = L"^((?:[\\w]+)|(?::)):";
//парсинг каталог/ярлычок\директория
	wchar_t const * RE_CSD = L"([^\\s\\.\\\\]*\\/)?(\\.?[^\\s\\.\\/\\\\]*)?([\\.\\\\][^\\s]*)?(\\s+.+)?";
//парсинг софт параметры
	wchar_t const * RE_SOFT = L"([^\\s]+)(\\s+\"?([^\"]*)\"?)?";
//		L"([^\\\\]*\\/)?([^\\/\\\\]*)?(\\\\[^\\s]+)?(\\s+.+)?";
//парсинг пути содержащего переменную среды
	wchar_t const * RE_EV = L"%?([^%]+)(%?)(.*)";

//поиск метасимволов
	wchar_t const* RE_SEARCH_META = 
		L"(?:.+\\\\\\\\)|(?:\\*)|(?:\\?)|(?:\\[[^\\]]+\\])";
	wchar_t const* RE_SEARCH_META_INTOKENS_ONLY =	//метасимволы в названиях (*, [], ?) но не пути (слеши)
		L"(?:\\*)|(?:\\?)|(?:\\[[^\\]]+\\])";

//exclude double prefixes
	wchar_t const* RE_DOUBLE_PREFIXES =
		L"(\\w[\\w\\d_]+\\:)*(\\w[\\w\\d_]+\\:)(.+)";
}

namespace {
	tstring remove_double_prefix(tstring const& srcStr) { //converts cd:ab:fe:command -> fe:commands (last prefix is always used)
		nf::tregex expression(NF_BOOST_REGEX_COMPILE(re::RE_DOUBLE_PREFIXES));
		nf::tsmatch what;
		if (NF_BOOST_REGEX_LIB::regex_match(srcStr, what, expression)) {
			return what[2] + what[3];
		} 
		return srcStr;
	}
}

bool nf::Parser::ParseString(tstring const &srcStr, nf::tparsed_command &t) {
	tstring csdp;
	t.flags = 0;

	tstring scommand = remove_double_prefix(srcStr);

	if (! GetCommandKind(scommand, t.kind, t.prefix,  csdp)) return false;
	if (! t.prefix.empty()) t.flags = t.flags | nf::FGC_ENABLED_PREFIX;

	if (t.kind == nf::QK_START_SOFT_SHORTCUT) {
		nf::tregex expression(NF_BOOST_REGEX_COMPILE(re::RE_SOFT));
		nf::tsmatch what;
		if (NF_BOOST_REGEX_LIB::regex_match(csdp, what, expression)) {
			t.catalog = L"";
			t.local_directory = L"";
			t.shortcut = what[1] + tstring(L" ") + what[2];
			t.param = L"";	//!TODO: сейчас параметры сохраняем в shortcut
		}
	} else if (t.kind == nf::QK_OPEN_DIRECTORY_DIRECTLY) {
		t.local_directory = csdp;
		Utils::RemoveSingleTrailingCharOnPlace(t.local_directory, SLASH_DIRS_CHAR);
		t.flags = t.flags | nf::FGC_ENABLED_LOCAL_DIRECTORY;
	} else if (ParseCSDP(csdp, t.catalog, t.shortcut, t.local_directory, t.param)) {
		//локальная директория должна содержать все слеши, иначе \\\ будет работать некорректно Utils::RemoveSingleTrailingCharOnPlace(t.local_directory, SLASH_DIRS_CHAR);
		//в то же время если слеш всего один, то его надо игнорировать
		if (! Utils::EndWith(t.local_directory, L"\\\\")) {
			Utils::RemoveSingleTrailingCharOnPlace(t.local_directory, SLASH_DIRS_CHAR);
		}

		if (! t.catalog.empty()) t.flags = t.flags | nf::FGC_ENABLED_CATALOG;
		if (! t.shortcut.empty()) t.flags = t.flags | nf::FGC_ENABLED_SHORTCUT;
		if (! t.local_directory.empty()) t.flags = t.flags | nf::FGC_ENABLED_LOCAL_DIRECTORY;
		if (! t.param.empty()) t.flags = t.flags | nf::FGC_ENABLED_PARAM;
	};
	return true;
}

bool nf::Parser::GetCommandKind(tstring const& source, nf::tcommands_kinds &kind, tstring &prefix, tstring &csdp) {
	for (int i = 0; i < re::NUM_COMMANDS; ++i) {
		tstring rexp(re::RE_PREFIX);
		rexp += re::LIST_RE[i];	

		nf::tregex expression(NF_BOOST_REGEX_COMPILE(rexp.c_str()));
		
		nf::tsmatch what;
		if (NF_BOOST_REGEX_LIB::regex_match(source, what, expression)) {
			prefix = what[1];
			csdp = what[3];
			kind = re::LIST_COMMANDS[i];
			return true;
		}
	}
	return false;
}

bool nf::Parser::ParseCSDP(tstring const&csdp, tstring &c, tstring &s, tstring &d, tstring &p) {	
//разделить каталог/ярлычек\директорию на составляющие
	tregex expression(NF_BOOST_REGEX_COMPILE(re::RE_CSD));
	tsmatch what;
	if (NF_BOOST_REGEX_LIB::regex_match(csdp, what, expression)) {
		c = what[1];
		s = what[2];
		//remove_prefix_from_shortcut(s);
		d = what[3];
		//if (d.size() == 1) d += L'*'; //указан только разделитель; директория не указана
		//Utils::RemoveSingleLeadingCharOnPlace(d, SLASH_DIRS_CHAR);
		p = what[4];
		return true;
	}
	return false;
}

bool nf::Parser::IsTokenMatchedToPattern(tstring const& srcToken, tstring const &srcPattern, bool bAddTrailingAsterix) {
	//	сравнение без учета регистра с учетом метасимволов ? и * (один символ и любое кол-во символов)
	//	"a*" соответствует "abc" и "a",  "b?" соответствует "ba" и "bc"
	if (bAddTrailingAsterix) {
		tstring p = srcPattern + tstring(L"*");
		return nf::FarCmpName(p.c_str(), srcToken.c_str(), FALSE) != 0;
	} else {
		return nf::FarCmpName(srcPattern.c_str(), srcToken.c_str(), FALSE) != 0;
	}
}

bool nf::Parser::ContainsMetachars(tstring const& sToken) {
	return (NF_BOOST_REGEX_LIB::regex_search(sToken, tsmatch(), tregex(NF_BOOST_REGEX_COMPILE(re::RE_SEARCH_META))));
}

bool nf::Parser::IsContainsMetachars_InTokensOnly(tstring const& sToken) {
	return (NF_BOOST_REGEX_LIB::regex_search(sToken, tsmatch(), tregex(NF_BOOST_REGEX_COMPILE(re::RE_SEARCH_META_INTOKENS_ONLY))));
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// распарсить путь в котором указана переменная среды
// возможные варианты: "VarName", "%VarName", "%VarName%", "%VarName%LocalPath", "VarName%"
// возвращает true, если название VarName задано не точно (варианты 1, 2)
bool nf::Parser::ParseEnvVarPath(tstring const& srcStr, tstring& varName, tstring& localPath) {
	localPath.clear();
	tsmatch what;
	if (! NF_BOOST_REGEX_LIB::regex_search(srcStr, what, tregex(NF_BOOST_REGEX_COMPILE(re::RE_EV)))) {
		varName = srcStr;
		return false;
	}
	varName = what[1];
	localPath = what[3];
	return ! tstring(what[2]).empty();
}

tstring nf::Parser::ExtractPrefix(tstring const &srcCommand) {
//regexp is not necessary here
	if (srcCommand.empty()) return L"";

	tstring::const_iterator p1 = srcCommand.begin() + 1;
	tstring::const_iterator p = std::find(p1, srcCommand.end(), L':');
	if (p == srcCommand.end()) return L"";

	return tstring(srcCommand.begin(), p + 1);
}

tstring nf::Parser::ConvertToMask(tstring const& srcStr, tasterix_mode asterixMode012) {	
	//добавляем звездочки только если маска, указанная пользователем, не содержит метасимволов
	//если у нас маска \a\b*\c то она должна обрабатываться как маска \*a*\b*\*c* 
	//поэтому обрабатываем a, b*, c по отдельности
	//правило преобразование маски a -> *a* определяется режимом настроек
	//1: a -> a*
	//2: a -> a
	//0: a -> *a*

	if (nf::Parser::ContainsMetachars(srcStr)) return srcStr;

	tstring result = srcStr;
	if (asterixMode012 == nf::ASTERIX_MODE_AS_IS) return srcStr;
	result.push_back(L'*');	
	if (asterixMode012 == nf::ASTERIX_MODE_POSTFIX) return result;

	result.insert(0, L"*");	
	return result;
}

tstring nf::Parser::ConvertToMask(tstring const& srcStr) {
	return ConvertToMask(srcStr, static_cast<tasterix_mode>(Utils::atoi(CSettings::GetInstance().GetValue(nf::ST_ASTERIX_MODE))));
}

tstring nf::Parser::ConvertMaskToReqex(tstring const& srcName) {
	static const nf::tregex esc(L"[\\^\\.\\$\\|\\(\\)\\+\\/\\\\]"); //dont' escape \\[\\]\\*\\? 
	static const tstring rep(L"\\\\\\1&"); //see http://stackoverflow.com/questions/1252992/how-to-escape-a-string-for-use-in-boost-regex

	tstring smask = NF_BOOST_REGEX_LIB::regex_replace(srcName, esc, rep, boost::match_default | boost::format_sed);

	smask = Utils::ReplaceStringAll(smask, L"*", L".*");
	return Utils::ReplaceStringAll(smask, L"?", L".?");
}


unsigned int nf::Parser::GetNetworkPathPrefixLength(tstring const &source) {
	static const nf::tregex expression(L"(\\\\\\\\|[^\\\\]+:\\\\\\\\).*");
	nf::tsmatch what;	
	if (NF_BOOST_REGEX_LIB::regex_match(source, what, expression)) {
		return static_cast<unsigned int>(tstring(what[1]).size());
	}
	return 0;
}

unsigned int nf::Parser::ExtractDeepOfSearch(tstring const& srcName) {
	static const nf::tregex expression(L":(\\d+)");
	nf::tsmatch what;	
	if (NF_BOOST_REGEX_LIB::regex_match(srcName, what, expression)) {
		return Utils::atoi(what[1]);
	}
	return 0;
}