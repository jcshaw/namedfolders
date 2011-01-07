#include "stdafx.h"

#include <boost/regex.hpp>
#include <cassert>
#include <shlwapi.h>

namespace nf {
int FarCmpName(const wchar_t *Pattern, const wchar_t *String, int SkipPath);
}


bool IsNamedFoldersCatalogExists(tstring const& catalogName) {
	return false; //stub
}
