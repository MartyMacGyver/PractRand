#ifndef __PRACTRAND_ENDIAN_H__
#define __PRACTRAND_ENDIAN_H__

#ifndef __PRACTRAND_CONFIG_H__
#include "config.h"
#endif

#if defined PRACTRAND_TARGET_IS_LITTLE_ENDIAN && defined PRACTRAND_TARGET_IS_BIG_ENDIAN
	#error "PractRand has conflicting endianness settings, see PractRand/config.h"
#endif
#if (!defined PRACTRAND_TARGET_IS_LITTLE_ENDIAN) && (!defined PRACTRAND_TARGET_IS_BIG_ENDIAN)
	#error "PractRand has no endianness setting, see PractRand/config.h"
#endif

namespace PractRand {
	static inline Uint16 invert_endianness16(Uint16 v) {return (v >> 8) | (v << 8);}
	static inline Uint32 invert_endianness32(Uint32 v) {
		v = ((v & 0xFF00FF00) >> 8) | ((v & 0x00ff00ff) << 8);
		return (v >> 16) | (v << 16);
	}
	static inline Uint64 invert_endianness64(Uint64 v) {
		v = ((v & 0xFF00FF00FF00FF00ull) >> 8) | ((v & 0x00ff00ff00ff00ffull) << 8);
		v = ((v & 0xFFFF0000FFFF0000ull) >> 16) | ((v & 0x0000ffff0000ffffull) << 16);
		return (v >> 32) | (v << 32);
	}
#if defined PRACTRAND_TARGET_IS_LITTLE_ENDIAN
	static inline Uint16 little_endian_conversion16 ( Uint16 v ) {return v;}
	static inline Uint32 little_endian_conversion32 ( Uint32 v ) {return v;}
	static inline Uint64 little_endian_conversion64 ( Uint64 v ) {return v;}
	static inline Uint16 big_endian_conversion16 ( Uint16 v ) {return invert_endianness16(v);}
	static inline Uint32 big_endian_conversion32 ( Uint32 v ) {return invert_endianness32(v);}
	static inline Uint64 big_endian_conversion64 ( Uint64 v ) {return invert_endianness64(v);}
	union split_int_16 {
		Uint16 whole;
		struct blah {
			Uint8 low8;
			Uint8 high8;
		} split;
	};
	union split_int_32 {
		Uint32 whole;
		struct blah {
			split_int_16 low16;
			split_int_16 high16;
		} split;
	};
	union split_int_64 {
		Uint64 whole;
		struct blah {
			split_int_32 low32;
			split_int_32 high32;
		} split;
	};
#elif defined PRACTRAND_TARGET_IS_BIG_ENDIAN
	static inline Uint16 big_endian_conversion16 ( Uint16 v ) {return v;}
	static inline Uint32 big_endian_conversion32 ( Uint32 v ) {return v;}
	static inline Uint64 big_endian_conversion64 ( Uint64 v ) {return v;}
	static inline Uint16 little_endian_conversion16 ( Uint16 v ) {return invert_endianness16(v);}
	static inline Uint32 little_endian_conversion32 ( Uint32 v ) {return invert_endianness32(v);}
	static inline Uint64 little_endian_conversion64 ( Uint64 v ) {return invert_endianness64(v);}
	union split_int_16 {
		Uint16 whole;
		struct blah {
			Uint8 high8;
			Uint8 low8;
		} split;
	};
	union split_int_32 {
		Uint32 whole;
		struct blah {
			split_int_16 high16;
			split_int_16 low16;
		} split;
	};
	union split_int_64 {
		Uint64 whole;
		struct blah {
			split_int_32 high32;
			split_int_32 low32;
		} split;
	};
#endif
}

#endif //__PRACTRAND_ENDIAN_H__
