/*
http://www.cuj.com/documents/s=8250/cujcexp2106alexandr/alexandr.htm
��������� ������ � �����:

int n = ...;
Widget* pWidget = MakeWidget(n);
ENFORCE(pWidget)("Widget number ")(n)(" is null and it shouldn't!");

������ (ENFORCE) ������������ �������� pWidget � ���� ��� �� �����������
�� ��������� �������� �������� (������� ����������).
������
	("Widget number ")(n)(" is null and it shouldn't!")
��������� "�������" ��������� �� ������.

�� ��������� �� ����������� �����, ������� ���������:

* char ������� �� TCHAR
+ ������ COM_ENFORCE
+ namespace'�
+ ComChecker 
+ ������� ����������������: ������ �� ������� ������� ���������� � stdafx.h ������ ���������
*/


#ifndef ENFORCE_H_
#define ENFORCE_H_

#include <windows.h>
#include <string>
#include <sstream>
#include <stdexcept>
#include <tchar.h>

namespace Ext
{

	namespace Private
	{
		struct DefaultPredicate
		{
			template <class T>
			static bool Wrong(const T& obj)
			{
				return !obj;
			}
		};
		
		struct ComChecker
		{
			static bool Wrong(const HRESULT& hr)
			{
				return FAILED(hr);
			}
		};

		struct DefaultRaiser
		{
			template <class T>
			static void Throw(const T&, const std::basic_string<TCHAR>& message, const TCHAR* locus)
			{
		#ifndef UNICODE
				throw std::runtime_error(message + _T('\n') + locus);
		#else
				throw std::runtime_error(static_cast<char const*>(_bstr_t((message + _T('\n') + locus).c_str())));
		#endif
			}
		};
	}

	namespace Shims {
		template<class T> inline
		std::basic_stringstream<TCHAR>& place_message_to_stream(std::basic_stringstream<TCHAR> &Stream, T const& Value) 
		{
			Stream << Value;
			return Stream;
		}
	//���� ���������� �������� ������������� ���� �������� � stream ���-�� �����
	//����������� ���-�� � ������� ��������� �������������
	//template<> inline
	//std::basic_stringstream<TCHAR> Ext::Shims::place_message_to_stream(std::basic_stringstream<TCHAR> &Stream, MyType Value)
	//{ //!TODO };
	}

template<typename Ref, typename P, typename R>
class Enforcer
{
public:
    Enforcer(Ref t, const TCHAR* locus) 
		: t_(t)
		, locus_(locus)
		, pmsg_(P::Wrong(t) ? new std::basic_stringstream<TCHAR>() : 0)
    {
    }
	Enforcer(Enforcer<Ref, P, R> const& S) 
		: t_(S.t_), locus_(S.locus_)
	{
		pmsg_ = S.pmsg_;
		const_cast<Enforcer<Ref, P, R> &>(S).pmsg_ = NULL; //move constructor
	}

	~Enforcer() {
		if (NULL != pmsg_) delete pmsg_;
	}

    Ref operator*() const
    {
		if (NULL != pmsg_) {
			R::Throw(t_, pmsg_->str(), locus_);
		}
        return t_;
    }

    template <class MsgType>
    Enforcer& operator()(const MsgType& msg)
    {
        if (NULL != pmsg_)  {
			Ext::Shims::place_message_to_stream<MsgType>(*pmsg_, msg);
			*pmsg_ << _T(" ");
        }
        return *this;
    }

private:
    Ref t_;
	mutable std::basic_stringstream<TCHAR>* pmsg_;
    TCHAR const* const locus_;
};

template <class P, class R, typename T>
inline Enforcer<const T&, P, R> MakeEnforcer(const T& t, const TCHAR* locus)
{
    return Enforcer<const T&, P, R>(t, locus);
}

template <class P, class R, typename T>
inline Enforcer<T&, P, R> MakeEnforcer(T& t, const TCHAR* locus)
{
    return Enforcer<T&, P, R>(t, locus);
}

template <class P, class R, typename T>
inline Enforcer<const T&, P, R> MakeEnforcerAndPrintExpression(const T& t, const TCHAR* locus)
{
	Enforcer<const T&, P, R> e(t, locus);
	e(t)(_T("\n"));
	return e;
}

template <class P, class R, typename T>
inline Enforcer<T&, P, R> MakeEnforcerAndPrintExpression(T& t, const TCHAR* locus)
{
	Enforcer<T&, P, R> e(t, locus);
	e(t)(_T("\n"));
	return e;
}


#define STRINGIZE(expr) STRINGIZE_HELPER(expr)
#define STRINGIZE_HELPER(expr) #expr

/************************************************************************/
/* ���������� �������� ������� ��������� ��� ����� ��������� � ����������� � stdafx.h

#define ENFORCE(exp) \
	*Ext::MakeEnforcer<Ext::Private::DefaultPredicate, Ext::Private::DefaultRaiser>((exp), _T("Expression '") _T(#exp) _T("' failed in '") \
    _T(__FILE__) _T("', line: ") _T( STRINGIZE(__LINE__)) )

#define COM_ENFORCE(exp) \
	*Ext::MakeEnforcer<Ext::Private::ComChecker, Ext::Private::DefaultRaiser>((exp), _T("Expression '") _T(#exp) _T("' failed in '") \
	_T(__FILE__) _T("', line: ") _T( STRINGIZE(__LINE__)) )
*/

/* ���� ����� ������ ��������� HRESULT (� ������������ ������ ������), �� ����� �������� 
#include "hresult_shims.h"
� ��������� ���� ������ ���:
#define COM_ENFORCE(exp) \
	*Ext::MakeEnforcerAndPrintExpression<Ext::Private::ComChecker2, Ext::Private::DefaultRaiser>((RammusLib::hresult_t(exp)), _T("Expression '") _T(#exp) _T("' failed in '") \
	_T(__FILE__) _T("', line: ") _T( STRINGIZE(__LINE__)))
*/

/************************************************************************/

}; //Ext
#endif

