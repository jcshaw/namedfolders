/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

#include "header.h"

namespace nf {
namespace Parser {
	/* ��������� ��� ����� �������� � ������ ������������ ? � * (���� ������ � ����� ���-�� ��������)
	"a" === "a*",  "a*" ������������� "abc" � "a",  "b?" ������������� "ba" � "bc"*/
	bool IsTokenMatchedToPattern(tstring const& srcToken, tstring const &srcPattern, bool bAddTrailingAsterix = true);

	// ������ �������� ����� �� ������������ *, ? ��� [a,b-c], (�����) 
	bool ContainsMetachars(tstring const& stoken);
	bool IsContainsMetachars_InTokensOnly(tstring const& stoken);

	// ���������� ���� � ������� ������� ���������� �����
	// ��������� ��������: "VarName", "%VarName", "%VarNme%", "%VarName%LocalPath"
	// ���������� true, ���� �������� VarName ������ �� ����� (�������� 1, 2)
	bool ParseEnvVarPath(tstring const& Src, tstring& VarName, tstring& LocalPath); 

	//���������� ������ � ����� � ������ ������ �������� asterixmode, � ������� � ������ ������������
	tstring ConvertToMask(tstring const& SrcStr);
	tstring ConvertMaskToReqex(tstring const& srcName);

	bool GetCommandKind(tstring const& source, nf::tcommands_kinds &kind, tstring &prefix, tstring &csdp);
	//csdp - catalog, shortcut, directory, parameters 
	bool ParseCSDP(tstring const&csdp, tstring &c, tstring &s, tstring &d, tstring &p);
	bool ParseString(tstring const &source, nf::tparsed_command &cmd);

	tstring ExtractPrefix(tstring const &CommandString);
};
};