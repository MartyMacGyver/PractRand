#include <string>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"

#include "PractRand/RNGs/xsm32.h"
#include "PractRand/RNGs/xsm64.h"

using namespace PractRand;
using namespace PractRand::Internals;

//polymorphic:
PRACTRAND__POLYMORPHIC_RNG_BASICS_C32(xsm32)
void PractRand::RNGs::Polymorphic::xsm32::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::xsm32::seek_forward128 (Uint64 how_far_low64, Uint64 how_far_high64) {implementation.seek_forward (how_far_low64);}
void PractRand::RNGs::Polymorphic::xsm32::seek_backward128(Uint64 how_far_low64, Uint64 how_far_high64) {implementation.seek_backward(how_far_low64);}
std::string PractRand::RNGs::Polymorphic::xsm32::get_name() const {return "xsm32";}

PRACTRAND__POLYMORPHIC_RNG_BASICS_C64(xsm64)
void PractRand::RNGs::Polymorphic::xsm64::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::xsm64::seek_forward128 (Uint64 how_far_low64, Uint64 how_far_high64) {implementation.seek_forward (how_far_low64, how_far_high64);}
void PractRand::RNGs::Polymorphic::xsm64::seek_backward128(Uint64 how_far_low64, Uint64 how_far_high64) {implementation.seek_backward(how_far_low64, how_far_high64);}
std::string PractRand::RNGs::Polymorphic::xsm64::get_name() const {return "xsm64";}

//raw:

Uint32 PractRand::RNGs::Raw::xsm32::raw32() {
	const Uint32 K = 0x6595a395;

	/*
	// qual: 16 GB @ 16 bit, speed: 1.165 GB/s avg, avalanche: marginal
	Uint32 tmp = lcg_high ^ (lcg_high >> 16);
	tmp *= K;
	tmp += rotate(tmp ^ lcg_low, 11);
	Uint32 old_lcg_low = lcg_low;
	lcg_low += lcg_adder;
	old_lcg_low += ((lcg_low < lcg_adder) ? 1 : 0);
	lcg_high += old_lcg_low;

	Uint32 old = history;
	history = tmp;
	return tmp + rotate(old, 16);//*/

	//*
	// qual: 128 GB @ 16 bit, speed: 1.055 GB/s avg, avalanche: good
	Uint32 rv = history * K;
	Uint32 tmp = lcg_high + rotate(lcg_high ^ lcg_low, 11);
	tmp *= K;
	Uint32 old_lcg_low = lcg_low;
	lcg_low += lcg_adder;
	old_lcg_low += ((lcg_low < lcg_adder) ? 1 : 0);
	lcg_high += old_lcg_low;

	rv ^= rv >> 16;
	rv += history = tmp ^= tmp >> 16;
	return rv;//*/

	/*
	// qual: 256 GB @ 16 bit, speed: 0.903 GB/s avg, avalanche: good
	Uint32 rv = history * K;
	Uint32 tmp = lcg_high ^ rotate(lcg_high + lcg_low, 11);
	tmp *= K;
	Uint32 old_lcg_low = lcg_low;
	lcg_low += lcg_adder;
	old_lcg_low += ((lcg_low < lcg_adder) ? 1 : 0);
	rv = (rv + lcg_high) ^ rotate(rv, 9);
	lcg_high += old_lcg_low;

	rv += history = tmp ^= tmp >> 16;
	return rv;//*/
}
void PractRand::RNGs::Raw::xsm32::step_backwards() {
	bool carry = lcg_low < lcg_adder;
	lcg_low -= lcg_adder;
	lcg_high -= lcg_low + carry;
}
void PractRand::RNGs::Raw::xsm32::seed(Uint64 s) {
	//guaranteed that no two seeds are closer than 2**31 apart on the same cycle
	// - that's the best I can do with 2**64 seeds and 2**95 states.  
	s ^= rotate(s, 21) ^ rotate(s, 39);
	lcg_adder = Uint32(s) | 1;
	lcg_high = Uint32(s >> 32);
	lcg_low = Uint32(s << 31) + lcg_adder;
	history = 0;
	raw32();
}
void PractRand::RNGs::Raw::xsm32::walk_state(StateWalkingObject *walker) {
	walker->handle(lcg_low);
	walker->handle(lcg_high);
	walker->handle(lcg_adder);
	walker->handle(history);
	if (walker->is_clumsy() && !walker->is_read_only()) {
		lcg_adder |= 1;
		step_backwards();
		raw32();
	}
}
void PractRand::RNGs::Raw::xsm32::seek_forward (Uint64 how_far) {
	if (!how_far) return;
	how_far -= 1;
	Uint64 lcg_state = lcg_low | (Uint64(lcg_high) << 32);
	lcg_state = fast_forward_lcg64(how_far, lcg_state, 0x0000000100000001ull, lcg_adder);
	lcg_low = Uint32(lcg_state);
	lcg_high = Uint32(lcg_state >> 32);
	raw32();
}
void PractRand::RNGs::Raw::xsm32::seek_backward(Uint64 how_far) {
	seek_forward(0 - how_far);
}


Uint64 PractRand::RNGs::Raw::xsm64::raw64() {
	const Uint64 K = 0xA3EC647659359ACDull;
	history *= K;
	Uint64 tmp = lcg_high + rotate(lcg_high ^ lcg_low, 19);
	tmp *= K;

	Uint64 old = lcg_low;
	lcg_low += lcg_adder;
	lcg_high += old + ((lcg_low < lcg_adder) ? 1 : 0);

	old = history ^ (history >> 32);
	history = tmp ^= tmp >> 32;
	return tmp + old;
}
void PractRand::RNGs::Raw::xsm64::step_backwards() {
	bool carry = lcg_low < lcg_adder;
	lcg_low -= lcg_adder;
	lcg_high -= lcg_low + carry;
}
void PractRand::RNGs::Raw::xsm64::seed(Uint64 s) {
	s ^= rotate64(s, 17) ^ rotate64(s, 29);
	//guarantees that no two seeds are within 2**127 of each other on the same cycle
	lcg_high = s << 63;
	lcg_adder = s | 1;
	lcg_low = lcg_adder;
	history = 0;
	raw64();
}
void PractRand::RNGs::Raw::xsm64::walk_state(StateWalkingObject *walker) {
	walker->handle(lcg_low);
	walker->handle(lcg_high);
	walker->handle(lcg_adder);
	walker->handle(history);
	if (walker->is_clumsy() && !walker->is_read_only()) {
		lcg_adder |= 1;
		step_backwards();
		raw64();
	}
}
void PractRand::RNGs::Raw::xsm64::seek_forward (Uint64 how_far_low, Uint64 how_far_high) {
	if (!how_far_low && !how_far_high) return;
	if (!how_far_low--) how_far_high--;
	fast_forward_lcg128(how_far_low, how_far_high, 
		lcg_low, lcg_high, 1, 1, lcg_adder, 0
	);
	raw64();
}
void PractRand::RNGs::Raw::xsm64::seek_backward(Uint64 how_far_low, Uint64 how_far_high) {
	if (!how_far_low && !how_far_high) return;
	how_far_low = 1 + ~how_far_low;
	how_far_high = ~how_far_high + (how_far_low ? 0 : 1);
	seek_forward(how_far_low, how_far_high);
}
