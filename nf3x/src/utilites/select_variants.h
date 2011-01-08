/*
* Far Named Folders 3.x
* Copyright (c) 2002-2011 by Victor Derevyanko
* www: http://code.google.com/p/namedfolders/
* e-mail: dvpublic0@gmail.com
*/
#pragma once

namespace nf {
namespace Shell {
	//найти все варианты каталогов удовлетворяющих шаблону
	int SelectCatalogs(tstring const& SrcPattern		//шаблон
						, nf::tvector_strings& DestList//список всех найденных совпадений					
						);	

	//найти в заданном каталоге все ярлыки, удовлетворяющие шаблону
	int SelectShortcuts(tstring const &catalog				//в каталоге
						, tstring const &shortcut_pattern	//щаблон 
						, nf::tshortcuts_list& DestList			//список всех найденных ярлычков
						, bool bSearchInSubcatalogs	//искать во вложенных каталогах тоже
						);

	//найти в заданном каталоге все псевдонимы
	//для которых именованные директории, на которые они ссылаются
	//удовлетворяют переданному шаблону
	int SelectShortcutsByPath(tstring catalog_pattern
							  , tstring const &value_pattern
							  , nf::tshortcuts_list& DestList
							  , bool bExactCoincidence
							  , bool bSearchInSubcatalogs = true);


	//упорядочить последовательность найденных вариантов по релевантности
	//точные совпадения поставить первыми и вернуть их точное количество
	int SortByRelevance(nf::tshortcuts_list& SrcDestList, 
						tstring const& catalog, 
						tstring const& shortcut_pattern);


} //Shell
} //nf