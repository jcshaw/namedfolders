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
		/// @return handle for last path item
		///         INVALID_HANDLE_VALUE - if any item in the path doesn't exist
		static tsettings_handle FarOpenKey(tkey_handle keyHandle, std::list<tstring> const& keys);

		/// create all path items if they don't exist
		/// @return handle for last path item
		///         INVALID_HANDLE_VALUE - if any item in the path doesn't exist and it's not possible to create it.
		static tsettings_handle FarCreateKey(tkey_handle keyHandle, std::list<tstring> const& keys);

		/// delete last item in the path.
		static bool FarDeleteLastKey(tkey_handle keyHandle, std::list<tstring> const& keys);

	public:
		static bool FarSet(tkey_handle keyHandle, tstring const& name, tstring const& strValue);
		static bool FarGet(tkey_handle keyHandle, tstring const& name, tstring& dest);
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