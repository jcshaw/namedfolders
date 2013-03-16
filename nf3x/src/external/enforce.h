/*
http://www.cuj.com/documents/s=8250/cujcexp2106alexandr/alexandr.htm
Обработка ошибок в стиле:

int n = ...;
Widget* pWidget = MakeWidget(n);
ENFORCE(pWidget)("Widget number ")(n)(" is null and it shouldn't!");

Макрос (ENFORCE) осуществляет проверку pWidget и если она не выполняется
то выполняет заданное действие (генерит исключение).
Строка
	("Widget number ")(n)(" is null and it shouldn't!")
позволяет "собрать" сообщение об ошибке.

По сравнению со стандартным кодом, внесены изменения:

* char заменен на wchar_t
+ макрос COM_ENFORCE
+ namespace'ы
+ ComChecker 
+ макросы закомментированы: теперь их следует вручную объявления в stdafx.h каждой программы
*/


#ifndef ENFORCE_H_
#define ENFORCE_H_

#include <windows.h>
#include <string>
#include <sstream>
#include <stdexcept>

#define _ENFORCER_WIDEN2(x) L ## x
#define _ENFORCER_WIDEN(x) _ENFORCER_WIDEN2(x)

namespace Ext
{

	namespace Private
	{
		struct DefaultPredicate {
			template <class T>
			static bool Wrong(const T& obj)
			{
				return !obj;
			}
		};
		
		struct DefaultRaiser {
			template <class T>
			static void Throw(const T&, const std::basic_string<wchar_t>& message, const wchar_t* locus) {
				throw std::runtime_error(message + std::basic_string<char_t>('\n') + locus);
			}
		};
	}

	namespace Shims {
		template<class T> inline
		std::basic_stringstream<wchar_t>& place_message_to_stream(std::basic_stringstream<wchar_t> &Stream, T const& Value) 
		{
			Stream << Value;
			return Stream;
		}
	//если необходимо значения определенного типа выводить в stream как-то хитро
	//необходиомо где-то в проекте объявлить специализацию
	//template<> inline
	//std::basic_stringstream<wchar_t> Ext::Shims::place_message_to_stream(std::basic_stringstream<wchar_t> &Stream, MyType Value)
	//{ //!TODO };
	}

template<typename Ref, typename P, typename R>
class Enforcer
{
public:
    Enforcer(Ref t, const wchar_t* locus) 
		: t_(t)
		, locus_(locus)
		, pmsg_(P::Wrong(t) ? new std::basic_stringstream<wchar_t>() : 0)
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
	mutable std::basic_stringstream<wchar_t>* pmsg_;
    wchar_t const* const locus_;
};

template <class P, class R, typename T>
inline Enforcer<const T&, P, R> MakeEnforcer(const T& t, const wchar_t* locus)
{
    return Enforcer<const T&, P, R>(t, locus);
}

template <class P, class R, typename T>
inline Enforcer<T&, P, R> MakeEnforcer(T& t, const wchar_t* locus)
{
    return Enforcer<T&, P, R>(t, locus);
}

template <class P, class R, typename T>
inline Enforcer<const T&, P, R> MakeEnforcerAndPrintExpression(const T& t, const wchar_t* locus)
{
	Enforcer<const T&, P, R> e(t, locus);
	e(t)(_T("\n"));
	return e;
}

template <class P, class R, typename T>
inline Enforcer<T&, P, R> MakeEnforcerAndPrintExpression(T& t, const wchar_t* locus)
{
	Enforcer<T&, P, R> e(t, locus);
	e(t)(_T("\n"));
	return e;
}


#define STRINGIZE(expr) STRINGIZE_HELPER(expr)
#define STRINGIZE_HELPER(expr) #expr

/************************************************************************/
/* Объявления макросов следует настроить под нужды программы и скопировать в stdafx.h

#define ENFORCE(exp) \
	*Ext::MakeEnforcer<Ext::Private::DefaultPredicate, Ext::Private::DefaultRaiser>((exp), _T("Expression '") _T(#exp) _T("' failed in '") \
    _T(__FILE__) _T("', line: ") _T( STRINGIZE(__LINE__)) )

#define COM_ENFORCE(exp) \
	*Ext::MakeEnforcer<Ext::Private::ComChecker, Ext::Private::DefaultRaiser>((exp), _T("Expression '") _T(#exp) _T("' failed in '") \
	_T(__FILE__) _T("', line: ") _T( STRINGIZE(__LINE__)) )
*/

/* Если нужна хитрая поддержка HRESULT (с расшифровкой текста ошибки), то нужно включить 
#include "hresult_shims.h"
и объявлять этот макрос так:
#define COM_ENFORCE(exp) \
	*Ext::MakeEnforcerAndPrintExpression<Ext::Private::ComChecker2, Ext::Private::DefaultRaiser>((RammusLib::hresult_t(exp)), _T("Expression '") _T(#exp) _T("' failed in '") \
	_T(__FILE__) _T("', line: ") _T( STRINGIZE(__LINE__)))
*/

/************************************************************************/

}; //Ext
#endif

