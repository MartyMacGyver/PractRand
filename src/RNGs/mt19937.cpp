#include <string>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"

#include "PractRand/RNGs/mt19937.h"
#include "PractRand/RNGs/jsf32.h"

using namespace PractRand;

//polymorphic:
PRACTRAND__POLYMORPHIC_RNG_BASICS_C32(mt19937)
void PractRand::RNGs::Polymorphic::mt19937::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::mt19937::seed(Uint32 s[], int seed_length) {implementation.seed(s, seed_length);}
void PractRand::RNGs::Polymorphic::mt19937::flush_buffers() {implementation.flush_buffers();}
std::string PractRand::RNGs::Polymorphic::mt19937::get_name() const {return "mt19937";}

//raw:
static inline unsigned long twist32( unsigned long m, unsigned long s0, unsigned long s1 ) {
	static const unsigned long gfsr_twist_table[2] = {0, 0x9908b0dful};
	return m ^ gfsr_twist_table[s1&1] ^ (((s0&0x80000000ul)|(s1&0x7ffffffful))>>1);
}
void PractRand::RNGs::Raw::mt19937::_advance_state() {//LOCKED, do not change
	Uint32 *p = state;
	long i;
	for( i = ARRAY_SIZE - OFFSET; i--; ++p )
		*p = Uint32(twist32( p[OFFSET], p[0], p[1] ));
	for( i = OFFSET; --i; ++p )
		*p = Uint32(twist32( *(p - (ARRAY_SIZE-OFFSET)), p[0], p[1] ));
	*p = Uint32(twist32( *(p - (ARRAY_SIZE-OFFSET)), p[0], state[0] ));

	used = 0;
}
Uint32 PractRand::RNGs::Raw::mt19937::raw32() {//LOCKED, do not change
	Uint32 r = untempered_raw32();
	r ^= (r >> 11);
	r ^= (r <<  7) & 0x9d2c5680u;
	r ^= (r << 15) & 0xefc60000u;
	return r ^ (r >> 18);
}
void PractRand::RNGs::Raw::mt19937::seed(Uint64 s) {
	//LOCKED, do not change
	//exception: revised behavior of seeds >= 2**32 in version 0.87
	if (s < (Uint64(1) << 32)) {
		state[0] = Uint32(s);
		for (long i=1; i < ARRAY_SIZE; i++) {
			state[i] = 1812433253UL * (state[i-1] ^ (state[i-1] >> 30)) + i;
		}
		used = ARRAY_SIZE;
	}
	else {
		Uint32 seed_array[2];
		seed_array[0] = Uint32(s >> 0);
		seed_array[1] = Uint32(s >> 32);
		seed(seed_array, 2);
	}
}
void PractRand::RNGs::Raw::mt19937::seed(Uint32 s[], int seed_length) {//LOCKED, do not change
	int i, j, k;
	seed(19650218UL);
	i=1; j=0;
	k = (ARRAY_SIZE > seed_length) ? ARRAY_SIZE : seed_length;
	for (; k; k--) {
		state[i] = (state[i] ^ ((state[i-1] ^ (state[i-1] >> 30)) * 1664525UL)) + s[j] + j;
		i++; j++;
		if (i >= ARRAY_SIZE) {
			state[0] = state[ARRAY_SIZE-1];
			i=1;
		}
		if (j >= seed_length) j=0;
	}
	for (k=ARRAY_SIZE-1; k; k--) {
		state[i] = (state[i] ^ ((state[i-1] ^ (state[i-1] >> 30)) * 1566083941UL)) - i;
		i++;
		if (i>=ARRAY_SIZE) { state[0] = state[ARRAY_SIZE-1]; i=1; }
	}
	state[0] = 0x80000000UL;
}
void PractRand::RNGs::Raw::mt19937::walk_state(StateWalkingObject *walker) {
	//LOCKED, do not change
	//exception - in version 0.85 added check for invalid state
	walker->handle(used);
	for (unsigned long i = 0; i < ARRAY_SIZE; i++) walker->handle(state[i]);
	if (used > ARRAY_SIZE) used = ARRAY_SIZE;
	if (walker->is_clumsy()) {
		unsigned long successive_zeroes;
		for (successive_zeroes = 0; successive_zeroes < ARRAY_SIZE; successive_zeroes++)
			if (state[successive_zeroes]) break;
		if (successive_zeroes == ARRAY_SIZE) state[0] = 1;
	}
}
void PractRand::RNGs::Raw::mt19937::self_test() {
	const Uint64 expected = 0x7d9883055dc1141ull;
	Raw::mt19937 rng; rng.seed(1371941);
	Uint64 checksum = 0;
	for (int i = 0; i < 8192; i++) {
		checksum ^= checksum << 24;
		checksum ^= checksum >> 27;
		checksum += rng.raw32();
	}
	if (checksum != expected) issue_error("mt19937::self_test() failed");

	const Uint64 expected2 = 0x2ec23c02564d6339ull;
    Uint32 init[4]={0x123, 0x234, 0x345, 0x456}, length=4;
	rng.seed(init, length);
	Uint64 checksum2 = 0;
	for (int i = 0; i < 8192; i++) {
		checksum2 ^= checksum2 << 24;
		checksum2 ^= checksum2 >> 27;
		checksum2 += rng.raw32();
	}
	if (checksum2 != expected2) issue_error("mt19937::self_test() failed array-based seeding\n");
}
