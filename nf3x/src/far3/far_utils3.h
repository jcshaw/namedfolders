#pragma once

#include "header.h"
namespace nf {
	namespace nffar3 {
		extern const GUID g_NamedFolderFar3_GUID;
	}

	inline const wchar_t *GetMsg(int MsgId) {
		return g_PluginInfo.GetMsg(&nf::nffar3::g_NamedFolderFar3_GUID, MsgId);
	}
}