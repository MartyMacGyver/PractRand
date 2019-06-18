
#ifdef _WIN32
#define _CRT_RAND_S
#include <windows.h>
#elif defined __APPLE__ && defined __MACH__
#include <libkern/OSAtomic.h>
#include <cstdint>
#endif
#ifdef _MSC_VER
#include <intrin.h>
#endif
#include <string>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"



/*
Two functions are performed that may need to use platform-specific functionality:
	1. obtaining entropy for seeding
		preferably by asking the system cryptographic random number generator for the entropy (eg /dev/urandom)
		if that is not possible, then use the time (as measured with libc calls) instead
			to do: add a fallback case that blocks while listening to timing noise for entropy
	2. obtaining a unique 64 bit number that should never duplicate, and should be thread-safe.
		preferably with an atomic increment
		if that is not possible then with a malloc(1) that is never freed
			(this should get used at most once per thread creation)
*/

using namespace PractRand;

bool PractRand::Internals::add_entropy_automatically( PractRand::RNGs::vRNG *rng, int milliseconds ) {
	//the intention is for "millisecond" to be an amount of time that this function is permitted to spend on obtaining entropy
	//but currently nothing that spends time in a controlled fashion is implemented, so it's meaningless

	enum {DESIRED_BITS = 256};
	enum {N32 = (DESIRED_BITS+31)/32, N64 = (DESIRED_BITS+63)/64};

#if (defined _WIN32) && 0 // DISABLED
	{//win32 crypto PRNG, simple interface
		//disabled because MinGW won't compile it and MSVC won't put it in the standard namespace
		int count = 0;
		unsigned int tmp = 0;
		for (int i = 0; i < N32 + 8; i++) {
			count += (!rand_s( &tmp )) ? 1 : 0;
			rng->add_entropy32(tmp);
			if (count == N32) {
				rng->flush_buffers();
				return true;
			}
		}
	}
#endif
#if (defined _WIN32) && 1
	{//win32 crypto PRNG, another interface
		HMODULE hLib=LoadLibraryA("ADVAPI32.DLL");
		if (hLib) {
			BOOLEAN (APIENTRY *get_random_bytes)(void*, ULONG) =
				(BOOLEAN (APIENTRY *)(void*,ULONG))GetProcAddress(hLib,"SystemFunction036");
			if (get_random_bytes) {
				Uint64 buf[N64];
				if(get_random_bytes(buf,N64*sizeof(buf[0]))) {
					for (int i = 0; i < N64; i++) rng->add_entropy64(buf[i]);
					FreeLibrary(hLib);
					rng->flush_buffers();
					std::memset(buf, 0, sizeof(buf));
					return true;
				}
			}
			FreeLibrary(hLib);
		}
	}
#endif
#if 1
	{//unix (linux/bsd/osx/etc, all flavors supposedly)
		//mostly safe to use even on platforms where it won't work
		std::FILE *f;
		Uint64 buf[N64];
		if (f = std::fopen("/dev/urandom", "rb")) {
			if (std::fread(buf,N64*sizeof(buf[0]),1,f) == 1) {
				for (int i = 0; i < N64; i++) rng->add_entropy64(buf[i]);
				std::fclose(f);
				rng->flush_buffers();
				std::memset(buf, 0, sizeof(buf));
				return true;
			}
			std::fclose(f);
		}
	}
#endif
#if 0 //DISABLED
	{
	//disabled to avoid the possibility of blocking
		if (millseconds && f = std::fopen("/dev/random", "rb")) {
			//skip this if a good source was already found, because this can block
			Uint64 buf[N64];
			if(std::fread(buf,N64*sizeof(buf[0]),1,f)) {
				for (int i = 0; i < N64; i++) rng->add_entropy64(buf[i]);
				rng->flush_buffers();
				std::memset(buf, 0, sizeof(buf));
				return true;
			}
		}
	}
#endif
#if 1
	{//libc
		//not much entropy, but we take what we can get
		rng->add_entropy64((Uint64)std::time(NULL));
		rng->add_entropy64((Uint64)std::clock());
		rng->add_entropy64((Uint64)rng);
		Uint64 *p = (Uint64*)std::malloc(sizeof(Uint64));
		rng->add_entropy64((Uint64)p);
		//rng->add_entropy64(*p);//commented to avoid issues with memory debuggers
		free(p);
	}
#endif
#if (defined _WIN32) && 1
	{//win32 (intended to work on win2k and later)
		::LARGE_INTEGER qt;
		QueryPerformanceCounter(&qt);
		rng->add_entropy64(qt.QuadPart);
		rng->add_entropy32(::GetCurrentProcessId());
		rng->add_entropy64((Uint64)::GetCurrentProcess());
		rng->add_entropy32(::GetCurrentThreadId());
		//rng->add_entropy32(::GetCurrentProcessorNumber());//may require Vista?
		SYSTEMTIME st;
		::GetSystemTime(&st);
		rng->add_entropy_N(&st, sizeof(st));
		MEMORYSTATUS mem;
		GlobalMemoryStatus(&mem);
		rng->add_entropy_N(&mem, sizeof(mem));
		DWORD t = GetCurrentTime();
		rng->add_entropy32(t);
	}
#endif
#if defined _MSC_VER && ((defined _M_IX86 && _M_IX86 >= 500) || defined _M_X64 || defined _M_AMD64)
	{
		rng->add_entropy64(__rdtsc());
	}
#endif
#if (defined _WIN32) && 0 //DISABLED
	{//WINDOWS REGISTRY
		//Not a true entropy source, but an accumulator across multiple runs, 
		//  which we need more than we need another entropy source.  
		//Probably ought to be a transaction,
		//  but that would require Vista as a minimum Windows version, 
		//  and I want to support 2k and XP also
		HKEY key = NULL;
		for (int tries = 0; tries < 3; tries++) {
			//printf("attempt %d\n", tries);
			if (key) {
				RegCloseKey(key);
				key = NULL;
			}
			DWORD d;
			long r;
			r = RegCreateKeyExA(
				HKEY_CURRENT_USER,
				"Software\\PractRand",
				0,
				NULL,
				REG_OPTION_NON_VOLATILE,
				KEY_QUERY_VALUE | KEY_SET_VALUE,
				NULL,
				&key,
				&d
			);
			if (r != ERROR_SUCCESS) continue;
			//printf("%s\n", (d==REG_CREATED_NEW_KEY) ? "failed to find key, creating" : "found key");
			enum {TARGET_SIZE = 128, BUFFER_SIZE=512};
			Uint8 buffer[BUFFER_SIZE];
			DWORD size = BUFFER_SIZE;
			DWORD type;
			//to do: use different seed names for different EntropyPool algorithms
			r = RegQueryValueExA(key, "seed", NULL, &type, buffer, &size);
			if (r == ERROR_MORE_DATA || type != REG_BINARY) {
				r = ERROR_FILE_NOT_FOUND;
				d = REG_CREATED_NEW_KEY;
				//printf("seed invalid\n");
			}
			if (r == ERROR_FILE_NOT_FOUND && (d == REG_CREATED_NEW_KEY || (tries > 0)) ) {
				//printf("resetting seed (either invalid or failed to read multiple times)\n");
				rng->flush_buffers();
				for (int i = 0; i < TARGET_SIZE; i++) buffer[i] = rng->raw8();
				r = RegSetValueExA(key, "seed", 0, REG_BINARY, buffer, TARGET_SIZE);
			}
			if (r != ERROR_SUCCESS) continue;
			//std::printf("using seed (%02x%02x)\n", buffer[1], buffer[0]);
			rng->add_entropy_N(buffer, size);
			if (size > TARGET_SIZE) size = TARGET_SIZE;
			rng->flush_buffers();
			//std::printf("buffers flushed\n");
			for (unsigned int i = 0; i < size; i++) buffer[i] ^= rng->raw8();
			for (unsigned int i = size; i < TARGET_SIZE; i++) buffer[i] = rng->raw8();
			//std::printf("attempting to set new seed (%02x%02x)\n", buffer[1], buffer[0]);
			r = RegSetValueExA(key, "seed", 0, REG_BINARY, buffer, TARGET_SIZE);
			if (r != ERROR_SUCCESS) continue;
			//std::printf("successfully set new seed\n");
			break;
		}
		if (key) {
			RegCloseKey(key);
			key = NULL;
		}
	}
#endif //_WIN32 (registry)

#if (defined _WIN32) && 0
	if (milliseconds && !good_entropy_source_found) {//accumulate entropy over time (does a VERY bad job)
		LARGE_INTEGER qt, qt2;
		QueryPerformanceCounter(&qt);
		DWORD start_time = GetTickCount();
		DWORD t = start_time;
		rng->add_entropy32(t);
		long count=0;
		while (t < start_time + milliseconds) {
			if (QueryPerformanceCounter(&qt2)) {
				if (qt2.QuadPart != qt.QuadPart) {
					rng->add_entropy32(qt2.LowPart);
					qt.QuadPart = qt2.QuadPart;
				}
			}
			count++;
			DWORD t2 = GetTickCount();
			if (t != t2) {
				rng->add_entropy32(t2 + count);
				t = t2;
			}
		}
		rng->add_entropy16(Uint16(count));
		milliseconds = 0;
	}
#endif

	rng->flush_buffers();
	return false;
}

Uint64 PractRand::Internals::issue_unique_identifier ( ) {
#if 0
#elif defined __GNUC__
	static volatile Uint64 count = 0;
	return __sync_fetch_and_add( &count, Uint64(1) );
#elif defined _WIN32
	static volatile LONGLONG count = 0;
	return InterlockedIncrement64( &count);
#elif defined __APPLE__ && defined __MACH__
	//OS X, /usr/include/libkern/OSAtomic.h, OSAtomicIncrement64
	static volatile int64_t count = 0;
	return OSAtomicIncrement64(&count);
#else
	//ugly, but without more knowledge of the target system or more dependencies there's not much more that can be done
	return (Uint64)std::malloc(1);
#endif
}

/*
	//don't care about the units since it's only used as an entropy source
	//however, rdtscp is to be avoided since not enough CPUs support it
Uint64 PractRand::Internals::high_resolution_time() {
#if defined _MSC_VER && ( defined _M_IX86 || defined _M_X64 )
	//intrinsic
	return __rdtsc();
	//Uint32 aux;
	//return __rdtscp(&aux);
#elif defined __GNUC__ && ( defined(__i386__) || defined(__x86_64__) )
	//from wikipedia
	Uint32 low, high;
	__asm__ __volatile__("rdtsc" : "=a"(low), "=d"(high) :: "ecx" );
	//__asm__ __volatile__("rdtscp" : "=a"(low), "=d"(high) :: "ecx" );
	return (Uint64(high) << 32) | low;
#elif defined __GNUC__ && defined(__powerpc__)
	//from http://www.mcs.anl.gov/~kazutomo/rdtsc.html
	Uint64 result = 0;
	unsigned long int upper, lower,tmp;
	__asm__ volatile(
		"0:                  \n"
		"\tmftbu   %0           \n"
		"\tmftb    %1           \n"
		"\tmftbu   %2           \n"
		"\tcmpw    %2,%0        \n"
		"\tbne     0b         \n"
		: "=r"(upper),"=r"(lower),"=r"(tmp)
	);
	result = upper;
	result = result<<32;
	result = result|lower;

	return(result);
#elif defined WIN32
	LARGE_INTEGER qt;
	QueryPerformanceCounter(&qt);
	return qt.QuadPart;
#else
	//to do: figure out the appropriate preprocessor defines to check for gettimeofday

	//very poor resolution, but we don't have a lot of alternatives at this point:
	return (Uint64)std::clock();
#endif
}
*/
