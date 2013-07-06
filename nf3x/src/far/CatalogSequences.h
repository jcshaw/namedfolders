#pragma once

#include "main/header.h"
#include "PluginSettings.h"
#include "far/plugin.hpp"
#include <list>
namespace nf {
	/// name and value
	typedef std::pair<tstring, tstring> shortcuts_sequence_item;
	// name of subkey
	typedef tstring catalogs_sequence_item;	

	inline tstring const& get_ssi_name(shortcuts_sequence_item const& si) {
		return si.first;
	}

	inline tstring const& get_ssi_value(shortcuts_sequence_item const& si) {
		return si.second;
	}

	inline tstring const& get_catalog_item_name(tstring const& sc) {
		return sc;
	}

	namespace Shims {
		/// create items for shortucts or subcatalogs sequences
		template<class T> 
		inline T create_sequence_item(PluginSettings::tkey_handle key, wchar_t const* name);

		template<> 
		inline nf::shortcuts_sequence_item create_sequence_item(PluginSettings::tkey_handle key, wchar_t const* name) {
			auto sh_value = PluginSettings::FarGet(key, name);
			return std::make_pair(name, sh_value);
		}
		template<> 
		inline nf::catalogs_sequence_item create_sequence_item(PluginSettings::tkey_handle key, wchar_t const* name) {
			return name;
		}

		/// @returns FST_STRING - for shortcuts_sequence_item,  FST_SUBKEY - for catalogs_sequence_item
		template<class T> 
		inline FARSETTINGSTYPES get_default_FARSETTINGSTYPES(T*);

		template<> 
		inline FARSETTINGSTYPES get_default_FARSETTINGSTYPES(nf::shortcuts_sequence_item*) {
			return FST_STRING;
		}
		template<> 
		inline FARSETTINGSTYPES get_default_FARSETTINGSTYPES(nf::catalogs_sequence_item*) {
			return FST_SUBKEY;
		}
	}

	/// Sequence of shortcuts / sequence of subcatalogs
	template <class T>
	class SequenceSettings {
		std::list<T> _Items;
	public:
		SequenceSettings(PluginSettings::tkey_handle key) {
			load_items(key, nf::Shims::get_default_FARSETTINGSTYPES((T*)nullptr));
		}

		/// by default we use SequenceShortcuts for shortcuts and SequenceItems for subcatalogs
		/// SequenceShortcuts loads shortcuts values. Sometime, we don't need the values.
		/// Then we can use this constructor, pass FST_STRING and get SequenceItems with shortcut names.
		SequenceSettings(PluginSettings::tkey_handle key, FARSETTINGSTYPES fst) {
			load_items(key, fst);
		}	

		std::list<T> const& getItems() const {
			return _Items;
		}
	private:
		void load_items(PluginSettings::tkey_handle key, FARSETTINGSTYPES fst) {
			FarSettingsEnum fse;
			if (PluginSettings::FarEnum(key, fse)) {
				for (size_t i = 0; i < fse.Count; ++i) {
					if (fse.Items[i].Type == fst) {
						_Items.push_back(nf::Shims::create_sequence_item<T>(key, fse.Items[i].Name));
					}
				}
			}
		}
	};

	/// sequence of shortcuts with values
	typedef SequenceSettings<nf::shortcuts_sequence_item> SequenceShortcuts;

	/// sequence of subcatalogs OR sequence of shortcuts without values
	typedef SequenceSettings<nf::catalogs_sequence_item> SequenceItems;
}