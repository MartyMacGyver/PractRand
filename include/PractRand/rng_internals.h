//#include <vector>

#define PRACTRAND__RANDF_IMPLEMENTATION(RNG)  {return  float(((RNG).raw32() & ((PractRand::Uint32(1) << 24)-1)) *  float(1.0/16777216.0));}
#define PRACTRAND__RANDLF_IMPLEMENTATION(RNG)  {return double(((RNG).raw64() & ((PractRand::Uint64(1) << 53)-1)) * double(1.0/9007199254740992.0));}

#define PRACTRAND__RANDI_IMPLEMENTATION(max)     \
	Uint32 mask, tmp;\
	max -= 1;\
	mask = max;\
	mask |= mask >> 1; mask |= mask >>  2; mask |= mask >> 4;\
	mask |= mask >> 8; mask |= mask >> 16;\
	do {\
		tmp = raw32() & mask;\
	} while (tmp > max);\
	return tmp;
#define PRACTRAND__RANDLI_IMPLEMENTATION(max)     \
	Uint64 mask, tmp;\
	max -= 1;\
	mask = max;\
	mask |= mask >> 1; mask |= mask >>  2; mask |= mask >>  4;\
	mask |= mask >> 8; mask |= mask >> 16; mask |= mask >> 32;\
	do {\
		tmp = raw64() & mask;\
	} while (tmp > max);\
	return tmp;

#define PRACTRAND__POLYMORPHIC_RNG_BASICS_C8(RNG) \
	Uint8  PractRand::RNGs::Polymorphic:: RNG ::raw8 () {return implementation.raw8();}\
	Uint16 PractRand::RNGs::Polymorphic:: RNG ::raw16() {Uint16 r = implementation.raw8(); return (Uint16(implementation.raw8()) << 8) | r;}\
	Uint32 PractRand::RNGs::Polymorphic:: RNG ::raw32() {\
		Uint32 r = implementation.raw8();\
		r = r | (Uint32(implementation.raw8()) << 8);\
		r = r | (Uint32(implementation.raw8()) << 16);\
		return r | (Uint32(implementation.raw8()) << 24);\
	}\
	Uint64 PractRand::RNGs::Polymorphic:: RNG ::raw64() {\
		Uint64 r = raw32();\
		return r | (Uint64(raw32()) << 32);\
	}\
	Uint64 PractRand::RNGs::Polymorphic:: RNG ::get_flags() const {return implementation.FLAGS;}\
	void PractRand::RNGs::Polymorphic:: RNG ::walk_state(StateWalkingObject *walker) {\
		implementation.walk_state(walker);\
	}
#define PRACTRAND__POLYMORPHIC_RNG_BASICS_C16(RNG) \
	Uint8  PractRand::RNGs::Polymorphic:: RNG ::raw8 () {return Uint8(implementation.raw16());}\
	Uint16 PractRand::RNGs::Polymorphic:: RNG ::raw16() {return implementation.raw16();}\
	Uint32 PractRand::RNGs::Polymorphic:: RNG ::raw32() {\
		Uint16 r = implementation.raw16();\
		return r | (Uint32(implementation.raw16()) << 16);\
	}\
	Uint64 PractRand::RNGs::Polymorphic:: RNG ::raw64() {\
		Uint64 r = implementation.raw16();\
		r = r | (Uint32(implementation.raw16()) << 16);\
		r = r | (Uint64(implementation.raw16()) << 32);\
		return r | (Uint64(implementation.raw16()) << 48);\
	}\
	Uint64 PractRand::RNGs::Polymorphic:: RNG ::get_flags() const {return implementation.FLAGS;}\
	void PractRand::RNGs::Polymorphic:: RNG ::walk_state(StateWalkingObject *walker) {\
		implementation.walk_state(walker);\
	}
#define PRACTRAND__POLYMORPHIC_RNG_BASICS_C32(RNG) \
	Uint8  PractRand::RNGs::Polymorphic:: RNG ::raw8 () {return Uint8 (implementation.raw32());}\
	Uint16 PractRand::RNGs::Polymorphic:: RNG ::raw16() {return Uint16(implementation.raw32());}\
	Uint32 PractRand::RNGs::Polymorphic:: RNG ::raw32() {return Uint32(implementation.raw32());}\
	Uint64 PractRand::RNGs::Polymorphic:: RNG ::raw64() {\
		Uint64 r = implementation.raw32();\
		return (r << 32) | implementation.raw32();\
	}\
	Uint64 PractRand::RNGs::Polymorphic:: RNG ::get_flags() const {return implementation.FLAGS;}\
	void PractRand::RNGs::Polymorphic:: RNG ::walk_state(StateWalkingObject *walker) {\
		implementation.walk_state(walker);\
	}
#define PRACTRAND__POLYMORPHIC_RNG_BASICS_C64(RNG) \
	Uint8  PractRand::RNGs::Polymorphic:: RNG ::raw8 () {return Uint8 (implementation.raw64());}\
	Uint16 PractRand::RNGs::Polymorphic:: RNG ::raw16() {return Uint16(implementation.raw64());}\
	Uint32 PractRand::RNGs::Polymorphic:: RNG ::raw32() {return Uint32(implementation.raw64());}\
	Uint64 PractRand::RNGs::Polymorphic:: RNG ::raw64() {return implementation.raw64();}\
	Uint64 PractRand::RNGs::Polymorphic:: RNG ::get_flags() const {return implementation.FLAGS;}\
	void PractRand::RNGs::Polymorphic:: RNG ::walk_state(StateWalkingObject *walker) {\
		implementation.walk_state(walker);\
	}



namespace PractRand {
	namespace RNGs {
		class vRNG;
	}
	void issue_error(const char *msg = 0);
	namespace Internals {
		//inline
		static inline Uint8  rotate8 (Uint8  value, int bits) {return (value << bits) | (value >> (8 -bits));}
		static inline Uint16 rotate16(Uint16 value, int bits) {return (value << bits) | (value >> (16-bits));}
		static inline Uint32 rotate32(Uint32 value, int bits) {return (value << bits) | (value >> (32-bits));}
		static inline Uint64 rotate64(Uint64 value, int bits) {return (value << bits) | (value >> (64-bits));}
		static inline Uint8  rotate(Uint8  value, int bits) {return rotate8 (value, bits);}
		static inline Uint16 rotate(Uint16 value, int bits) {return rotate16(value, bits);}
		static inline Uint32 rotate(Uint32 value, int bits) {return rotate32(value, bits);}
		static inline Uint64 rotate(Uint64 value, int bits) {return rotate64(value, bits);}
		//non_uniform.cpp
		double generate_gaussian_fast(Uint64 raw64);//fast CDF-based hybrid method
		//double generate_gaussian_high_quality(Uint64 raw192[3]);//slow PDF-based ziggurat method
		//rand.cpp
		void test_random_access(PractRand::RNGs::vRNG *rng, PractRand::RNGs::vRNG *known_good, Uint64 period_low64, Uint64 period_high64);
		//platform_specific.cpp
		bool add_entropy_automatically( PractRand::RNGs::vRNG *entropy_pool, int milliseconds=0 );
		Uint64 issue_unique_identifier();
		Uint64 high_resolution_time();
		//math.cpp
		void fast_forward_lcg128 ( Uint64 how_far_low, Uint64 how_far_high, Uint64 &value_low, Uint64 &value_high, Uint64 mul_low, Uint64 mul_high, Uint64 add_low, Uint64 add_high );
		Uint64 fast_forward_lcg64 ( Uint64 how_far, Uint64 val, Uint64 mul, Uint64 add );
		Uint32 fast_forward_lcg32 ( Uint32 how_far, Uint32 val, Uint32 mul, Uint32 add );
		Uint32 fast_forward_lcg32c ( Uint32 how_far, Uint32 val, Uint32 mul, Uint32 add, Uint32 mod );
		/*class XorshiftMatrix {
			//matrix representing a state transition function for an RNG that uses only shifts and xors
			std::vector<bool> bits;
			int size;
		public:
			XorshiftMatrix(int size_, bool identity);
			void apply(const std::vector<bool> &input, std::vector<bool> &output);
			XorshiftMatrix operator*(const XorshiftMatrix &other) const;
			bool operator==(const XorshiftMatrix &other) const;
			bool operator!=(const XorshiftMatrix &other) const {return !(*this == other);}
			XorshiftMatrix exponent(Uint64 exponent_value) const;
			XorshiftMatrix exponent2Xminus1(Uint64 X) const;
			bool verify_period_factorization(const std::vector<Uint64> &factors) const;
			bool get(int in_index, int out_index) const {return bits[in_index + out_index * size];}
			void set(int in_index, int out_index, bool value) {bits[in_index + out_index * size] = value;}
			void toggle(int in_index, int out_index, bool value) {bits[in_index + out_index * size] = !bits[in_index + out_index * size];}
		};*/
	}
}
