/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko, RAMMuS Group
* www: http://www.rammus.ru, http://derevyanko.blogspot.com/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

namespace nf
{

namespace Parser 
{

	/* ��������� ��� ����� �������� � ������ ������������ ? � * (���� ������ � ����� ���-�� ��������)
	"a" === "a*",  "a*" ������������� "abc" � "a",  "b?" ������������� "ba" � "bc"*/
	bool IsTokenMatchedToPattern(tstring const& stoken
								, tstring const &spattern
								, bool bAddTrailingAsterix = true);

	bool IsTokenMatchedToPatternOEM(tstring const& stokenOEM
									, tstring const &spatternOEM
									, bool bAddTrailingAsterix = true);

	// ������ �������� ����� �� ������������ *, ? ��� [a,b-c], (�����) 
	bool IsContainsMetachars(tstring const& stoken);
	bool IsContainsMetachars_InTokensOnly(tstring const& stoken);

	// ���������� ���� � ������� ������� ���������� �����
	// ��������� ��������: "VarName", "%VarName", "%VarNme%", "%VarName%LocalPath"
	// ���������� true, ���� �������� VarName ������ �� ����� (�������� 1, 2)
	bool ParseEnvVarPath(tstring const& Src
						, tstring& VarName
						, tstring& LocalPath
						); 

	//���������� ������ � �����
	//� ������ ������ �������� asterixmode, � ������� � ������ ������������
	tstring ConvertToMask(tstring const& SrcStr);

	bool GetCommandKind(tstring const& source, 
		nf::tcommands_kinds &kind, 
		tstring &prefix,
		tstring &csdp);
	//csdp - catalog, shortcut, directory, parameters 
	bool ParseCSDP(tstring const&csdp, tstring &c, tstring &s, tstring &d, tstring &p);
	bool ParseString(tstring const &source, nf::tparsed_command &cmd);

	tstring ExtractPrefix(tstring const &CommandString);
};

};