#pragma once

#include "main/header.h"
#include "PluginSettings.h"
#include "far/plugin.hpp"
#include <list>
namespace nf {
	/// name and value
	typedef std::pair<tstring, tstring> titem_sequence_values;
	// name of subkey
	typedef tstring titem_sequence_catalogs;	

	inline tstring const& get_ssi_name(titem_sequence_values const& si) {
		return si.first;
	}

	inline tstring const& get_ssi_value(titem_sequence_values const& si) {
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
		SequenceSettings(nf::PluginSettings const& ps, PluginSettings::tkey_handle key) {
			load_items(ps, key, get_default_FARSETTINGSTYPES((T*)nullptr));
		}

		/// by default we use SequenceShortcuts for shortcuts and SequenceItems for subcatalogs
		/// SequenceShortcuts loads shortcuts values. Sometime, we don't need the values.
		/// Then we can use this constructor, pass FST_STRING and get SequenceItems with shortcut names.
		SequenceSettings(nf::PluginSettings const& ps, PluginSettings::tkey_handle key, FARSETTINGSTYPES fst) {
			load_items(ps, key, fst);
		}	

		std::list<T> const& getItems() const {
			return _Items;
		}
	private:
		void load_items(nf::PluginSettings const& ps, PluginSettings::tkey_handle key, FARSETTINGSTYPES fst) {
			FarSettingsEnum fse;
			if (ps.FarEnum(key, fse)) {
				for (size_t i = 0; i < fse.Count; ++i) {
					if (fse.Items[i].Type == fst) {
						_Items.push_back(create_sequence_item<T>(ps, key, fse.Items[i]));
					}
				}
			}
		}
	private:
		/// create items for shortucts or subcatalogs sequences
		template<class T> T create_sequence_item(nf::PluginSettings const& ps, PluginSettings::tkey_handle key, FarSettingsName const& fsn);
		template<> nf::titem_sequence_values create_sequence_item(nf::PluginSettings const& ps, PluginSettings::tkey_handle key, FarSettingsName const& fsn) {
			tstring dest;
			if (! ps.FarGet(key, fsn.Name, dest)) {
				dest.clear();
			}
			return std::make_pair(fsn.Name, dest);
		}
		template<> nf::titem_sequence_catalogs create_sequence_item(nf::PluginSettings const& ps, PluginSettings::tkey_handle key, FarSettingsName const& fsn) {
			return fsn.Name;
		}

		/// @returns FST_STRING - for shortcuts_sequence_item,  FST_SUBKEY - for catalogs_sequence_item
		template<class T> FARSETTINGSTYPES get_default_FARSETTINGSTYPES(T*);
		template<> FARSETTINGSTYPES get_default_FARSETTINGSTYPES(nf::titem_sequence_values*) {
			return FST_STRING;
		}
		template<> FARSETTINGSTYPES get_default_FARSETTINGSTYPES(nf::titem_sequence_catalogs*) {
			return FST_SUBKEY;
		}

	};

	/// sequence of shortcuts with values
	typedef SequenceSettings<nf::titem_sequence_values> SequenceValues;

	/// sequence of subcatalogs OR sequence of shortcuts without values
	typedef SequenceSettings<nf::titem_sequence_catalogs> SequenceCatalogs;
}