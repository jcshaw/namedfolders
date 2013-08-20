#pragma  once

#include <boost/utility.hpp>

namespace nf {
	class PluginSettings : boost::noncopyable {		
		PluginSettings();
	public:
		~PluginSettings();
		/// HANDLE of plugins settings object, see FAR::SettingsControl
		typedef HANDLE tsettings_handle;
		typedef HANDLE tkey_handle;
		static tsettings_handle getRootHandle();
		static void closeRootHandle();
	public:
		static tsettings_handle FarOpenKey(tkey_handle keyHandle, tstring const& keyName);
		static tsettings_handle FarCreateKey(tkey_handle keyHandle, tstring const& keyName);
		static bool FarDeleteKey(tkey_handle keyHandle, tstring const& keyName);
		static bool FarDeleteKey(tkey_handle keyHandle);
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
		static bool FarGet(tkey_handle keyHandle, tstring const& name, __int64& dest);
		static bool FarDeleteValue(tkey_handle keyHandle, tstring const& valueName);

	public:
		static bool FarEnum(tkey_handle keyHandle, FarSettingsEnum& dest);

		static bool isInvalidHandle(tsettings_handle const& h) {
			return h == 0;
		}
		/// Make copy of source catalog into target path
		/// @param src - source path, i.e. a/Catalogs/c
		/// @param dest - target path, i.e. a/Catalogs/b/Catalogs
		/// @returns true if /a/Catalogs/b/Catalogs/c is successfully created
		static bool CopyKey(nf::tlist_strings const& src, nf::tlist_strings const& dest);
	private:
		/// try to find subkey of keyHandle with name equals to keyName 
		/// main difference from FarOpenKey is that the search is case insensitive
		static bool ic_search_key_name(tkey_handle keyHandle, tstring const& keyName, tstring& destKeyName);
	};

}