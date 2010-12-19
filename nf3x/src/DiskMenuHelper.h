#pragma once

#include <boost/foreach.hpp>
#include <vector>

#include "header.h"
#include "stlcatalogs.h"

namespace nf {
	class DiskMenuHelper { //support of possibility to show catalogs in disk menu
	public:
		DiskMenuHelper() 
			: m_Array(0)
		{
			GetStringsArray();
// 			m_Array = new wchar_t const*[1];
// 			m_Array[0] = nf::GetMsg(lg::DISKMENUSTRING);
		}
		~DiskMenuHelper() {
			delete[] m_Array;
		}
		size_t GetCountStrings() {
			return m_Catalogs.size() + (show_main_string() ? 1 : 0);
		}
		const wchar_t* const* GetStringsArray() {
			m_Catalogs.clear();
			if (CSettings::GetInstance().GetValue(nf::ST_SHOW_CATALOGS_IN_DISK_MENU) != 0) {
				if (m_Array) delete[] m_Array;
				load_list_nf_catalogs();
				m_Array = new wchar_t const*[1 + m_Catalogs.size()];
				m_Array[0] = nf::GetMsg(lg::DISKMENUSTRING);	
				for (unsigned int i = 0; i < m_Catalogs.size(); ++i) {
					tautobuffer_char& b = *m_Catalogs[i].get();
					m_Array[1 + i] = &b[0];
				}
			}
			if (m_Catalogs.size() != 0 && ! show_main_string()) {
				return m_Array + 1;
			} else {
				return m_Array;
			}
		}
		tstring GetCommand(unsigned int diskMenuCommandIndex) {
			if (m_Catalogs.size() < diskMenuCommandIndex - 1) return L"cd:*";
			tautobuffer_char& b = *m_Catalogs[diskMenuCommandIndex - 1].get();
			tstring cmd = &b[0];
			return Utils::ReplaceStringAll(cmd, L"nf:", L"cd:") + L"/*";
		}
	private:
		bool show_main_string() {
			return CSettings::GetInstance().GetValue(nf::ST_SHOW_IN_DISK_MENU) != 0;
		}
		void load_list_nf_catalogs() {
			nf::sc::CCatalog c;
			nf::sc::subcatalogs_sequence seq(c.GetSequenceSubcatalogs());
			m_Catalogs.reserve(seq.size());
			BOOST_FOREACH(sc::catalogs_sequence_item const& csi, seq) {
				tstring name = csi.GetName();
				if (name != L".." && name != L".") {					
					m_Catalogs.push_back(tstring_buffer(Utils::Str2Buffer(L"nf:" + name)));
				}
			}
		}
		std::vector<tstring_buffer> m_Catalogs;
		wchar_t const** m_Array;
	};
}