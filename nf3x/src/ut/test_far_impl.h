#include "stdafx.h"

#include <boost/regex.hpp>
#include <cassert>
#include <shlwapi.h>

namespace nf {
int FarCmpName(const char *Pattern, const char *String, int SkipPath);
}
