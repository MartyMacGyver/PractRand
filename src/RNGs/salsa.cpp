
#include <cstring>
#include <string>
#include <sstream>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"
#include "PractRand/endian.h"

#include "PractRand/RNGs/salsa.h"

using namespace PractRand;
using namespace PractRand::Internals;

/*
Salsa matrix structure:
00-03	constant0, seed0, seed1, seed2
04-07	seed3, constant1, position0, position1
08-11	IV0, IV1, constant2, seed0/4
12-15	seed1/5, seed2/6, seed3/7, constant3

The constants go on the diagonal, everything else fits its way in around that, 
with the position & IV going between the first half of the seed and the second half of the seed.  
If the seed is short then the 2nd half is treated as equal to the first half.  
*/
enum { POS_INDEX0 = 8, POS_INDEX1 = 9, IV_INDEX0 = 6, IV_INDEX1 = 7 };
enum { SEED_INDEX_A = 1, SEED_INDEX_B = 11 };
enum { CONST_INDEX_0 = 0, CONST_INDEX_1 = 5, CONST_INDEX_2 = 10, CONST_INDEX_3 = 15 };
enum { POSITION_OVERFLOW_INDEX = CONST_INDEX_0};

//polymorphic:
PRACTRAND__POLYMORPHIC_RNG_BASICS_C32(salsa)
std::string PractRand::RNGs::Polymorphic::salsa::get_name() const {
	std::ostringstream tmp;
	tmp << "salsa(" << implementation.get_rounds() << ")";
	return tmp.str();
}
void PractRand::RNGs::Polymorphic::salsa::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::salsa::seed(Uint32 seed_and_iv[10], bool extend_cycle_) {implementation.seed(seed_and_iv, extend_cycle_);}
void PractRand::RNGs::Polymorphic::salsa::seed_short(Uint32 seed_and_iv[6], bool extend_cycle_) {implementation.seed(seed_and_iv, extend_cycle_);}
void PractRand::RNGs::Polymorphic::salsa::seek_forward128 (Uint64 how_far_low64, Uint64 how_far_high64) {implementation.seek_forward (how_far_low64, how_far_high64);}
void PractRand::RNGs::Polymorphic::salsa::seek_backward128(Uint64 how_far_low64, Uint64 how_far_high64) {implementation.seek_backward(how_far_low64, how_far_high64);}
void PractRand::RNGs::Polymorphic::salsa::set_rounds(int rounds_) {implementation.set_rounds(rounds_);}
int PractRand::RNGs::Polymorphic::salsa::get_rounds() const {return implementation.get_rounds();}


//raw:
PractRand::RNGs::Raw::salsa::~salsa() {std::memset(this, 0, sizeof(this));}
static void salsa_mix_core(Uint32 &a, Uint32 &b, Uint32 &c, Uint32 &d) {
	b ^= rotate32(a + d, 7);
	c ^= rotate32(b + a, 9);
	d ^= rotate32(c + b, 13);
	a ^= rotate32(d + c, 18);
}
//	const Uint32 *constants = short_seed ? salsa_short_seed_constants : salsa_long_seed_constants;
void PractRand::RNGs::Raw::salsa::_core() {
	for (int i = 0; i < 16; i++) outbuf[i] = state[i];
	if (extend_cycle) outbuf[POSITION_OVERFLOW_INDEX] += position_overflow;
#define QUARTERROUND(i1,i2,i3,i4) salsa_mix_core( outbuf[i1], outbuf[i2], outbuf[i3], outbuf[i4] );
	for (int round = 1; round < rounds; round+=2) {
		QUARTERROUND( 0, 4, 8,12)//columns
		QUARTERROUND( 5, 9,13, 1)
		QUARTERROUND(10,14, 2, 6)
		QUARTERROUND(15, 3, 7,11)
		QUARTERROUND( 0, 1, 2, 3)//rows
		QUARTERROUND( 5, 6, 7, 4)
		QUARTERROUND(10,11, 8, 9)
		QUARTERROUND(15,12,13,14)
	}
	if (rounds & 1) {
		QUARTERROUND( 0, 4, 8,12)//columns
		QUARTERROUND( 5, 9,13, 1)
		QUARTERROUND(10,14, 2, 6)
		QUARTERROUND(15, 3, 7,11)
	}
#undef QUARTERROUND
	for (int i = 0; i < 16; i++) outbuf[i] += state[i];
	if (extend_cycle) outbuf[POSITION_OVERFLOW_INDEX] += position_overflow;
}
Uint32 PractRand::RNGs::Raw::salsa::_refill_and_raw32() {
	_advance_1();
	_core();
	used = 1;
	return outbuf[0];
}
void PractRand::RNGs::Raw::salsa::_advance_1() {
	if (!++state[POS_INDEX0]) {
		if (!++state[POS_INDEX1]) position_overflow++;
	}
}
//void PractRand::RNGs::Raw::salsa::_reverse_1();
void PractRand::RNGs::Raw::salsa::_set_position(Uint64 low, Uint64 high) {
	used = low & 15;
	low >>= 4;
	low |= high << 60;
	high >>= 4;
	state[POS_INDEX0] = Uint32(low);
	state[POS_INDEX1] = Uint32(low >> 32);
	position_overflow = Uint32(high);
	_core();
}
void PractRand::RNGs::Raw::salsa::_get_position(Uint64 &low, Uint64 &high) const {
	low = used + (Uint64(state[POS_INDEX0]) << 4) + (Uint64(state[POS_INDEX1]) << 36);
	high = (state[POS_INDEX1] >> 28) + (Uint64(position_overflow) << 4);
}
void PractRand::RNGs::Raw::salsa::seed(Uint64 s) {
	Uint32 seed_and_iv[10] = {0};
	seed_and_iv[0] = Uint32(s);
	seed_and_iv[1] = Uint32(s >> 32);
	seed(seed_and_iv, true);
}
const Uint32 salsa_short_seed_constants[4] = {
	(Uint32(101) << 0) + (Uint32(120) << 8) + (Uint32(112) << 16) + (Uint32( 97) << 24),
	(Uint32(110) << 0) + (Uint32(100) << 8) + (Uint32( 32) << 16) + (Uint32( 49) << 24),
	(Uint32( 54) << 0) + (Uint32( 45) << 8) + (Uint32( 98) << 16) + (Uint32(121) << 24),
	(Uint32(116) << 0) + (Uint32(101) << 8) + (Uint32( 32) << 16) + (Uint32(107) << 24),
};
const Uint32 salsa_long_seed_constants[4] = {
	(Uint32(101) << 0) + (Uint32(120) << 8) + (Uint32(112) << 16) + (Uint32( 97) << 24),
	(Uint32(110) << 0) + (Uint32(100) << 8) + (Uint32( 32) << 16) + (Uint32( 51) << 24),
	(Uint32( 50) << 0) + (Uint32( 45) << 8) + (Uint32( 98) << 16) + (Uint32(121) << 24),
	(Uint32(116) << 0) + (Uint32(101) << 8) + (Uint32( 32) << 16) + (Uint32(107) << 24),
};
void PractRand::RNGs::Raw::salsa::seed(const Uint32 seed_and_iv[10], bool extend_cycle_) {
	const Uint32 *constants = salsa_long_seed_constants;
	state[CONST_INDEX_0] = constants[0];
	state[CONST_INDEX_1] = constants[1];
	state[CONST_INDEX_2] = constants[2];
	state[CONST_INDEX_3] = constants[3];
	position_overflow = 0;
	extend_cycle = extend_cycle_;
	for (int i = 0; i < 4; i++) state[SEED_INDEX_A + i] = seed_and_iv[i];
	for (int i = 0; i < 4; i++) state[SEED_INDEX_B + i] = seed_and_iv[i+4];
	state[POS_INDEX0] = 0;
	state[POS_INDEX1] = 0;
	state[IV_INDEX0] = seed_and_iv[8];
	state[IV_INDEX1] = seed_and_iv[9];
	_core();
	used = 0;
}
void PractRand::RNGs::Raw::salsa::seed_short(const Uint32 seed_and_iv[6], bool extend_cycle_) {
	const Uint32 *constants = salsa_short_seed_constants;
	state[CONST_INDEX_0] = constants[0];
	state[CONST_INDEX_1] = constants[1];
	state[CONST_INDEX_2] = constants[2];
	state[CONST_INDEX_3] = constants[3];
	position_overflow = 0;
	extend_cycle = extend_cycle_;
	for (int i = 0; i < 4; i++) state[SEED_INDEX_A + i] = seed_and_iv[i];
	for (int i = 0; i < 4; i++) state[SEED_INDEX_B + i] = seed_and_iv[i];
	state[POS_INDEX0] = 0;
	state[POS_INDEX1] = 0;
	state[IV_INDEX0] = seed_and_iv[4];
	state[IV_INDEX1] = seed_and_iv[5];
	_core();
	used = 0;
}
void PractRand::RNGs::Raw::salsa::walk_state(StateWalkingObject *walker) {
	for (int i = 0; i < 16; i++) walker->handle(state[i]);
	walker->handle(used);
	walker->handle(extend_cycle);
	if (extend_cycle) walker->handle(position_overflow);
	if (walker->is_seeder()) {
		const Uint32 *constants = salsa_long_seed_constants;
		state[CONST_INDEX_0] = constants[0];
		state[CONST_INDEX_1] = constants[1];
		state[CONST_INDEX_2] = constants[2];
		state[CONST_INDEX_3] = constants[3];
		state[POS_INDEX0] = 0;
		state[POS_INDEX1] = 0;
		state[IV_INDEX0] = 0;
		state[IV_INDEX1] = 0;
		extend_cycle = true;
		position_overflow = 0;
	}
	else walker->handle(rounds);

	if (used >= 16) used = 16;
	if (!walker->is_read_only()) {
		_core();
		used &= 15;
	}
}
void PractRand::RNGs::Raw::salsa::seek_forward (Uint64 how_far_low, Uint64 how_far_high) {
	Uint64 pos_low, pos_high;
	_get_position(pos_low, pos_high);
	Uint64 new_pos_low = pos_low + how_far_low;
	if (new_pos_low < pos_low) how_far_high++;
	Uint64 new_pos_high = pos_high + how_far_high;
	_set_position(new_pos_low, new_pos_high);
}
void PractRand::RNGs::Raw::salsa::seek_backward(Uint64 how_far_low, Uint64 how_far_high) {
	seek_forward(~how_far_low, ~how_far_high);
	raw32();
}
void PractRand::RNGs::Raw::salsa::set_rounds(int rounds_) {
	if (rounds_ < 1 || rounds_ > 255) issue_error("salsa rounds out of range");
	if (rounds == rounds_) return;
	rounds = rounds_;
	//_core();
}
static void test_salsa ( Uint32 rounds, const Uint32 *seed_and_iv, bool short_seed, Uint32 expected0, Uint32 index, Uint32 expected1) {
	PractRand::RNGs::Raw::salsa rng;
	rng.set_rounds(rounds);
	if (!short_seed) rng.seed(seed_and_iv, false);
	else rng.seed_short(seed_and_iv, false);
	Uint32 observed0 = rng.raw32();
	Uint32 observed1;
	if (!index) observed1 = observed0;
	else {
		for (Uint32 i = 1; i < index; i++) rng.raw32();
		observed1 = rng.raw32();
	}

	if (expected0 != observed0 || expected1 != observed1) {
		PractRand::issue_error("Salsa CSPRNG failed self-test");
	}
}
void PractRand::RNGs::Raw::salsa::self_test() {
	PractRand::RNGs::Raw::salsa engine;
	Uint32 seed_and_iv[10] = {0};
	for (int i = 0; i < 4; i++) seed_and_iv[i+0] = i * 0x04040404 + 0x04030201;
	for (int i = 0; i < 4; i++) seed_and_iv[i+4] = i * 0x04040404 + 0xCCCBCAC9;
	for (int i = 0; i < 2; i++) seed_and_iv[i+8] = i * 0x04040404 + 0x68676665;
	engine.set_rounds(20);
	engine.seed(seed_and_iv, false);
	Uint64 N = (Uint64(109)<<0)+(Uint64(110)<<8)+(Uint64(111)<<16)+(Uint64(112)<<24)+(Uint64(113)<<32)+(Uint64(114)<<40)+(Uint64(115)<<48)+(Uint64(116)<<56);
	engine.seek_forward( N << 4, N >> 60);
	Uint64 E = (Uint64(69)<<0)+(Uint64(37)<<8)+(Uint64(68)<<16)+(Uint64(39)<<24)+(Uint64(41)<<32)+(Uint64(15)<<40)+(Uint64(107)<<48)+(Uint64(193)<<56);
	if (Uint32(E) != engine.raw32()) issue_error("salsa::self_test() failed\n");
	if (Uint32(E>>32) != engine.raw32()) issue_error("salsa::self_test() failed\n");

	engine.set_rounds(20);
	for (int i = 0; i < 6; i++) seed_and_iv[i] = 0;
	engine.seed_short(seed_and_iv, false);
	Uint64 E2 = 0x4c12ebcfaead1365ull;
	if (Uint32(E2) != engine.raw32()) issue_error("salsa::self_test() failed a\n");
	if (Uint32(E2>>32) != engine.raw32()) issue_error("salsa::self_test() failed b\n");
}


