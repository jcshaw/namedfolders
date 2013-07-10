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

	/// Sequence of shortcuts / sequence of subcatalogs
	template <class T>
	class SequenceSettings {
		std::list<T> _Items;
	public:
		SequenceSettings(PluginSettings *ps, PluginSettings::tkey_handle key) {
			load_items(ps, key, get_default_FARSETTINGSTYPES((T*)nullptr));
		}

		/// by default we use SequenceShortcuts for shortcuts and SequenceItems for subcatalogs
		/// SequenceShortcuts loads shortcuts values. Sometime, we don't need the values.
		/// Then we can use this constructor, pass FST_STRING and get SequenceItems with shortcut names.
		SequenceSettings(PluginSettings *ps, PluginSettings::tkey_handle key, FARSETTINGSTYPES fst) {
			load_items(ps, key, fst);
		}	

		std::list<T> const& getItems() const {
			return _Items;
		}
	private:
		void load_items(PluginSettings *ps, PluginSettings::tkey_handle key, FARSETTINGSTYPES fst) {
			FarSettingsEnum fse;
			if (ps->FarEnum(key, fse)) {
				for (size_t i = 0; i < fse.Count; ++i) {
					if (fse.Items[i].Type == fst) {
						_Items.push_back(create_sequence_item<T>(ps, key, fse.Items[i]));
					}
				}
			}
		}
	private:
		/// create items for shortucts or subcatalogs sequences
		template<class T> T create_sequence_item(PluginSettings *ps, PluginSettings::tkey_handle key, FarSettingsName const& fsn);
		template<> nf::shortcuts_sequence_item create_sequence_item(PluginSettings *ps, PluginSettings::tkey_handle key, FarSettingsName const& fsn) {
			tstring dest;
			if (! ps->FarGet(key, fsn.Name, dest)) {
				dest.clear();
			}
			return std::make_pair(fsn.Name, dest);
		}
		template<> nf::catalogs_sequence_item create_sequence_item(PluginSettings *ps, PluginSettings::tkey_handle key, FarSettingsName const& fsn) {
			return fsn.Name;
		}

		/// @returns FST_STRING - for shortcuts_sequence_item,  FST_SUBKEY - for catalogs_sequence_item
		template<class T> FARSETTINGSTYPES get_default_FARSETTINGSTYPES(T*);
		template<> FARSETTINGSTYPES get_default_FARSETTINGSTYPES(nf::shortcuts_sequence_item*) {
			return FST_STRING;
		}
		template<> FARSETTINGSTYPES get_default_FARSETTINGSTYPES(nf::catalogs_sequence_item*) {
			return FST_SUBKEY;
		}

	};

	/// sequence of shortcuts with values
	typedef SequenceSettings<nf::shortcuts_sequence_item> SequenceShortcuts;

	/// sequence of subcatalogs OR sequence of shortcuts without values
	typedef SequenceSettings<nf::catalogs_sequence_item> SequenceItems;
}