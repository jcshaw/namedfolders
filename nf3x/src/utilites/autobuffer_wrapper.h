#pragma  once 

#include "header.h"

namespace nf {
	//this class is wrapper for nf::tautobuffer_byte with interface of std::vector<int>
	//it allows to avoid using std::vector<int> in the program (reduce size on ~3 kb)
	template<class T>
	class autobuffer_wrapper { 
	public:
		explicit autobuffer_wrapper() : m_Index(0) 
		{}
		T& operator[](unsigned int itemIndex) {
			return *reinterpret_cast<T*>(&m_Buffer[itemIndex * sizeof(T)]);
		}
		void resize(unsigned int countItems) {
			m_Buffer.resize(countItems * sizeof(T));
		}
		void push_back(T const& value) {
			assert(m_Index < size());
			this->operator[](m_Index++) = value;
		}
		size_t size() const {
			return static_cast<size_t>(m_Buffer.size() / sizeof(T));
		}
	private:
		nf::tautobuffer_byte m_Buffer;
		unsigned int m_Index; //counter for filling buffer using push_back
	};
}