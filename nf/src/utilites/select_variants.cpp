/*
* Far Named Folders 3.x
* Copyright (c) 2002-2010 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#include "StdAfx.h"
#include "select_variants.h"

#include <functional>
#include <cassert>
#include <shlwapi.h>
#include <boost/foreach.hpp>

#include "stlcatalogs.h"
#include <algorithm>
#include "Parser.h"
#include "codec_for_values.h"
#include "strings_utils.h"

#include "searcherpaths.h"
#include "PanelInfoWrap.h"

using namespace nf;

namespace {
	int append_if_equal_to_pattern(sc::shortcuts_sequence_item sv
		, nf::tshortcuts_list &DestList
		, tstring const& Catalog
		, tstring const& srcPattern
		, bool bIsTemporary)
	{
		if (nf::Parser::IsTokenMatchedToPatternOEM(sv.GetName(), srcPattern)) {
			DestList.push_back(nf::MakeShortcut(Catalog, sv.GetName(), bIsTemporary));
		}
		return 0;
	}

	inline bool is_match(tstring const &Value, tstring const& valuePattern) {
		return nf::Parser::IsTokenMatchedToPattern(Value, valuePattern.c_str(), false);
	}
	int append_if_equal_paths(sc::shortcuts_sequence_item sv
		, nf::tshortcuts_list& DestList
		, tstring const& Catalog
		, tstring const& valuePattern
		, bool bTemporary)
	{
		nf::tshortcut_value_parsed_pair vp = nf::DecodeValues(sv.GetValue());

		if ( (vp.first.bValueEnabled && is_match(vp.first.value, valuePattern)) 
			|| (vp.second.bValueEnabled && is_match(vp.second.value, valuePattern)) )
		{
			DestList.push_back(nf::MakeShortcut(Catalog, sv.GetName(), bTemporary));
		}
		return 0;
	}

	int select_shortcuts(boost::function<int (sc::shortcuts_sequence_item, bool, tstring)> &Func
		, bool bSearchInSubcatalogs
		, tstring Catalog)
	{
		Utils::RemoveTrailingCharsOnPlace(Catalog, SLASH_CATS_CHAR);
		sc::CCatalog c(Catalog, 0, false);

		//���������� ��� ���������� ����������
		sc::shortcuts_sequence seq(c.GetSequenceShortcuts(false));
		std::for_each(seq.begin(), seq.end(), boost::bind(Func, _1, false, Catalog));

		//���������� ��� ��������� ����������
		sc::shortcuts_sequence seq2(c.GetSequenceShortcuts(true));
		std::for_each(seq2.begin(), seq2.end(), boost::bind(Func, _1, true, Catalog));

		if (bSearchInSubcatalogs) {	//look through all subcatalogs
			sc::subcatalogs_sequence seqc(c.GetSequenceSubcatalogs());
			BOOST_FOREACH(sc::subcatalogs_sequence::value_type const& cat, seqc) {
				sc::catalogs_sequence_item cv(cat);
				tstring subcatalog_name = cv.GetName();
				Utils::RemoveTrailingCharsOnPlace(subcatalog_name,  SLASH_CATS_CHAR);
				tstring cat2 = Utils::CombinePath(Catalog, subcatalog_name, SLASH_CATS);
				select_shortcuts(Func, bSearchInSubcatalogs, cat2);
			}
		}

		return 0;
	}

	int select_shortcuts_ex(nf::tshortcuts_list& destList
		, tstring const &shPattern
		, bool bSearchInSubcatalogs
		, tstring Catalog
		, bool bSearchByPath)
	{
		boost::function<int (sc::shortcuts_sequence_item, bool, tstring)> f 
			= boost::bind( (bSearchByPath ? append_if_equal_paths : append_if_equal_to_pattern)
				, _1,  boost::ref(destList),  _3,  boost::ref(shPattern), _2);

		select_shortcuts(f, bSearchInSubcatalogs, Catalog);

		if (nf::CSettings::GetInstance().GetValue(nf::ST_SUBDIRECTORIES_AS_ALIASES))
		{	//��������� ���������� "�������������" � �������� �����������
			//����� �������� �� �� ��������� �����������, ��������� � �� ����� ������� .slash
			tstring current_directory = CPanelInfoWrap(INVALID_HANDLE_VALUE).GetPanelCurDir(true);
			//��� ����������� ������� current_directory ����� ���� ������
			if (! current_directory.empty()) {
				//tstring pattern = nf::Parser::ConvertToMask(ShortcutPatternOEM);
				tstring pattern = Parser::IsContainsMetachars(shPattern) 
						? shPattern
						: shPattern + L"*";	//���� ������������� ��� 'b*', � �� ��� '*b*', ����� ��������� ������ �����������
				WinSTL::findfile_sequence_t f(current_directory.c_str()
					, pattern.c_str()
					, WinSTL::findfile_sequence_t::directories); //���� ������ ����������
				BOOST_FOREACH(WinSTL::findfile_sequence_t::value_type const& t, f) {
					destList.push_back(nf::MakeShortcut(L".", tstring(L"\\") + (t).get_filename(), false));
				} 
			}
		}

		return static_cast<int>(destList.size());
	}
}

int nf::Shell::SelectShortcuts(tstring const &catalog
							   , tstring const &shortcut_pattern
							   , nf::tshortcuts_list& DestList
							   , bool bSearchInSubcatalogs)
{
	//������������ ���� x - ����� ��� ������� ��������������� x
	//� ��������� �������� � �� ��������� ������������
	//������� ���-�� ������ ���������� (������ ���������� ������ ���� �������)
	sc::CCatalog c(catalog, 0, false);

	//���� ������������ ���� �� �����, � �������� ���� 
	//�� ��������� ����������� - � �������� ����� ����
	//���� ����� ��������� ������; ���������, ��� ������ ��������� ����, ����� 
	//����� ����� �� ���������� ��� ������� ��������
	if (DestList.empty())
	if (! ::PathIsRelative(shortcut_pattern.c_str()))	//������ ���� ������ ���������� ���� 
	if (::PathFileExists(shortcut_pattern.c_str())) {
		DestList.push_back(nf::MakeShortcut(L"", shortcut_pattern, false));
	}

	return select_shortcuts_ex(DestList, shortcut_pattern, bSearchInSubcatalogs, catalog, false);
}

int nf::Shell::SelectShortcutsByPath(tstring catalog
									 , tstring const &value_pattern
									 , nf::tshortcuts_list& DestList
									 , bool bExactCoincidence //!TODO
									 , bool bSearchInSubcatalogs)
{
	Utils::RemoveTrailingCharsOnPlace(catalog, SLASH_CATS_CHAR);
	sc::CCatalog c(catalog, false);

	tstring value_pattern_oem = nf::Parser::ConvertToMask(L"*") + value_pattern + tstring(L"*");
	return select_shortcuts_ex(DestList, value_pattern_oem, bSearchInSubcatalogs, catalog, true);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
namespace
{
	void search_catalogs(sc::CCatalog& c, std::list<tstring> &ListCatalogParts, nf::tcatalogs_list& list)
	{
		tstring smask;
		smask.swap(*ListCatalogParts.begin());
		ListCatalogParts.erase(ListCatalogParts.begin());

		sc::subcatalogs_sequence f(c.GetSequenceSubcatalogs());
		BOOST_FOREACH(sc::subcatalogs_sequence::value_type &t, f) {
			sc::catalogs_sequence_item cv(t);
			tstring catalog_name = cv.GetName().c_str();
			if (! nf::Parser::IsTokenMatchedToPatternOEM(catalog_name, smask)) continue;

			//��� ������� ���������� ���������� ������������ ����������� ����� ��� ��������� ��������� �������
			sc::CCatalog subc(cv, &c);
			if (ListCatalogParts.empty()) {
				list.push_back(subc.CatalogPath()); //���� ��� ��������� ������� ������� - ������� ��������� ���������� � ������
			} else {
				search_catalogs(subc, ListCatalogParts, list); //���������� ���� ��������� ������� �������
			}
		}
	}
}

int nf::Shell::SelectCatalogs(tstring const& pattern	//������
							 , nf::tcatalogs_list& list)
{ //����� ��� �������� ��������� ��������������� �������, ������� ������ ���� ��������� ����������	
  //������� ���-�� ������ ���������� (������ ���������� ������ ���� �������)
	sc::CCatalog c;	//!TODO: ��������������
	std::list<tstring> catalog_parts;
	Utils::SplitStringByRegex(pattern, catalog_parts, SLASH_CATS);
	::search_catalogs(c, catalog_parts, list);

	return 0;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
namespace
{
	inline bool is_equal_to_pattern(tstring const& Pattern, nf::tshortcut_info const& s1) {
		return ! lstrcmpi(s1.shortcut.c_str(), Pattern.c_str()) ? 0 : 1;
	}
	inline bool less_hashed(tstring const& Pattern, nf::tshortcut_info const& s1, nf::tshortcut_info const &s2) {
		bool h1 = is_equal_to_pattern(Pattern, s1);
		bool h2 = is_equal_to_pattern(Pattern, s2);
		if ((! h1) || (! h2)) return h1 < h2;
		return s1.shortcut < s2.shortcut;
	}
}

int nf::Shell::SortByRelevance(nf::tshortcuts_list& list
							   , tstring const& SrcCatalog
							   , tstring const& SrcShPattern)
{
	//�������� ��� ������ ���������� � ������ ������, ��������� ��������� �� �����������
	list.sort(boost::bind(less_hashed, SrcShPattern, _1, _2));
	//std::sort(list.begin(), list.end(), boost::bind(less_hashed, SrcShPattern, _1, _2) ); 
	//������� ���-�� ��������� ����� ������ ������� (��� ��� hash ����� 0)
	nf::tshortcuts_list::iterator p = find_if(list.begin(), list.end()
		, boost::bind(std::equal_to<bool>(), false,  boost::bind(&is_equal_to_pattern, SrcShPattern, _1)));
	if (p == list.end()) return 0;
	return static_cast<int>(std::distance(p, list.begin()) + 1);
}


