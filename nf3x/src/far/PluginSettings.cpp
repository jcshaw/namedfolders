#include "stdafx.h"
#include "PluginSettings.h"

#include <boost/foreach.hpp>

extern struct PluginStartupInfo g_PluginInfo; 

#define _ENFORCER_WIDEN2(x) L ## x
#define _ENFORCER_WIDEN(x) _ENFORCER_WIDEN2(x)
#define _ENFORCER_WFILE _ENFORCER_WIDEN(__FILE__)
#define _ENFORCER_WSTRINGIZE(x) _ENFORCER_WIDEN(STRINGIZE(x))

namespace {
	nf::PluginSettings::tsettings_handle open_handle() {
		FarSettingsCreate fsc = {
			sizeof (FarSettingsCreate)
			, nf::NF_PLUGIN_GUID
			, 0
		};

		if (! g_PluginInfo.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, PSL_ROAMING, &fsc)) {
			throw std::exception("SCTL_CREATE");
		}
		return fsc.Handle;
	}
}
nf::PluginSettings::PluginSettings() 
	: _Handle(open_handle())
{
}

nf::PluginSettings::~PluginSettings() {
	g_PluginInfo.SettingsControl(_Handle, SCTL_FREE, 0, 0);
}

nf::PluginSettings::tsettings_handle nf::PluginSettings::getHandle() {
	static PluginSettings s;
	return s._Handle;
}


nf::PluginSettings::tsettings_handle nf::PluginSettings::FarOpenKey(tkey_handle keyHandle, tstring const& keyName) {
	FarSettingsValue fsv = {
		sizeof(FarSettingsValue)
		, reinterpret_cast<size_t>(keyHandle) 
		, keyName.c_str()
	};
	return (tkey_handle)g_PluginInfo.SettingsControl(PluginSettings::getHandle()
		, SCTL_OPENSUBKEY
		, 0
		, &fsv
	);
}


nf::PluginSettings::tsettings_handle nf::PluginSettings::FarCreateKey(tkey_handle keyHandle, tstring const& keyName) {
	FarSettingsValue fsv = {
		sizeof(FarSettingsValue)
		, reinterpret_cast<size_t>(keyHandle) 
		, keyName.c_str()
	};
	return (tkey_handle)g_PluginInfo.SettingsControl(PluginSettings::getHandle()
		, SCTL_CREATESUBKEY
		, 0
		, &fsv
	);
}

bool nf::PluginSettings::FarDeleteKey(tkey_handle keyHandle, tstring const& keyName) {
	FarSettingsValue fsv = {
		sizeof(FarSettingsValue)
		, reinterpret_cast<size_t>(keyHandle) 
		, keyName.c_str()
	};
	return 0 != g_PluginInfo.SettingsControl(PluginSettings::getHandle()
		, SCTL_DELETE
		, 0
		, &fsv
	);
}

bool nf::PluginSettings::FarSet(tkey_handle keyHandle, tstring const& name, tstring const& strValue) {
	FarSettingsItem fsi = {
		sizeof(FarSettingsItem)
		, reinterpret_cast<size_t>(keyHandle) 
		, name.c_str()
		, FST_STRING
		, 0
	};
	fsi.String = strValue.c_str();
	return 0 != g_PluginInfo.SettingsControl(PluginSettings::getHandle()
		, SCTL_SET
		, 0
		, &fsi
	);
}
bool nf::PluginSettings::FarGet(tkey_handle keyHandle, tstring const& name, tstring& dest) {
	FarSettingsItem fsi = {
		sizeof(FarSettingsItem)
		, reinterpret_cast<size_t>(keyHandle) 
		, name.c_str()
		, FST_STRING
		, 0
	};
	if (g_PluginInfo.SettingsControl(PluginSettings::getHandle(), SCTL_GET, 0, &fsi)) {
		dest = fsi.String;
		return true;
	} else {
		return false;
	}
}
bool nf::PluginSettings::FarSet(tkey_handle keyHandle, tstring const& name, __int64 number) {
	FarSettingsItem fsi = {
		sizeof(FarSettingsItem)
		, reinterpret_cast<size_t>(keyHandle) 
		, name.c_str()
		, FST_STRING
		, number
	};
	return 0 != g_PluginInfo.SettingsControl(PluginSettings::getHandle()
		, SCTL_SET
		, 0
		, &fsi
	);
}
__int64 nf::PluginSettings::FarGet(tkey_handle keyHandle, tstring const& name, __int64 defaultValue) {
	FarSettingsItem fsi = {
		sizeof(FarSettingsItem)
		, reinterpret_cast<size_t>(keyHandle) 
		, name.c_str()
		, FST_STRING
		, 0
	};
	return g_PluginInfo.SettingsControl(PluginSettings::getHandle(), SCTL_GET, 0, &fsi)
		? fsi.Number
		: 0;
}

bool nf::PluginSettings::FarEnum(tkey_handle keyHandle, FarSettingsEnum& fse) {
	fse.StructSize = sizeof(FarSettingsEnum);
	fse.Root = reinterpret_cast<size_t>(keyHandle);
	return 0 != g_PluginInfo.SettingsControl(PluginSettings::getHandle(), SCTL_ENUM, 0, &fse);
}	


nf::PluginSettings::tsettings_handle nf::PluginSettings::FarOpenKey(tkey_handle keyHandle, std::list<tstring> const& keys) {
	if (keys.empty()) {
		return 0;
	}

	tkey_handle key = keyHandle;
	BOOST_FOREACH(auto const& path_item, keys) {
		key = FarOpenKey(key, path_item);
		if (key == 0) {
			break;
		}
	}
	return key;
}

nf::PluginSettings::tsettings_handle nf::PluginSettings::FarCreateKey(tkey_handle keyHandle, std::list<tstring> const& keys) {
	if (keys.empty()) {
		return 0;
	}

	tkey_handle key = keyHandle;
	BOOST_FOREACH(auto const& path_item, keys) {
		tkey_handle subkey = FarOpenKey(key, path_item);
		if (subkey == 0) {
			key = FarCreateKey(key, path_item);
			if (key == 0) {
				break;
			}
		} else {
			key = subkey;
		}
	}
	return key;
}

bool nf::PluginSettings::FarDeleteLastKey(tkey_handle keyHandle, std::list<tstring> const& keys) {
	if (keys.empty()) {
		return false;
	}

	if (keys.size() == 1) {
		return FarDeleteKey(keyHandle, *keys.begin());
	}

	tkey_handle key = keyHandle;
	auto n = keys.size();
	BOOST_FOREACH(auto const& path_item, keys) {
		if (n == 1) {
			return FarDeleteKey(key, path_item);
		} else {
			key = FarOpenKey(key, path_item);
			if (key == 0) {
				return false;
			}
			--n;
		}
	}
	return false;
}
