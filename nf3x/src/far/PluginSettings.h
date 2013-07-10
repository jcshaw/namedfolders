#pragma  once

#include <boost/utility.hpp>

namespace nf {
	class PluginSettings : boost::noncopyable {		
	public:
		PluginSettings();
		~PluginSettings();
		/// HANDLE of plugins settings object, see FAR::SettingsControl
		typedef HANDLE tsettings_handle;
		typedef HANDLE tkey_handle;

		tsettings_handle getHandle() const {
			return _Handle;
		}
	public:
		tsettings_handle FarOpenKey(tkey_handle keyHandle, tstring const& keyName);
		tsettings_handle FarCreateKey(tkey_handle keyHandle, tstring const& keyName);
		bool FarDeleteKey(tkey_handle keyHandle, tstring const& keyName);
		bool FarDeleteKey(tkey_handle keyHandle);

	public:	
		/// @return handle for last path item
		///         INVALID_HANDLE_VALUE - if any item in the path doesn't exist
		tsettings_handle FarOpenKey(tkey_handle keyHandle, std::list<tstring> const& keys);

		/// create all path items if they don't exist
		/// @return handle for last path item
		///         INVALID_HANDLE_VALUE - if any item in the path doesn't exist and it's not possible to create it.
		tsettings_handle FarCreateKey(tkey_handle keyHandle, std::list<tstring> const& keys);

		/// delete last item in the path.
		bool FarDeleteLastKey(tkey_handle keyHandle, std::list<tstring> const& keys);

	public:
		bool FarSet(tkey_handle keyHandle, tstring const& name, tstring const& strValue);
		bool FarGet(tkey_handle keyHandle, tstring const& name, tstring& dest);
		bool FarSet(tkey_handle keyHandle, tstring const& name, __int64 number);
		__int64 FarGet(tkey_handle keyHandle, tstring const& name, __int64 defaultValue);

	public:
		bool FarEnum(tkey_handle keyHandle, FarSettingsEnum& dest);

		static bool isInvalidHandle(tsettings_handle const& h) {
			return h == 0;
		}
	private:
		tsettings_handle _Handle;
	};

}