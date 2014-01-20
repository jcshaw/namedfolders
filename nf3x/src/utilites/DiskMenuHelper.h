#pragma once

#include <boost/foreach.hpp>
#include <vector>
#include <Objbase.h>

#include "header.h"
#include "stlcatalogs.h"
#include "catalog_utils.h"
#include <boost/shared_ptr.hpp>

namespace nf {
	class DiskMenuHelper { //support of possibility to show catalogs in disk menu
		std::vector<tstring_buffer> _Catalogs;
		wchar_t const** _ArrayNames;
		std::vector<boost::shared_ptr<GUID>> _CatalogGuids;
		std::vector<GUID> _ArrayGuids;
	public:
		DiskMenuHelper() 
			: _ArrayNames(0) 
			, _ArrayGuids(0)
		{
			prepare();
		}
		~DiskMenuHelper() {
			delete[] _ArrayNames;
			//delete[] _ArrayGuids;
		}
		size_t GetCountStrings() {
			return _Catalogs.size() + (show_main_string() ? 1 : 0);
		}
		wchar_t const* const* GetStringsArray() {
			return _Catalogs.size() != 0 && ! show_main_string()
				? _ArrayNames + 1
				: _ArrayNames;
		}
		GUID const* GetGuidsPtr() {
			return _CatalogGuids.size() != 0 && ! show_main_string()
				? &(_ArrayGuids)[1]
				: &(_ArrayGuids)[0];
		}
		static tstring GetCommand(GUID const* pSelectedGuid) {
			nf::sc::CCatalog c;
			auto seq = c.GetSequenceSubcatalogs();
			tstring dest_catalog_path;
			return search_by_guid(c, seq.get(), pSelectedGuid, dest_catalog_path)
				? L"cd:" + dest_catalog_path + L"/*"
				: L"cd:*";
		}
	private:
		void prepare() {
			_Catalogs.clear();
			_CatalogGuids.clear();

			if (CSettings::GetInstance().GetValue(nf::ST_SHOW_CATALOGS_IN_DISK_MENU) != 0) {
				load_list_nf_catalogs();
			}
			recreate_arrays();
		}
		void recreate_arrays() {
			if (_ArrayNames) {
				delete[] _ArrayNames;
			}
// 			if (_ArrayGuids) {
// 				delete[] _ArrayGuids;
// 			}

			_ArrayNames = new wchar_t const*[1 + _Catalogs.size()];
			_ArrayNames[0] = nf::GetMsg(lg::DISKMENUSTRING);	

			_ArrayGuids.resize(1 + _CatalogGuids.size());
			_ArrayGuids[0] = nf::NF_DISKMENU_GUID;
			for (unsigned int i = 0; i < _Catalogs.size(); ++i) {
				tautobuffer_char& b = *_Catalogs[i].get();
				_ArrayNames[1 + i] = &b[0];
				_ArrayGuids[1 + i] = *_CatalogGuids[i].get();
			}
		}
		bool show_main_string() {
			return CSettings::GetInstance().GetValue(nf::ST_SHOW_IN_DISK_MENU) != 0;
		}
		void load_list_nf_catalogs() {
			nf::sc::CCatalog c;
			auto seq = c.GetSequenceSubcatalogs();
			_Catalogs.reserve(seq->getItems().size());
			_CatalogGuids.reserve(seq->getItems().size());
			search_catalogs(c, seq.get());
		}

		void search_catalogs(nf::sc::CCatalog &c, nf::SequenceCatalogs* pseq) {
			BOOST_FOREACH(auto const& csi, pseq->getItems()) {
				auto name = nf::get_catalog_item_name(csi);
				if (name != L".." && name != L".") {					
					nf::sc::CCatalog subcatalog(name, &c);
					tstring dest;
					if (subcatalog.GetProperty(nf::sc::CATALOG_PROPERTY_SHOW_IN_DISK_MENU, dest)) {
						if (dest.size() && dest[0] != L'0') {
							GUID guid;
							bool guid_found = subcatalog.GetProperty(nf::sc::CATALOG_PROPERTY_SHOW_IN_DISK_MENU_GUID, dest)
								? SUCCEEDED(CLSIDFromString(dest.c_str(), &guid))
								: false;

							if (! guid_found) {
								//generate new guid for catalog								
								if (SUCCEEDED(::CoCreateGuid(&guid))) {
									OLECHAR szGUID[64] = {0};
									if (0 != ::StringFromGUID2(guid, szGUID, 64)) {
										subcatalog.SetProperty(nf::sc::CATALOG_PROPERTY_SHOW_IN_DISK_MENU_GUID, &szGUID[0]);
										guid_found = true;
									}
								}
							}
							if (guid_found) {
								_CatalogGuids.push_back(boost::shared_ptr<GUID>(new GUID(guid)));
								_Catalogs.push_back(tstring_buffer(Utils::Str2Buffer(L"nf:" + name)));
							}
						}
					}
					//add all embedded catalogs marked by SHOW_IN_DISK_MENU property
					auto seq = subcatalog.GetSequenceSubcatalogs();
					search_catalogs(subcatalog, seq.get());
				}
			}
		}

		static bool search_by_guid(nf::sc::CCatalog &c, nf::SequenceCatalogs* pseq, GUID const* pSelectedGuid, tstring &destCatalogPath) {
			BOOST_FOREACH(auto const& csi, pseq->getItems()) {
				auto name = nf::get_catalog_item_name(csi);
				if (name != L".." && name != L".") {					
					nf::sc::CCatalog subcatalog(name, &c);
					tstring dest;
					if (subcatalog.GetProperty(nf::sc::CATALOG_PROPERTY_SHOW_IN_DISK_MENU, dest)) {
						if (dest.size() && dest[0] != L'0') {
							GUID guid;
							if (subcatalog.GetProperty(nf::sc::CATALOG_PROPERTY_SHOW_IN_DISK_MENU_GUID, dest) && SUCCEEDED(CLSIDFromString(dest.c_str(), &guid))) {
								if (*pSelectedGuid == guid) {
									destCatalogPath = subcatalog.getCatalogPath();
									return true;
								}
							}
						}
					}
					//add all embedded catalogs marked by SHOW_IN_DISK_MENU property
					auto seq = subcatalog.GetSequenceSubcatalogs();
					if (search_by_guid(subcatalog, seq.get(), pSelectedGuid, destCatalogPath)) {
						return true;
					}
				}
			}
			return false;
		}
	};
}