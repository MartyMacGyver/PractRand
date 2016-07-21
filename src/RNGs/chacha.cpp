
#include <cstring>
#include <string>
#include <sstream>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"

#include "PractRand/RNGs/chacha.h"

#ifndef PRACTRAND_NO_SIMD
#if defined _MSC_VER && _M_IX86_FP == 2
#include <emmintrin.h>
#endif
#endif

using namespace PractRand;
using namespace PractRand::Internals;

/*
ChaCha matrix structure:
00-03	constant0, constant1, constant2, constant3
04-07	seed0, seed1, seed2, seed3
08-11	seed0/4, seed1/5, seed2/6, seed3/7
12-15	position0, position1, IV0, IV1

top row is constants
2nd row is the first 16 bytes of seed
3rd row is the rest of the seed (for long seeds) or a duplicate of the seed (for short seeds)
4th row is the position & initialization vector
*/
enum { POS_INDEX0 = 12 - 4, POS_INDEX1 = 13 - 4, IV_INDEX0 = 14 - 4, IV_INDEX1 = 15 - 4 };
enum { POSITION_OVERFLOW_INDEX = 0};

//polymorphic:
PRACTRAND__POLYMORPHIC_RNG_BASICS_C32(chacha)
std::string PractRand::RNGs::Polymorphic::chacha::get_name() const {
	std::ostringstream tmp;
	tmp << "chacha(" << implementation.get_rounds() << ")";
	return tmp.str();
}
void PractRand::RNGs::Polymorphic::chacha::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::chacha::seed(Uint32 seed_and_iv[10], bool extend_cycle_) {implementation.seed(seed_and_iv, extend_cycle_);}
void PractRand::RNGs::Polymorphic::chacha::seed_short(Uint32 seed_and_iv[6], bool extend_cycle_) {implementation.seed_short(seed_and_iv, extend_cycle_);}
void PractRand::RNGs::Polymorphic::chacha::seek_forward128 (Uint64 how_far_low64, Uint64 how_far_high64) {implementation.seek_forward (how_far_low64, how_far_high64);}
void PractRand::RNGs::Polymorphic::chacha::seek_backward128(Uint64 how_far_low64, Uint64 how_far_high64) {implementation.seek_backward(how_far_low64, how_far_high64);}
void PractRand::RNGs::Polymorphic::chacha::set_rounds(int rounds_) {implementation.set_rounds(rounds_);}
int PractRand::RNGs::Polymorphic::chacha::get_rounds() const {return implementation.get_rounds();}

//raw:
PractRand::RNGs::Raw::chacha::~chacha() {std::memset(this, 0, sizeof(this));}
#ifdef PRACTRAND_ALIGN_128
static const Uint32 PRACTRAND_ALIGN_128 chacha_long_seed_constants[4] = {
#else
static const Uint32 chacha_long_seed_constants[4] = {
#endif
	//"expand 32-byte k"
	(Uint32('e') << 0) + (Uint32('x') << 8) + (Uint32('p') << 16) + (Uint32('a') << 24),
	(Uint32('n') << 0) + (Uint32('d') << 8) + (Uint32(' ') << 16) + (Uint32('3') << 24),
	(Uint32('2') << 0) + (Uint32('-') << 8) + (Uint32('b') << 16) + (Uint32('y') << 24),
	(Uint32('t') << 0) + (Uint32('e') << 8) + (Uint32(' ') << 16) + (Uint32('k') << 24)
};
#ifdef PRACTRAND_ALIGN_128
static const Uint32 PRACTRAND_ALIGN_128 chacha_short_seed_constants[4] = {
#else
static const Uint32 chacha_short_seed_constants[4] = {
#endif
	//"expand 16-byte k"
	(Uint32('e') << 0) + (Uint32('x') << 8) + (Uint32('p') << 16) + (Uint32('a') << 24),
	(Uint32('n') << 0) + (Uint32('d') << 8) + (Uint32(' ') << 16) + (Uint32('1') << 24),
	(Uint32('6') << 0) + (Uint32('-') << 8) + (Uint32('b') << 16) + (Uint32('y') << 24),
	(Uint32('t') << 0) + (Uint32('e') << 8) + (Uint32(' ') << 16) + (Uint32('k') << 24)
};

void PractRand::RNGs::Raw::chacha::_advance_1() {
	if (!++state[POS_INDEX0]) {
		if (!++state[POS_INDEX1]) position_overflow++;
	}
}
void PractRand::RNGs::Raw::chacha::_set_position(Uint64 low, Uint64 high) {
	used = low & 15;
	low >>= 4;
	low |= high << 60;
	high >>= 4;
	state[POS_INDEX0] = Uint32(low);
	state[POS_INDEX1] = Uint32(low >> 32);
	position_overflow = Uint32(high);
	_core();
}
void PractRand::RNGs::Raw::chacha::_get_position(Uint64 &low, Uint64 &high) const {
	low = used + (Uint64(state[POS_INDEX0]) << 4) + (Uint64(state[POS_INDEX1]) << 36);
	high = (state[POS_INDEX1] >> 28) + (Uint64(position_overflow) << 4);
}
#if !defined PRACTRAND_NO_SIMD && defined _MSC_VER && defined _M_IX86_FP && _M_IX86_FP == 2
void PractRand::RNGs::Raw::chacha::_core() {
	const Uint32 *constants = short_seed ? chacha_short_seed_constants : chacha_long_seed_constants;
	for (int i = 0; i < 4; i++) outbuf[i] = constants[i];
	if (extend_cycle) outbuf[POSITION_OVERFLOW_INDEX] += position_overflow;
	__m128i *buf = (__m128i*)&outbuf[0];
	__m128i *base = (__m128i*)&state[0];
	buf[1] = base[0];
	buf[2] = base[1];
	buf[3] = base[2];
	for (int i = 1; i < rounds; i+=2) {
		//a += b; d ^= a; d <<<= 16;
		buf[0] = _mm_add_epi32(buf[0], buf[1]);
		buf[3] = _mm_xor_si128(buf[3], buf[0]);
		buf[3] = _mm_or_si128(_mm_slli_epi32(buf[3], 16), _mm_srli_epi32(buf[3], 16));
		//c += d; b ^= c; b <<<= 12;
		buf[2] = _mm_add_epi32(buf[2], buf[3]);
		buf[1] = _mm_xor_si128(buf[1], buf[2]);
		buf[1] = _mm_or_si128(_mm_slli_epi32(buf[1], 12), _mm_srli_epi32(buf[1], 20));
		//a += b; d ^= a; d <<<= 8;
		buf[0] = _mm_add_epi32(buf[0], buf[1]);
		buf[3] = _mm_xor_si128(buf[3], buf[0]);
		buf[3] = _mm_or_si128(_mm_slli_epi32(buf[3], 8), _mm_srli_epi32(buf[3], 24));
		//c += d; b ^= c; b <<<= 7;
		buf[2] = _mm_add_epi32(buf[2], buf[3]);
		buf[1] = _mm_xor_si128(buf[1], buf[2]);
		buf[1] = _mm_or_si128(_mm_slli_epi32(buf[1], 7), _mm_srli_epi32(buf[1], 25));
		//pseudo-transpose A:
		buf[1] = _mm_shuffle_epi32(buf[1], 0x39);//2103
		buf[2] = _mm_shuffle_epi32(buf[2], 0x4e);//1032
		buf[3] = _mm_shuffle_epi32(buf[3], 0x93);//0321

		//a += b; d ^= a; d <<<= 16;
		buf[0] = _mm_add_epi32(buf[0], buf[1]);
		buf[3] = _mm_xor_si128(buf[3], buf[0]);
		buf[3] = _mm_or_si128(_mm_slli_epi32(buf[3], 16), _mm_srli_epi32(buf[3], 16));
		//c += d; b ^= c; b <<<= 12;
		buf[2] = _mm_add_epi32(buf[2], buf[3]);
		buf[1] = _mm_xor_si128(buf[1], buf[2]);
		buf[1] = _mm_or_si128(_mm_slli_epi32(buf[1], 12), _mm_srli_epi32(buf[1], 20));
		//a += b; d ^= a; d <<<= 8;
		buf[0] = _mm_add_epi32(buf[0], buf[1]);
		buf[3] = _mm_xor_si128(buf[3], buf[0]);
		buf[3] = _mm_or_si128(_mm_slli_epi32(buf[3], 8), _mm_srli_epi32(buf[3], 24));
		//c += d; b ^= c; b <<<= 7;
		buf[2] = _mm_add_epi32(buf[2], buf[3]);
		buf[1] = _mm_xor_si128(buf[1], buf[2]);
		buf[1] = _mm_or_si128(_mm_slli_epi32(buf[1], 7), _mm_srli_epi32(buf[1], 25));
		//pseudo-transpose B:
		buf[1] = _mm_shuffle_epi32(buf[1], 0x93);//2103
		buf[2] = _mm_shuffle_epi32(buf[2], 0x4e);//1032
		buf[3] = _mm_shuffle_epi32(buf[3], 0x39);//0321
	}
	if (rounds & 1) {
		//a += b; d ^= a; d <<<= 16;
		buf[0] = _mm_add_epi32(buf[0], buf[1]);
		buf[3] = _mm_xor_si128(buf[3], buf[0]);
		buf[3] = _mm_or_si128(_mm_slli_epi32(buf[3], 16), _mm_srli_epi32(buf[3], 16));
		//c += d; b ^= c; b <<<= 12;
		buf[2] = _mm_add_epi32(buf[2], buf[3]);
		buf[1] = _mm_xor_si128(buf[1], buf[2]);
		buf[1] = _mm_or_si128(_mm_slli_epi32(buf[1], 12), _mm_srli_epi32(buf[1], 20));
		//a += b; d ^= a; d <<<= 8;
		buf[0] = _mm_add_epi32(buf[0], buf[1]);
		buf[3] = _mm_xor_si128(buf[3], buf[0]);
		buf[3] = _mm_or_si128(_mm_slli_epi32(buf[3], 8), _mm_srli_epi32(buf[3], 24));
		//c += d; b ^= c; b <<<= 7;
		buf[2] = _mm_add_epi32(buf[2], buf[3]);
		buf[1] = _mm_xor_si128(buf[1], buf[2]);
		buf[1] = _mm_or_si128(_mm_slli_epi32(buf[1], 7), _mm_srli_epi32(buf[1], 25));
		//skip the pseudo-transform
	}
	buf[0] = _mm_add_epi32(buf[0], *(__m128i*)constants);
	buf[1] = _mm_add_epi32(buf[1], base[0]);
	buf[2] = _mm_add_epi32(buf[2], base[1]);
	buf[3] = _mm_add_epi32(buf[3], base[2]);
	if (extend_cycle) outbuf[POSITION_OVERFLOW_INDEX] += position_overflow;//is this safe from the optimizer with all the pointer casts I've done earlier?
}
#else
void PractRand::RNGs::Raw::chacha::_core() {
	const Uint32 *constants = short_seed ? chacha_short_seed_constants : chacha_long_seed_constants;

	for (int i = 0; i < 4; i++) outbuf[i] = constants[i];
	for (int i = 4; i < 16; i++) outbuf[i] = state[i-4];
	if (extend_cycle) outbuf[POSITION_OVERFLOW_INDEX] += position_overflow;
#define BLAH(base,i1,i2,i3,R) base[i1] += base[i2]; base[i3] = rotate32(base[i3] ^ base[i1], R);
#define QUARTERROUND(i1,i2,i3,i4) BLAH(outbuf,i1,i2,i4,16) BLAH(outbuf,i3,i4,i2,12) BLAH(outbuf,i1,i2,i4,8) BLAH(outbuf,i3,i4,i2,7)
	for (int round = 1; round < rounds; round+=2) {
		QUARTERROUND( 0, 4, 8,12)
		QUARTERROUND( 1, 5, 9,13)
		QUARTERROUND( 2, 6,10,14)
		QUARTERROUND( 3, 7,11,15)
		QUARTERROUND( 0, 5,10,15)
		QUARTERROUND( 1, 6,11,12)
		QUARTERROUND( 2, 7, 8,13)
		QUARTERROUND( 3, 4, 9,14)
	}
	if (rounds & 1) {
		QUARTERROUND( 0, 5,10,15)
		QUARTERROUND( 1, 6,11,12)
		QUARTERROUND( 2, 7, 8,13)
		QUARTERROUND( 3, 4, 9,14)
	}
#undef BLAH
#undef QUARTERROUND
	for (int i = 0; i < 4; i++) outbuf[i] += constants[i];
	for (int i = 4; i < 16; i++) outbuf[i] += state[i-4];
	if (extend_cycle) outbuf[POSITION_OVERFLOW_INDEX] += position_overflow;
}
#endif
Uint32 PractRand::RNGs::Raw::chacha::_refill_and_raw32() {
	_advance_1();
	_core();
	used = 1;
	return outbuf[0];
}
void PractRand::RNGs::Raw::chacha::seed(Uint64 s) {
	Uint32 seed_and_iv[10] = {0};
	seed_and_iv[0] = Uint32(s);
	seed_and_iv[1] = Uint32(s >> 32);
	seed(seed_and_iv, true);
}
void PractRand::RNGs::Raw::chacha::seed(const Uint32 seed_and_iv[10], bool extend_cycle_) {
	short_seed = false;
	position_overflow = 0;
	extend_cycle = extend_cycle_;
	for (int i = 0; i < 8; i++) state[i] = seed_and_iv[i];
	state[POS_INDEX0] = 0;
	state[POS_INDEX1] = 0;
	state[IV_INDEX0] = seed_and_iv[8];
	state[IV_INDEX1] = seed_and_iv[9];
	_core();
	used = 0;
}
void PractRand::RNGs::Raw::chacha::seed_short(const Uint32 seed_and_iv[6], bool extend_cycle_) {
	short_seed = true;
	position_overflow = 0;
	extend_cycle = extend_cycle_;
	for (int i = 0; i < 8; i++) state[i] = seed_and_iv[i & 3];
	state[POS_INDEX0] = 0;
	state[POS_INDEX1] = 0;
	state[IV_INDEX0] = seed_and_iv[4];
	state[IV_INDEX1] = seed_and_iv[5];
	_core();
	used = 0;
}
void PractRand::RNGs::Raw::chacha::walk_state(StateWalkingObject *walker) {
	for (int i = 0; i < 12; i++) walker->handle(state[i]);
	walker->handle(used);
	walker->handle(extend_cycle);
	if (extend_cycle) walker->handle(position_overflow);
	walker->handle(short_seed);
	if (walker->is_seeder()) {
		short_seed = false;
		extend_cycle = true;
		position_overflow = 0;
		state[POS_INDEX0] = 0;
		state[POS_INDEX1] = 0;
		state[IV_INDEX0] = 0;
		state[IV_INDEX1] = 0;
	}
	else walker->handle(rounds);


	if (used >= 16) used = 16;
	if (!walker->is_read_only()) {
		_core();
		used &= 15;
	}
}
void PractRand::RNGs::Raw::chacha::seek_forward (Uint64 how_far_low, Uint64 how_far_high) {
	Uint64 pos_low, pos_high;
	_get_position(pos_low, pos_high);
	Uint64 new_pos_low = pos_low + how_far_low;
	if (new_pos_low < pos_low) how_far_high++;
	Uint64 new_pos_high = pos_high + how_far_high;
	_set_position(new_pos_low, new_pos_high);
}
void PractRand::RNGs::Raw::chacha::seek_backward(Uint64 how_far_low, Uint64 how_far_high) {
	seek_forward(~how_far_low, ~how_far_high);
	raw32();
}
void PractRand::RNGs::Raw::chacha::set_rounds(int rounds_) {
	if (rounds_ < 1 || rounds_ > 255) issue_error("chacha rounds out of range");
	if (rounds == rounds_) return;
	rounds = rounds_;
	//_core();
}
static void test_chacha ( Uint32 rounds, const Uint32 *seed_and_iv, bool short_seed, Uint32 expected0, Uint32 index, Uint32 expected1) {
	PractRand::RNGs::Raw::chacha rng;
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
		PractRand::issue_error("ChaCha CSPRNG failed self-test");
	}
}
void PractRand::RNGs::Raw::chacha::self_test() {
	Uint32 seed_and_iv[10] = {0};
	test_chacha(  8, seed_and_iv, false, 0x2fef003e, 16, 0x0dfaaed2);
	test_chacha( 12, seed_and_iv, false, 0x6a9af49b, 16, 0x4188d50b);
	test_chacha( 20, seed_and_iv, false, 0xade0b876, 16, 0xbee7079f);
	seed_and_iv[0] = 0x80;
	test_chacha(  8, seed_and_iv,  true, 0x1ee8b1be, 16, 0x7d720502);
	seed_and_iv[4] = 0x01;
	test_chacha(  8, seed_and_iv,  true, 0x3dc23b69, 16, 0xc9814bee);
	test_chacha(  8, seed_and_iv, false, 0x6bcbf0bb, 16, 0x281b6aa7);
	seed_and_iv[9] = 0x01;
	test_chacha(  8, seed_and_iv, false, 0xcc135949, 16, 0x82f734d3);
	seed_and_iv[8] = 0x12;
	test_chacha(  8, seed_and_iv, false, 0xd5a08851, 3, 0xab4a48a9);
}




