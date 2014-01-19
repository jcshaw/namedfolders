#pragma  once

#include <boost/utility.hpp>

namespace nf {
	/// Wrapper around FAR Sqlite API
	class PluginSettings : boost::noncopyable {		
		HANDLE _h;
	public:
		PluginSettings();
		~PluginSettings();
		/// HANDLE of plugins settings object, see FAR::SettingsControl
		typedef HANDLE tsettings_handle;
		typedef HANDLE tkey_handle;
	public:
		tsettings_handle FarOpenKey(tkey_handle keyHandle, tstring const& keyName) const;
		tsettings_handle FarCreateKey(tkey_handle keyHandle, tstring const& keyName);
		bool FarDeleteKey(tkey_handle keyHandle, tstring const& keyName);
		bool FarDeleteKey(tkey_handle keyHandle);
	public:	
		/// @return handle for last path item
		///         INVALID_HANDLE_VALUE - if any item in the path doesn't exist
		tsettings_handle FarOpenKey(tkey_handle keyHandle, std::list<tstring> const& keys) const;

		/// create all path items if they don't exist
		/// @return handle for last path item
		///         INVALID_HANDLE_VALUE - if any item in the path doesn't exist and it's not possible to create it.
		tsettings_handle FarCreateKey(tkey_handle keyHandle, std::list<tstring> const& keys);

		/// delete last item in the path.
		bool FarDeleteLastKey(tkey_handle keyHandle, std::list<tstring> const& keys);

	public:
		bool FarSet(tkey_handle keyHandle, tstring const& name, tstring const& strValue);
		bool FarGet(tkey_handle keyHandle, tstring const& name, tstring& dest) const;
		bool FarSet(tkey_handle keyHandle, tstring const& name, __int64 number);
		bool FarGet(tkey_handle keyHandle, tstring const& name, __int64& dest) const;
		bool FarDeleteValue(tkey_handle keyHandle, tstring const& valueName);

	public:
		bool FarEnum(tkey_handle keyHandle, FarSettingsEnum& dest) const;

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
		bool ic_search_key_name(tkey_handle keyHandle, tstring const& keyName, tstring& destKeyName) const;
		bool copy_key(tkey_handle hSrc, tkey_handle hDest);
	};

}