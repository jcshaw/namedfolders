#pragma once

#include "strings_utils.h"

namespace Utils {
	tstring ExtractCatalogName(tstring const& srcPath);

	//srcCatalog - current catalog, that should be renamed
	//targetCatalog - new catalog name, that user has specified (it can include symbols '..' or '.', can be absolute or relative)
	//destCatalog - full name of new catalog.
	bool ExpandCatalogPath(tstring const &srcCatalog
		, tstring const& targetCatalog
		, tstring& destCatalog);
	tstring MakePathCompact(tstring const &srcCatalog, tstring const &root);
	tstring GetCanonicalCatalogName(tstring const& srcCatalog);
	namespace Private {
		tstring mix_paths(std::list<tstring> const& s1, std::list<tstring> const& s2, wchar_t destDelimeter);
	}
}
