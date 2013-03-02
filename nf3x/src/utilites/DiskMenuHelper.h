#pragma once

#include <boost/foreach.hpp>
#include <vector>

#include "header.h"
#include "stlcatalogs.h"
#include "catalog_utils.h"

namespace nf {
	class DiskMenuHelper { //support of possibility to show catalogs in disk menu
		std::vector<tstring_buffer> _Catalogs;
		wchar_t const** _ArrayNames;
		std::vector<GUID> _CatalogGuids;
		GUID const** _ArrayGuids;
	public:
		DiskMenuHelper() 
			: _ArrayNames(0) 
			, _ArrayGuids(0)
		{
			prepare();
		}
		~DiskMenuHelper() {
			delete[] _ArrayNames;
			delete[] _ArrayGuids;
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
				? (_ArrayGuids + 1)[0]
				: (_ArrayGuids)[0];
		}
		tstring GetCommand(unsigned int diskMenuCommandIndex) {
			if (_Catalogs.size() < diskMenuCommandIndex - 1) return L"cd:*";
			tautobuffer_char& b = *_Catalogs[diskMenuCommandIndex - 1].get();
			tstring cmd = &b[0];
			return Utils::ReplaceStringAll(cmd, L"nf:", L"cd:") + L"/*";
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
			_ArrayNames = new wchar_t const*[1 + _Catalogs.size()];
			_ArrayNames[0] = nf::GetMsg(lg::DISKMENUSTRING);	
			for (unsigned int i = 0; i < _Catalogs.size(); ++i) {
				tautobuffer_char& b = *_Catalogs[i].get();
				_ArrayNames[1 + i] = &b[0];
			}
		}
		bool show_main_string() {
			return CSettings::GetInstance().GetValue(nf::ST_SHOW_IN_DISK_MENU) != 0;
		}
		void load_list_nf_catalogs() {
			nf::sc::CCatalog c;
			nf::sc::subcatalogs_sequence seq(c.GetSequenceSubcatalogs());
			_Catalogs.reserve(seq.size());
			BOOST_FOREACH(sc::catalogs_sequence_item const& csi, seq) {
				tstring name = csi.GetName();
				if (name != L".." && name != L".") {					
					nf::sc::CCatalog subcatalog(name, &c);
					tstring dest;
					if (subcatalog.GetProperty(nf::sc::CATALOG_PROPERTY_SHOW_IN_DISK_MENU, dest)) {
						if (dest.size() && dest[0] != L'0') {
							_Catalogs.push_back(tstring_buffer(Utils::Str2Buffer(L"nf:" + name)));
						}
					}
				}
			}
		}
	};
}