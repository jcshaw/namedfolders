#pragma once

/*
Источник:
	взято из статьи Andrei Alexandrescu and Petru Marginean
 	"Simplify Your Exception-Safe Code - Forever"
	http://www.moderncppdesign.com/publications/cuj-12-2000.html
	ftp://ftp.cuj.com/pub/2000/1812/alexander.zip

Summary:
	ScopeGuard is useful when you need to perform automatic cleanup of resources. 
	This idiom is important when you want to assemble an operation out of several 
	atomic operations, each of which could fail.

Examples:
	You use ScopeGuard like so: if you need to do several operations in an 
	"all-or-none" fashion, you put a ScopeGuard after each operation. The 
	execution of that ScopeGuard nullifies the effect of the operation above it:	

		friends_.push_back(&newFriend);		//выполняем операцию
		ScopeGuard guard = MakeObjGuard(  //в случае неудачи она будет 
			friends_, &UserCont::pop_back);	//отменена автоматически
		pDB_->AddFriend(GetName(), 				//выполняем вторую операцию
			newFriend.GetName());
    guard.Dismiss();									//подтверждаем первую


	ScopeGuard works with regular functions, too:
	
		void* buffer = std::malloc(1024);	//выделили ресурс
		ScopeGuard freeIt = MakeGuard(		//при выходе из области видимости
			std::free, buffer);							//ресурс будет автоматически освобожден
		FILE* topSecret = std::fopen("cia.txt");
		ScopeGuard closeIt = MakeGuard(std::fclose, topSecret);


	Supporting Parameters by Reference
	RefHolder and its companion helper function ByRef are ingenious; 
	they seamlessly adapt a reference to a value and allow ScopeGuardImpl1 
	to work with references without any modification. All you have to do 
	is to wrap your references in calls to ByRef, like so:

		void Decrement(int& x) { --x; }
		void UseResource(int refCount)
		{
    	++refCount;
	    ScopeGuard guard = MakeGuard(Decrement, 
				ByRef(refCount));
  	  ...
		}	

	Sometimes, however, you want the ScopeGuard to always execute when 
	you exit the block.  In this case, creating a dummy variable of type 
	ScopeGuard is awkward - you only need a temporary, you don't 
	need a named temporary.
			   ~~~~~
    {
        FILE* topSecret = fopen("cia.txt");
        ON_BLOCK_EXIT(std::fclose, topSecret);
        ... use topSecret ...
    } // topSecret automagically closed
*/


template <class T>
class RefHolder
{
	T& ref_;
public:
	RefHolder(T& ref) : ref_(ref) {}
	operator T& () const 
	{
		return ref_;
	}
private:
    // Disable assignment - not implemented
    RefHolder& operator=(const RefHolder&);
};

template <class T>
inline RefHolder<T> ByRef(T& t)
{
	return RefHolder<T>(t);
}

class ScopeGuardImplBase
{
	ScopeGuardImplBase& operator =(const ScopeGuardImplBase&);
protected:
	~ScopeGuardImplBase()
	{
	}
	ScopeGuardImplBase(const ScopeGuardImplBase& other) throw() 
		: dismissed_(other.dismissed_)
	{
		other.Dismiss();
	}
	template <typename J>
	static void SafeExecute(J& j) throw() 
	{
		if (!j.dismissed_)
			try
			{
				j.Execute();
			}
			catch(...)
			{
			}
	}
	
	mutable bool dismissed_;
public:
	ScopeGuardImplBase() throw() : dismissed_(false) 
	{
	}
	void Dismiss() const throw() 
	{
		dismissed_ = true;
	}
};

typedef const ScopeGuardImplBase& ScopeGuard;

template <typename F>
class ScopeGuardImpl0 : public ScopeGuardImplBase
{
public:
	static ScopeGuardImpl0<F> MakeGuard(F fun)
	{
		return ScopeGuardImpl0<F>(fun);
	}
	~ScopeGuardImpl0() throw() 
	{
		SafeExecute(*this);
	}
	void Execute() 
	{
		fun_();
	}
protected:
	ScopeGuardImpl0(F fun) : fun_(fun) 
	{
	}
	F fun_;
};

template <typename F> 
inline ScopeGuardImpl0<F> MakeGuard(F fun)
{
	return ScopeGuardImpl0<F>::MakeGuard(fun);
}

template <typename F, typename P1>
class ScopeGuardImpl1 : public ScopeGuardImplBase
{
public:
	static ScopeGuardImpl1<F, P1> MakeGuard(F fun, P1 p1)
	{
		return ScopeGuardImpl1<F, P1>(fun, p1);
	}
	~ScopeGuardImpl1() throw() 
	{
		SafeExecute(*this);
	}
	void Execute()
	{
		fun_(p1_);
	}
protected:
	ScopeGuardImpl1(F fun, P1 p1) : fun_(fun), p1_(p1) 
	{
	}
	F fun_;
	const P1 p1_;
};

template <typename F, typename P1> 
inline ScopeGuardImpl1<F, P1> MakeGuard(F fun, P1 p1)
{
	return ScopeGuardImpl1<F, P1>::MakeGuard(fun, p1);
}

template <typename F, typename P1, typename P2>
class ScopeGuardImpl2: public ScopeGuardImplBase
{
public:
	static ScopeGuardImpl2<F, P1, P2> MakeGuard(F fun, P1 p1, P2 p2)
	{
		return ScopeGuardImpl2<F, P1, P2>(fun, p1, p2);
	}
	~ScopeGuardImpl2() throw() 
	{
		SafeExecute(*this);
	}
	void Execute()
	{
		fun_(p1_, p2_);
	}
protected:
	ScopeGuardImpl2(F fun, P1 p1, P2 p2) : fun_(fun), p1_(p1), p2_(p2) 
	{
	}
	F fun_;
	const P1 p1_;
	const P2 p2_;
};

template <typename F, typename P1, typename P2>
inline ScopeGuardImpl2<F, P1, P2> MakeGuard(F fun, P1 p1, P2 p2)
{
	return ScopeGuardImpl2<F, P1, P2>::MakeGuard(fun, p1, p2);
}

template <typename F, typename P1, typename P2, typename P3>
class ScopeGuardImpl3 : public ScopeGuardImplBase
{
public:
	static ScopeGuardImpl3<F, P1, P2, P3> MakeGuard(F fun, P1 p1, P2 p2, P3 p3)
	{
		return ScopeGuardImpl3<F, P1, P2, P3>(fun, p1, p2, p3);
	}
	~ScopeGuardImpl3() throw() 
	{
		SafeExecute(*this);
	}
	void Execute()
	{
		fun_(p1_, p2_, p3_);
	}
protected:
	ScopeGuardImpl3(F fun, P1 p1, P2 p2, P3 p3) : fun_(fun), p1_(p1), p2_(p2), p3_(p3) 
	{
	}
	F fun_;
	const P1 p1_;
	const P2 p2_;
	const P3 p3_;
};

template <typename F, typename P1, typename P2, typename P3>
inline ScopeGuardImpl3<F, P1, P2, P3> MakeGuard(F fun, P1 p1, P2 p2, P3 p3)
{
	return ScopeGuardImpl3<F, P1, P2, P3>::MakeGuard(fun, p1, p2, p3);
}

//************************************************************

template <class Obj, typename MemFun>
class ObjScopeGuardImpl0 : public ScopeGuardImplBase
{
public:
	static ObjScopeGuardImpl0<Obj, MemFun> MakeObjGuard(Obj& obj, MemFun memFun)
	{
		return ObjScopeGuardImpl0<Obj, MemFun>(obj, memFun);
	}
	~ObjScopeGuardImpl0() throw() 
	{
		SafeExecute(*this);
	}
	void Execute() 
	{
		(obj_.*memFun_)();
	}
protected:
	ObjScopeGuardImpl0(Obj& obj, MemFun memFun) 
		: obj_(obj), memFun_(memFun) {}
	Obj& obj_;
	MemFun memFun_;
};

template <class Obj, typename MemFun>
inline ObjScopeGuardImpl0<Obj, MemFun> MakeObjGuard(Obj& obj, MemFun memFun)
{
	return ObjScopeGuardImpl0<Obj, MemFun>::MakeObjGuard(obj, memFun);
}

template <class Obj, typename MemFun, typename P1>
class ObjScopeGuardImpl1 : public ScopeGuardImplBase
{
public:
	static ObjScopeGuardImpl1<Obj, MemFun, P1> MakeObjGuard(Obj& obj, MemFun memFun, P1 p1)
	{
		return ObjScopeGuardImpl1<Obj, MemFun, P1>(obj, memFun, p1);
	}
	~ObjScopeGuardImpl1() throw() 
	{
		SafeExecute(*this);
	}
	void Execute() 
	{
		(obj_.*memFun_)(p1_);
	}
protected:
	ObjScopeGuardImpl1(Obj& obj, MemFun memFun, P1 p1) 
		: obj_(obj), memFun_(memFun), p1_(p1) {}
	Obj& obj_;
	MemFun memFun_;
	const P1 p1_;
};

template <class Obj, typename MemFun, typename P1>
inline ObjScopeGuardImpl1<Obj, MemFun, P1> MakeObjGuard(Obj& obj, MemFun memFun, P1 p1)
{
	return ObjScopeGuardImpl1<Obj, MemFun, P1>::MakeObjGuard(obj, memFun, p1);
}

template <class Obj, typename MemFun, typename P1, typename P2>
class ObjScopeGuardImpl2 : public ScopeGuardImplBase
{
public:
	static ObjScopeGuardImpl2<Obj, MemFun, P1, P2> MakeObjGuard(Obj& obj, MemFun memFun, P1 p1, P2 p2)
	{
		return ObjScopeGuardImpl2<Obj, MemFun, P1, P2>(obj, memFun, p1, p2);
	}
	~ObjScopeGuardImpl2() throw() 
	{
		SafeExecute(*this);
	}
	void Execute() 
	{
		(obj_.*memFun_)(p1_, p2_);
	}
protected:
	ObjScopeGuardImpl2(Obj& obj, MemFun memFun, P1 p1, P2 p2) 
		: obj_(obj), memFun_(memFun), p1_(p1), p2_(p2) {}
	Obj& obj_;
	MemFun memFun_;
	const P1 p1_;
	const P2 p2_;
};

template <class Obj, typename MemFun, typename P1, typename P2>
inline ObjScopeGuardImpl2<Obj, MemFun, P1, P2> MakeObjGuard(Obj& obj, MemFun memFun, P1 p1, P2 p2)
{
	return ObjScopeGuardImpl2<Obj, MemFun, P1, P2>::MakeObjGuard(obj, memFun, p1, p2);
}

#define CONCATENATE_DIRECT(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_DIRECT(s1, s2)
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __LINE__)

#define ON_BLOCK_EXIT ScopeGuard ANONYMOUS_VARIABLE(scopeGuard) = MakeGuard
#define ON_BLOCK_EXIT_OBJ ScopeGuard ANONYMOUS_VARIABLE(scopeGuard) = MakeObjGuard

