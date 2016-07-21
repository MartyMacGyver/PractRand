#ifndef __PRACTRAND_CONFIG_H__
#define __PRACTRAND_CONFIG_H__
/*
Things to configure in this file:
1.  Endianness (usually CPU dependent)
2.  Thread-local-storage aka TLS (usually compiler dependent)
3.  Integer types (compiler and CPU dependent)
*/

/*
1.  Endianness:
	The choices are little-endian and big-endian.  x86 CPUs are little-endian, 
as are ARM, ALPHA, and IA64/Itanium.  MIPs CPUs are big-endian, as are Sparcs, 
PA-RISC, 68k, and almost all PPC and Power.  
	See http://en.wikipedia.org/wiki/Endianness for more information.  
	PractRand does not care about endianness for floating point values, only 
for integer values, so if your CPU and FPU are at odds, go with the CPU.  
	Exactly one of the two endiannesses must be defined or PractRand will not 
compile.  If the wrong one is defined then PractRand will mostly still work, 
though some kinds of things (particularly serialization) may not work 
precisely as intended, and if the PractRands self-test function is called then 
it may report an endianness error.  
*/
#define PRACTRAND_TARGET_IS_LITTLE_ENDIAN 1
//#define PRACTRAND_TARGET_IS_BIG_ENDIAN 1



/*
2.  Thread-local-storage aka TLS
	On Microsoft compilers define this as __declspec(thread).  On gcc it 
should be __thread instead.  Borland and Intel C++ can use either.  
	See http://en.wikipedia.org/wiki/Thread-local_storage for more 
information.  
	If your compiler does not support TLS or supports TLS by an interface 
that is not compatible with PractRand then you can leave this one 
undefined.  PractRand autoseeding will not work as well in multithreaded 
programs, but it will still work.  
*/
#if defined _MSC_VER
#define PRACTRAND_THREAD_LOCAL_STORAGE __declspec(thread)
#elif __GNUC__
#define PRACTRAND_THREAD_LOCAL_STORAGE __thread
#else
//either leave PRACTRAND_THREAD_LOCAL_STORAGE undefined, 
//or define it to something that makes sense for your compiler
//WARNING: PractRand autoseeding might not be thread-safe if it is undefined
#endif



/*
3.  Integer type sizes - reconfigure if needed
	If you know C/C++ programing this one should be self-explanatory.  The 
default configuration should work for most 32 bit and 64 bit platforms.  
If you are on a 16 bit or 8 bit platform then you will have to 
reconfigure it.  PractRand does not use the C99 standard for this because 
that is not yet standard for C++ (soon...), and some compilers I use do 
not yet include the C99 integer size headers.  
	If your compiler does not support 64 bit integers then PractRand can 
not be built at all - many many things in PractRand require them.  
	If this section is misconfigured then PractRand should fail to 
compile with error messages about _compiletime_assert_integer_size.  
*/
namespace PractRand {
	//typical for 32 and 64 bit compiler targets
	typedef unsigned char          Uint8;
	typedef unsigned short int     Uint16;
	typedef unsigned int           Uint32;
	typedef unsigned long long int Uint64;
	typedef signed char          Sint8;
	typedef signed short int     Sint16;
	typedef signed int           Sint32;
	typedef signed long long int Sint64;

	//double-checking those integer sizes:
	typedef char _compiletime_assert_uint8_size [(sizeof(Uint8 )==1) ? 1 : -1];
	typedef char _compiletime_assert_uint16_size[(sizeof(Uint16)==2) ? 1 : -1];
	typedef char _compiletime_assert_uint32_size[(sizeof(Uint32)==4) ? 1 : -1];
	typedef char _compiletime_assert_uint64_size[(sizeof(Uint64)==8) ? 1 : -1];
	typedef char _compiletime_assert_sint8_size [(sizeof(Sint8 )==1) ? 1 : -1];
	typedef char _compiletime_assert_sint16_size[(sizeof(Sint16)==2) ? 1 : -1];
	typedef char _compiletime_assert_sint32_size[(sizeof(Sint32)==4) ? 1 : -1];
	typedef char _compiletime_assert_sint64_size[(sizeof(Sint64)==8) ? 1 : -1];
}

/* 
4. SIMD stuff can greatly speed up the ChaCha RNG
(not used by much yet, only ChaCha so far and only on MSVC)

SIMD strongly NOT recommended at this time, as I haven't come up with a 
clean way to force a properly aligned malloc to be used when appropriate.  
(in theory the spec may guarantee that all mallocs are aligned 
appropriately, but in practice that does not seem to be the case)
I may have to have the Raw::ChaCha class contain pointers at memory blocks 
instead of memory blocks themselves, there doesn't seem to be any other 
way to allow a sane interface.  
*/

#define PRACTRAND_NO_SIMD

#if defined _MSC_VER
#define PRACTRAND_ALIGN_128 __declspec(align(16))
#endif//defined _MSC_VER && _M_IX86_FP > 0


#endif //__PRACTRAND_CONFIG_H__
