
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
	static void _issue_win32_error(int number, const char *msg) {
		enum { BUFSIZE = 256 };
		TCHAR buf[BUFSIZE];
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
			number,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),// languageid
			buf, BUFSIZE, NULL
		);
		std::fprintf(stderr, "GetLastError() = %d: %s", number, buf);
		issue_error(msg);
	}
	void sleep(int milliseconds) {
		::Sleep(milliseconds);
	}
	void create_thread( unsigned long (THREADFUNC_CALLING_CONVENTION *func)(void*), void *param ) {
	//void create_thread( THREADFUNC_RETURN_TYPE (*func)(THREADFUNC_CALLING_CONVENTION *), void *param ) {
		HANDLE h = CreateThread( NULL, 32768, func, param, 0, NULL);
		//if (!h) issue_error("create_thread: CreateThread faild", )
		if (!h) {
			std::fprintf(stderr, "failed to create thread");
			std::exit(1);
		}
		if (!CloseHandle(h)) {
			std::fprintf(stderr, "failed to close thread handle");
			std::exit(1);
		}
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
	static void _issue_pthread_error(int number, const char *msg) {
		std::fprintf(stderr, "errno = %d: %s", number, std::strerror(number));
		issue_error(msg);
	}
	void sleep(int milliseconds) {
		if (milliseconds > 1000) {
			::sleep(milliseconds / 1000);
			milliseconds %= 1000;
		}
		usleep(1000 * milliseconds + 1);
	}
	void create_thread( THREADFUNC_RETURN_TYPE (THREADFUNC_CALLING_CONVENTION *func)(void*), void *param ) {
		pthread_t thread;
		if (pthread_create(&thread, NULL, func, param)) {
			_issue_pthread_error(errno, "Threading::create_thread: pthread_create failed");

		}
		if (pthread_detach(thread)) {
			_issue_pthread_error(errno, "Threading::create_thread: pthread_create failed");
		}
	}
	Lock::Lock() {
		//InitializeCriticalSectionAndSpinCount((CRITICAL_SECTION*)&impl_data, 2000);
		if (pthread_mutex_init( (pthread_mutex_t *)&impl_data, NULL )) {
			_issue_pthread_error(errno, "Lock constructor: pthread_mutex_init failed");
		}
	}
	Lock::~Lock() {
		//DeleteCriticalSection((CRITICAL_SECTION*)&impl_data);
		if (pthread_mutex_destroy( (pthread_mutex_t *)&impl_data )) {
			_issue_pthread_error(errno, "Lock destructor: pthread_mutex_destroy failed");
		}
	}
	void Lock::enter() {
		//EnterCriticalSection((CRITICAL_SECTION*)&impl_data);
		if (pthread_mutex_lock( (pthread_mutex_t *)&impl_data )) {
			_issue_pthread_error(errno, "Lock::enter: pthread_mutex_lock failed");
		}
	}
	void Lock::leave() {
		//LeaveCriticalSection((CRITICAL_SECTION*)&impl_data);
		if (pthread_mutex_unlock( (pthread_mutex_t *)&impl_data )) {
			_issue_pthread_error(errno, "Lock::leave: pthread_mutex_unlock failed");
		}
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
