#pragma once

#include "header.h"
namespace nf {
	inline const wchar_t *GetMsg(int MsgId) {
		return g_PluginInfo.GetMsg(&nf::NF_PLUGIN_GUID, MsgId);
	}
}