#pragma  once

namespace nf {
	class PluginSettings {
		PluginSettings();
	public:
		~PluginSettings();
		/// HANDLE of plugins settings object, see FAR::SettingsControl
		typedef HANDLE tsettings_handle;
		typedef HANDLE tkey_handle;

		/// access to plugin settings object
		/// use PluginSettings::getHandle() to have access
		static tsettings_handle getHandle();
	public:
		static tsettings_handle FarOpenKey(tkey_handle keyHandle, tstring const& keyName);
		static tsettings_handle FarCreateKey(tkey_handle keyHandle, tstring const& keyName);
		static bool FarDeleteKey(tkey_handle keyHandle, tstring const& keyName);

	public:
		static bool FarSet(tkey_handle keyHandle, tstring const& name, tstring const& strValue);
		static tstring FarGet(tkey_handle keyHandle, tstring const& name);
		static bool FarSet(tkey_handle keyHandle, tstring const& name, __int64 number);
		static __int64 FarGet(tkey_handle keyHandle, tstring const& name, __int64 defaultValue);

	public:
		static bool FarEnum(tkey_handle keyHandle, FarSettingsEnum& dest);

		static bool isInvalidHandle(tsettings_handle const& h) {
			return h == 0;
		}

	private:
		tsettings_handle const _Handle;
	};

}