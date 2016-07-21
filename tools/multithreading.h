
//the interface:

#if defined WIN32
#define THREADFUNC_CALLING_CONVENTION __stdcall
#define THREADFUNC_RETURN_TYPE unsigned long
#else 
#define THREADFUNC_CALLING_CONVENTION
#define THREADFUNC_RETURN_TYPE void *
#endif
namespace Threading {
	void create_thread( THREADFUNC_RETURN_TYPE (THREADFUNC_CALLING_CONVENTION *threadfunc)(void*), void *param );
	void sleep(int milliseconds);
	class Lock {
		//implementation details hidden (by ugly methods) 
		//in order to avoid platform-specific include files here
		enum {MAX_IMPL_SIZE=64};//make sure this is large enough to hold whatever the real implementation is
		union {
			char impl_data[MAX_IMPL_SIZE];
			long long aligned_data;//to force a reasonable alignment
		};
	public:
		Lock();
		~Lock();
		void enter();
		void leave();
		bool try_enter();//true on success
		void _assert_is_held();//throw exception if lock is not held by current thread
#if defined _DEBUG
		void assert_is_held() {_assert_is_held();}
#else
		void assert_is_held() {}
#endif
	};
}


//the implementation details:

#if defined WIN32
#include <Windows.h>
namespace Threading {
	//compile time assert that Lock is big enough:
	typedef char compile_time_assertion[(sizeof(Lock) >= sizeof(CRITICAL_SECTION)) ? 1 : -1];
	void sleep(int milliseconds) {
		::Sleep(milliseconds);
	}
	void create_thread( unsigned long (THREADFUNC_CALLING_CONVENTION *func)(void*), void *param ) {
	//void create_thread( THREADFUNC_RETURN_TYPE (*func)(THREADFUNC_CALLING_CONVENTION *), void *param ) {
		HANDLE h = CreateThread( NULL, 32768, func, param, 0, NULL);
		if (!h) {
			std::fprintf(stderr, "failed to create thread");
			std::exit(1);
		}
		CloseHandle(h);
	}
	Lock::Lock() {
		InitializeCriticalSectionAndSpinCount((CRITICAL_SECTION*)&impl_data, 2000);
	}
	Lock::~Lock() {
		DeleteCriticalSection((CRITICAL_SECTION*)&impl_data);
	}
	void Lock::enter() {
		EnterCriticalSection((CRITICAL_SECTION*)&impl_data);
	}
	void Lock::leave() {
		LeaveCriticalSection((CRITICAL_SECTION*)&impl_data);
	}
	bool Lock::try_enter() {
		return TryEnterCriticalSection((CRITICAL_SECTION*)&impl_data) ? true : false;
	}
	void Lock::_assert_is_held() {
#ifdef _DEBUG
		CRITICAL_SECTION * cs = (CRITICAL_SECTION*)&impl_data;
		if (DWORD(cs->OwningThread) != GetCurrentThreadId()) {
			std::fprintf(stderr, "lock not held");
			std::exit(1);
		}
#endif
	}
}
#else //unix????
#include <unistd.h>
#include <pthread.h>
namespace Threading {
	//compile time assert that Lock is big enough:
	typedef char compile_time_assertion[(sizeof(Lock) >= sizeof(pthread_mutex_t)) ? 1 : -1];
	void sleep(int milliseconds) {
		if (milliseconds > 1000) {
			::sleep(milliseconds / 1000);
			milliseconds %= 1000;
		}
		usleep(1000 * milliseconds + 1);
	}
	void create_thread( THREADFUNC_RETURN_TYPE (THREADFUNC_CALLING_CONVENTION *func)(void*), void *param ) {
		pthread_t thread;
		pthread_create(&thread, NULL, func, param);
	}
	Lock::Lock() {
		//InitializeCriticalSectionAndSpinCount((CRITICAL_SECTION*)&impl_data, 2000);
		pthread_mutex_init( (pthread_mutex_t *)&impl_data, NULL );
	}
	Lock::~Lock() {
		//DeleteCriticalSection((CRITICAL_SECTION*)&impl_data);
		pthread_mutex_destroy( (pthread_mutex_t *)&impl_data );
	}
	void Lock::enter() {
		//EnterCriticalSection((CRITICAL_SECTION*)&impl_data);
		pthread_mutex_lock( (pthread_mutex_t *)&impl_data );
	}
	void Lock::leave() {
		//LeaveCriticalSection((CRITICAL_SECTION*)&impl_data);
		pthread_mutex_unlock( (pthread_mutex_t *)&impl_data );
	}
	bool Lock::try_enter() {
		//return TryEnterCriticalSection((CRITICAL_SECTION*)&impl_data) ? true : false;
		return pthread_mutex_trylock( (pthread_mutex_t *)&impl_data ) ? false : true;
	}
	//how to implement this in pthreads?
	/*void Lock::_assert_is_held() {
#ifdef _DEBUG
		CRITICAL_SECTION * cs = (CRITICAL_SECTION*)&impl_data;
		if (DWORD(cs->OwningThread) != GetCurrentThreadId()) {
			std::fprintf(stderr, "lock not held");
			std::exit(1);
		}
#endif
	}*/
}
#endif
