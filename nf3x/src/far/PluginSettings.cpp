#include "stdafx.h"
#include "PluginSettings.h"

#include <boost/foreach.hpp>
#include "CatalogSequences.h"

#include <FarCatalog.h>

extern struct PluginStartupInfo g_PluginInfo; 

#define _ENFORCER_WIDEN2(x) L ## x
#define _ENFORCER_WIDEN(x) _ENFORCER_WIDEN2(x)
#define _ENFORCER_WFILE _ENFORCER_WIDEN(__FILE__)
#define _ENFORCER_WSTRINGIZE(x) _ENFORCER_WIDEN(STRINGIZE(x))

nf::PluginSettings::PluginSettings()  {
	FarSettingsCreate fsc = {
		sizeof (FarSettingsCreate)
		, nf::NF_PLUGIN_GUID
		, 0
	};

	if (! g_PluginInfo.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, PSL_ROAMING, &fsc)) {
		throw std::exception("SCTL_CREATE");
	}
	_h = fsc.Handle;
}

nf::PluginSettings::~PluginSettings() {
	if (_h != 0) {
		g_PluginInfo.SettingsControl(_h, SCTL_FREE, 0, 0);
		_h = 0;
	}	
}

namespace {
	tstring to_lowcase(tstring const& src) {
		nf::tautobuffer_char buffer(src.size() + 1);
		lstrcpy(&buffer[0], src.c_str());
		g_PluginInfo.FSF->LStrlwr(&buffer[0]);
		return &buffer[0];
	}
}


nf::PluginSettings::tsettings_handle nf::PluginSettings::FarOpenKey(tkey_handle keyHandle, tstring const& keyName) const {
	tstring key_name;
	if (! ic_search_key_name(keyHandle, keyName, key_name)) {
		return 0;
	}

	FarSettingsValue fsv = {
		sizeof(FarSettingsValue)
		, reinterpret_cast<size_t>(keyHandle) 
		, key_name.c_str()
	};
	return (tkey_handle)g_PluginInfo.SettingsControl(_h
		, SCTL_OPENSUBKEY
		, 0
		, &fsv
	);
}


nf::PluginSettings::tsettings_handle nf::PluginSettings::FarCreateKey(tkey_handle keyHandle, tstring const& keyName) {
	tstring key_name;
	if (! ic_search_key_name(keyHandle, keyName, key_name)) {
		key_name = keyName;
	}

	FarSettingsValue fsv = {
		sizeof(FarSettingsValue)
		, reinterpret_cast<size_t>(keyHandle) 
		, keyName.c_str()
	};
	return (tkey_handle)g_PluginInfo.SettingsControl(_h
		, SCTL_CREATESUBKEY
		, 0
		, &fsv
	);
}

bool nf::PluginSettings::FarDeleteKey(tkey_handle keyHandle, tstring const& keyName) {
	auto key = FarOpenKey(keyHandle, keyName);
	if (! nf::PluginSettings::isInvalidHandle(key)) {
		return FarDeleteKey(key);
	} else {
		return false;
	}
}

bool nf::PluginSettings::FarDeleteKey(tkey_handle keyHandle) {
	//из whatsnew: для SCTL_DELETE если FarSettingsValue.Value==NULL то удаляется ключ FarSettingsValue.Root.
	FarSettingsValue fsv = {
		sizeof(FarSettingsValue)
		, reinterpret_cast<size_t>(keyHandle) 
		, nullptr 
	};
	auto ret = g_PluginInfo.SettingsControl(_h
		, SCTL_DELETE
		, 0
		, &fsv
		);
	return ret != 0;

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
	return 0 != g_PluginInfo.SettingsControl(_h
		, SCTL_SET
		, 0
		, &fsi
	);
}
bool nf::PluginSettings::FarGet(tkey_handle keyHandle, tstring const& name, tstring& dest) const {
	FarSettingsItem fsi = {
		sizeof(FarSettingsItem)
		, reinterpret_cast<size_t>(keyHandle) 
		, name.c_str()
		, FST_STRING
		, 0
	};
	if (g_PluginInfo.SettingsControl(_h, SCTL_GET, 0, &fsi)) {
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
		, FST_QWORD
		, number
	};
	return 0 != g_PluginInfo.SettingsControl(_h
		, SCTL_SET
		, 0
		, &fsi
	);
}
bool nf::PluginSettings::FarGet(tkey_handle keyHandle, tstring const& name, __int64& dest) const {
	FarSettingsItem fsi = {
		sizeof(FarSettingsItem)
		, reinterpret_cast<size_t>(keyHandle) 
		, name.c_str()
		, FST_QWORD
		, 0
	};
	if (g_PluginInfo.SettingsControl(_h, SCTL_GET, 0, &fsi)) {
		dest = fsi.Number;
		return true;
	} else {
		return false;
	}
}

bool nf::PluginSettings::FarEnum(tkey_handle keyHandle, FarSettingsEnum& fse) const {
	fse.StructSize = sizeof(FarSettingsEnum);
	fse.Root = reinterpret_cast<size_t>(keyHandle);
	auto code = g_PluginInfo.SettingsControl(_h, SCTL_ENUM, 0, &fse);
	return 0 != code;
}	


nf::PluginSettings::tsettings_handle nf::PluginSettings::FarOpenKey(tkey_handle keyHandle, std::list<tstring> const& keys) const {
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

bool nf::PluginSettings::FarDeleteValue(tkey_handle keyHandle, tstring const& valueName) {
	//из whatsnew: для SCTL_DELETE если FarSettingsValue.Value==NULL то удаляется ключ FarSettingsValue.Root.
	FarSettingsValue fsv = {
		sizeof(FarSettingsValue)
		, reinterpret_cast<size_t>(keyHandle) 
		, valueName.c_str()
	};
	auto ret = g_PluginInfo.SettingsControl(_h
		, SCTL_DELETE
		, 0
		, &fsv
		);
	return ret != 0;
}

bool nf::PluginSettings::copy_key(nf::PluginSettings::tkey_handle hSrc, nf::PluginSettings::tkey_handle hDest) {
	bool bret = true;
	FarSettingsEnum fse;
	fse.StructSize = sizeof(FarSettingsEnum);
	fse.Root = reinterpret_cast<size_t>(hSrc);
	if (0 == g_PluginInfo.SettingsControl(_h, SCTL_ENUM, 0, &fse)) {
		return false;
	}
	for (size_t i = 0; i < fse.Count; ++i) {
		switch (fse.Items[i].Type) {
		case FST_SUBKEY: {
			auto hdest_subkey = nf::PluginSettings::FarCreateKey(hDest, fse.Items[i].Name);
			if (! nf::PluginSettings::isInvalidHandle(hdest_subkey)) {
				auto hsrc_subkey = nf::PluginSettings::FarOpenKey(hSrc, fse.Items[i].Name);
				if (! nf::PluginSettings::isInvalidHandle(hsrc_subkey)) {
					bret &= copy_key(hsrc_subkey, hdest_subkey);
				}
			}
							} break;
		case FST_QWORD: 
			assert(false); //this type is not used in NF
			break;
		case FST_STRING: {
			tstring svalue;
			if (nf::PluginSettings::FarGet(hSrc, fse.Items[i].Name, svalue)) {
				nf::PluginSettings::FarSet(hDest, fse.Items[i].Name, svalue);
			} else {
				bret = false;
			}
							} break;
		case FST_DATA: 
			assert(false); //this type is not used in NF
			break;
		}
	}

	return bret;
}

bool nf::PluginSettings::CopyKey(nf::tlist_strings const& src, nf::tlist_strings const& dest) {
	PluginSettings ps;
	FarCatalog fc_src(src, false);
	FarCatalog fc_dest(dest, false);

	auto hsrc = fc_src.openFarHandle(ps, false);
	auto hdest = fc_dest.openFarHandle(ps, true);
	if (PluginSettings::isInvalidHandle(hsrc) || PluginSettings::isInvalidHandle(hdest)) {
		return false;
	}

	return ps.copy_key(hsrc, hdest);
}

bool nf::PluginSettings::ic_search_key_name(tkey_handle keyHandle, tstring const& keyName, tstring& destKeyName) const {
	nf::SequenceCatalogs sc(*this, keyHandle);
	BOOST_FOREACH(tstring const& name, sc.getItems()) {
		if (0 == g_PluginInfo.FSF->LStricmp(name.c_str(), keyName.c_str())) {
			destKeyName = name;
			return true;
		}
	}
	return false;
}


