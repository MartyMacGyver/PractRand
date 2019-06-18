#include <string>
#include <sstream>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"

#include "PractRand/RNGs/other/simple.h"

namespace PractRand {
	using namespace Internals;
	namespace RNGs {
		namespace Polymorphic {
			namespace NotRecommended {
				Uint16 xsalta16x3::raw16() {//slightly more complex output function
					Uint16 tmp, old;
					tmp = a + c + ((b >> 11) | (b << 5));
					old = a;
					a = b ^ (b >> 3);
					b = c ^ (c << 7);
					c = c ^ (c >> 8) ^ old;
					return tmp;
				}
				std::string xsalta16x3::get_name() const { return "xsalta16x3"; }
				void xsalta16x3::walk_state(StateWalkingObject *walker) {
					walker->handle(a); walker->handle(b); walker->handle(c);
					if (!a && !b && !c) a = 1;
				}
				Uint16 xsaltb16x3::raw16() {//radically different output function
					Uint16 tmp, old;
					tmp = (a & b) | (b & c) | (c & a);
					old = a;
					a = b ^ (b >> 3);
					b = c ^ (c << 7);
					c = c ^ (c >> 8) ^ old;
					return c + ((tmp << 5) | (tmp >> 11));
				}
				std::string xsaltb16x3::get_name() const { return "xsaltb16x3"; }
				void xsaltb16x3::walk_state(StateWalkingObject *walker) {
					walker->handle(a); walker->handle(b); walker->handle(c);
					if (!a && !b && !c) a = 1;
				}
				Uint16 xsaltc16x3::raw16() {//deviating from the standard LFSR state function
					Uint16 old;
					old = a;
					a = b ^ (b >> 5);
					b = c + (c << 3);
					c = c ^ (c >> 7) ^ old;
					return a + b;
				}
				std::string xsaltc16x3::get_name() const { return "xsaltc16x3"; }
				void xsaltc16x3::walk_state(StateWalkingObject *walker) {
					walker->handle(a); walker->handle(b); walker->handle(c);
					if (!a && !b && !c) a = 1;
				}

				Uint32 xorshift32::raw32() {
					a ^= a << 13;
					a ^= a >> 17;
					a ^= a << 5;
					return a;
				}
				std::string xorshift32::get_name() const { return "xorshift32"; }
				void xorshift32::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					if (!a) a = 1;
				}
				Uint64 xorshift64::raw64() {
					a ^= a << 13;
					a ^= a >> 7;
					a ^= a << 17;
					return a;
				}
				std::string xorshift64::get_name() const { return "xorshift64"; }
				void xorshift64::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					if (!a) a = 1;
				}
				void xorshift64of128::xrs(int bits) {
					if (bits < 64) {
						low ^= low >> bits;
						low ^= high << (64 - bits);
						high ^= high >> bits;
					}
					else if (bits > 64) low ^= high >> (bits - 64);
					else low ^= high;
				}
				void xorshift64of128::xls(int bits) {
					if (bits < 64) {
						high ^= high << bits;
						high ^= low >> (64 - bits);
						low ^= low << bits;
					}
					else if (bits > 64) low ^= high >> (bits - 64);
					else low ^= high;
				}
				Uint64 xorshift64of128::raw64() {
					xls(16);
					xrs(53);
					xls(47);
					return low;
				}
				std::string xorshift64of128::get_name() const { return "xorshift64of128"; }
				void xorshift64of128::walk_state(StateWalkingObject *walker) {
					walker->handle(low); walker->handle(high);
					if (!high && !low) low = 1;
				}

				std::string xorshift32of128::get_name() const { return "xorshift32of128"; }
				void xorshift32of128::walk_state(StateWalkingObject *walker) { impl.walk_state(walker); }
				std::string xorshift32of64::get_name() const { return "xorshift32of64"; }
				void xorshift32of64::walk_state(StateWalkingObject *walker) { impl.walk_state(walker); }
				std::string xorshift16of32::get_name() const { return "xorshift16of32"; }
				void xorshift16of32::walk_state(StateWalkingObject *walker) { impl.walk_state(walker); }

				Uint32 xorshift32x4::raw32() {
					Uint32 tmp = x ^ (x << 15);
					x = y;
					y = z;
					z = w;
					tmp ^= tmp >> 4;
					w ^= (w >> 21) ^ tmp;
					return w;
				}
				std::string xorshift32x4::get_name() const { return "xorshift32x4"; }
				void xorshift32x4::walk_state(StateWalkingObject *walker) {
					walker->handle(x);
					walker->handle(y);
					walker->handle(z);
					walker->handle(w);
					if (!(x || y || z || w)) x = 1;
				}

				Uint32 xorwow32of96::raw32() {
					a += 362437;
					return a + impl.raw32();
				}
				std::string xorwow32of96::get_name() const { return "xorwow32of96"; }
				void xorwow32of96::walk_state(StateWalkingObject *walker) {
					impl.walk_state(walker);
					walker->handle(a);
				}
				Uint32 xorwow32x6::raw32() {
					Uint32 tmp = x;
					x = y;
					y = z;
					z = w ^ (w << 1);
					w = v ^ (v >> 7);
					v ^= (v << 4) ^ tmp;
					d += 362437;
					return v + d;
				}
				std::string xorwow32x6::get_name() const { return "xorwow32x6"; }
				void xorwow32x6::walk_state(StateWalkingObject *walker) {
					walker->handle(x);
					walker->handle(y);
					walker->handle(z);
					walker->handle(w);
					walker->handle(v);
					walker->handle(d);
					if (!(x || y || z || w || v)) x = 1;
				}
				Uint64 xoroshiro128plus::raw64() {
					Uint64 result = state0 + state1;
					Uint64 tmp = state0 ^ state1;
					state0 = ((state0 << 55) | (state0 >> (64 - 55))) ^ tmp ^ (tmp << 14);
					state1 = ((tmp << 36) | (tmp >> (64 - 36)));
					return result;
				}
				std::string xoroshiro128plus::get_name() const { return "xoroshiro128plus"; }
				void xoroshiro128plus::walk_state(StateWalkingObject *walker) {
					walker->handle(state0);
					walker->handle(state1);
				}
				Uint64 xoroshiro128plus_2p64::raw64() {
					Uint64 result = state0 + state1;
					static const Uint64 JUMP[] = { 0xbeac0467eba5facb, 0xd86b048b86aa9922 };

					Uint64 s0 = 0;
					Uint64 s1 = 0;
					for (int i = 0; i < sizeof JUMP / sizeof *JUMP; i++) {
						for (int b = 0; b < 64; b++) {
							if (JUMP[i] & 1ULL << b) {
								s0 ^= state0;
								s1 ^= state1;
							}
							Uint64 tmp = state0 ^ state1;
							state0 = ((state0 << 55) | (state0 >> (64 - 55))) ^ tmp ^ (tmp << 14);
							state1 = ((tmp << 36) | (tmp >> (64 - 36)));
						}
					}
					state0 = s0;
					state1 = s1;
					return result;
				}
				std::string xoroshiro128plus_2p64::get_name() const { return "xoroshiro128plus_2p64"; }
				void xoroshiro128plus_2p64::walk_state(StateWalkingObject *walker) {
					walker->handle(state0);
					walker->handle(state1);
				}


				Uint32 sapparot::raw32() {
					Uint32 tmp;
					tmp = a + 0x9e3779b9;
					tmp = (tmp << 7) | (tmp >> 25);
					a = b ^ (~tmp) ^ (tmp << 3);
					a = (a << 7) | (a >> 25);
					b = tmp;
					return a ^ b;
				}
				std::string sapparot::get_name() const { return "sapparot"; }
				void sapparot::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
				}

				Uint16 sap16of48::raw16() {
					Uint16 tmp;
					tmp = a + 0x79b9 + c;
					tmp = (tmp << 5) | (tmp >> 11);
					a = b ^ (~tmp) ^ (tmp << 3);
					a = (a << 5) | (a >> 11);
					b = tmp;
					c = (c + a) ^ b;
					return b;
				}
				std::string sap16of48::get_name() const { return "sap16of48"; }
				void sap16of48::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint32 sap32of96::raw32() {
					Uint32 tmp;
					tmp = a + 0x9e3779b9 + c;
					tmp = (tmp << 7) | (tmp >> 25);
					a = b ^ (~tmp) ^ (tmp << 3);
					a = (a << 7) | (a >> 25);
					b = tmp;
					c = (c + a) ^ b;
					return b;
				}
				std::string sap32of96::get_name() const { return "sap32of96"; }
				void sap32of96::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}

				Uint32 flea32x1::raw32() {
					enum { SHIFT1 = 15, SHIFT2 = 27 };
					Uint32 e = a[d % SIZE];
					a[d % SIZE] = ((b << SHIFT1) | (b >> (32 - SHIFT1)));
					b = c + ((d << SHIFT2) | (d >> (32 - SHIFT2)));
					c = d + a[i++ % SIZE];
					d = e + c;
					return b;
				}
				std::string flea32x1::get_name() const { return "flea32x1"; }
				void flea32x1::walk_state(StateWalkingObject *walker) {
					for (int z = 0; z < SIZE; z++) walker->handle(a[z]);
					walker->handle(b);
					walker->handle(c);
					walker->handle(d);
					walker->handle(i);
				}

				/*
				sfc version 1:
				tmp = a ^ counter++;
				a = b + (b << SHIFT1);
				b = ((b << SHIFT2) | (b >> (WORD_BITS - SHIFT2))) + tmp;
				steps:
				load a			load b			load counter
				*	b<<SHIFT1		b<<<SHIFT2		counter + 1		a ^ counter
				+ b				+ a ^ counter	store counter
				store a			store b
				overall:
				quality suffers, but it's fast
				note that (b + (b << SHIFT1)) can be a single LEA on x86 for small shifts
				constants:
				16: 2,5 ?
				32: 5,12 ?
				64: 7,41 ?
				sfc version 2:
				code:
				tmp = a ^ b;
				a = b + (b << SHIFT1);
				b = ((b << SHIFT2) | (b >> (WORD_BITS - SHIFT2))) + tmp + counter++;
				steps:
				load a			load b				load counter
				*	b << SHIFT1		b <<< SHIFT2		a ^ b			counter + 1
				+ b				+((a^b)+counter)***
				store a			store b
				suggested values for SHIFT1,SHIFT2 by output avalanche:
				16 bit: *2,5 - 2,6 - 3,5 - 3,6
				32 bit: 4,9 - 5,9 - *5,12 - 6,10 - 9,13
				64 bit: 6,40 - 7,17 - *7,41 - 10,24
				overall:
				does okay @ 32 & 64 bit, but poorly at 16 bit
				note that (b + (b << SHIFT1)) can be a single LEA on x86 for small shifts
				sfc version 3:
				code
				tmp = a + b + counter++;
				a = b ^ (b >> SHIFT1);
				b = ((b << SHIFT2) | (b >> (WORD_BITS - SHIFT2))) + tmp;
				steps:
				load a			load b			load counter
				b >> SHIFT1		b <<< SHIFT2	a+b+counter***		counter + 1
				*	^ b				+(a+b+counter)	store counter
				store a			store b
				overall:
				good statistical properties
				slower than earlier versions on my CPU, but still fast
				refence values for SHIFT1,SHIFT2:
				16 bit: 2,5
				32 bit: 5,12
				64 bit: 7,41
				sfc version 4:
				code
				Word old = a + b + counter++;
				a = b ^ (b >> SHIFT2);
				b = c + (c << SHIFT3);
				c = old + rotate(c,SHIFT1);
				return old;
				steps:
				load a			load b			load c				load counter
				*	b >> SHIFT2		b << SHIFT3		c <<< SHIFT1		counter + 1		a+b+counter***
				^ b				+ b				+(a+b+counter)		store counter
				store a			store b			store c
				overall:
				very good behavior on statistical tests
				uses an extra word / register - not as nice for inlining
				adequate speed
				refence values for SHIFT1,SHIFT2,SHIFT3:
				8 bit:  3,2,1
				16 bit: 7,5,2
				32 bit: 25,8,3
				64 bit: 25,12,3
				*/
				Uint16 sfc_v1_16::raw16() {
					Uint16 tmp = a ^ counter++;
					a = b + (b << 2);
					enum { BARREL_SHIFT = 5 };
					b = ((b << BARREL_SHIFT) | (b >> (16 - BARREL_SHIFT))) + tmp;
					return a;
				}
				std::string sfc_v1_16::get_name() const { return "sfc_v1_16"; }
				void sfc_v1_16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(counter);
				}
				Uint32 sfc_v1_32::raw32() {
					Uint32 tmp = a ^ counter++;
					a = b + (b << 5);
					enum { BARREL_SHIFT = 12 };
					b = ((b << BARREL_SHIFT) | (b >> (32 - BARREL_SHIFT))) + tmp;
					return a;
				}
				std::string sfc_v1_32::get_name() const { return "sfc_v1_32"; }
				void sfc_v1_32::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(counter);
				}
				Uint16 sfc_v2_16::raw16() {
					Uint16 tmp = a ^ b;
					a = b + (b << 2);
					enum { BARREL_SHIFT = 5 };
					b = ((b << BARREL_SHIFT) | (b >> (16 - BARREL_SHIFT))) + tmp + counter++;
					return tmp;
				}
				std::string sfc_v2_16::get_name() const { return "sfc_v2_16"; }
				void sfc_v2_16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(counter);
				}
				Uint32 sfc_v2_32::raw32() {
					Uint32 tmp = a ^ b;
					a = b + (b << 5);
					enum { BARREL_SHIFT = 12 };
					b = ((b << BARREL_SHIFT) | (b >> (32 - BARREL_SHIFT))) + tmp + counter++;
					return tmp;
				}
				std::string sfc_v2_32::get_name() const { return "sfc_v2_32"; }
				void sfc_v2_32::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(counter);
				}
				Uint16 sfc_v3_16::raw16() {
					Uint16 tmp = a + b + counter++;
					a = b ^ (b >> 2);
					enum { BARREL_SHIFT = 5 };
					b = ((b << BARREL_SHIFT) | (b >> (16 - BARREL_SHIFT))) + tmp;
					return tmp;
				}
				std::string sfc_v3_16::get_name() const { return "sfc_v3_16"; }
				void sfc_v3_16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(counter);
				}
				Uint32 sfc_v3_32::raw32() {
					Uint32 tmp = a + b + counter++;
					a = b ^ (b >> 5);
					enum { BARREL_SHIFT = 12 };
					b = ((b << BARREL_SHIFT) | (b >> (32 - BARREL_SHIFT))) + tmp;
					return tmp;
				}
				std::string sfc_v3_32::get_name() const { return "sfc_v3_32"; }
				void sfc_v3_32::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(counter);
				}
				Uint16 jsf16::raw16() {
					Uint16 e = a - ((b << 13) | (b >> 3));
					a = b ^ ((c << 9) | (c >> 7));
					b = c + d;
					c = d + e;
					d = e + a;
					return d;
				}
				std::string jsf16::get_name() const { return "jsf16"; }
				/*seed:
					a = Uint16(s);
					b = Uint16(s >> 16);
					c = Uint16(s >> 32);
					d = Uint16(s >> 48);
					if (!(a|b) && !(c|d)) d++;
					for (int i = 0; i < 20; i++) raw16();
					*/
				void jsf16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
					walker->handle(d);
					if (!(a | b) && !(c | d)) d++;
				}

				Uint32 simpleA::raw32() {
					enum { BARREL_SHIFT1 = 19 };
					Uint32 tmp = b ^ ((a << BARREL_SHIFT1) | (a >> (32 - BARREL_SHIFT1)));
					a = ~b + c;
					b = c;
					c += tmp;
					return b;
				}
				std::string simpleA::get_name() const { return "simpleA"; }
				void simpleA::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint16 simpleB::raw16() {
					enum { BARREL_SHIFT1 = 3, BARREL_SHIFT2 = 5 };
					Uint16 tmp = ((a << BARREL_SHIFT1) | (a >> (16 - BARREL_SHIFT1))) ^ ~b;
					a = b + c;
					b = c;
					c = tmp + ((c << BARREL_SHIFT2) | (c >> (16 - BARREL_SHIFT2)));
					return tmp;
				}
				std::string simpleB::get_name() const { return "simpleB"; }
				void simpleB::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint16 simpleC::raw16() {
					enum { BARREL_SHIFT1 = 3, BARREL_SHIFT2 = 5 };
					Uint16 tmp = ((a << BARREL_SHIFT1) | (a >> (16 - BARREL_SHIFT1))) ^ ~b;
					a = b + ((c << BARREL_SHIFT2) | (c >> (16 - BARREL_SHIFT2)));
					b = c ^ (c >> 2);
					c += tmp;
					return tmp;
				}
				std::string simpleC::get_name() const { return "simpleC"; }
				void simpleC::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint32 simpleD::raw32() {
					enum { BARREL_SHIFT1 = 19 };
					Uint32 old = a;
					Uint32 tmp = b ^ ((a << BARREL_SHIFT1) | (a >> (32 - BARREL_SHIFT1)));
					a = b + c;
					b = c ^ old;
					c = old + tmp;
					return tmp;
				}
				std::string simpleD::get_name() const { return "simpleD"; }
				void simpleD::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint32 simpleE::raw32() {
					Uint32 old = a + b;
					a = b ^ c;
					b = c + old;
					c = old + ((c << 13) | (c >> 19));
					return old;
				}
				std::string simpleE::get_name() const { return "simpleE"; }
				void simpleE::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint16 simpleF::raw16() {
					Uint16 old = a ^ b;
					a = b ^ (c & d);
					b = c + old;
					c = ~d;
					d = old + ((d << 5) | (d >> 11));
					return c;
				}
				std::string simpleF::get_name() const { return "simpleF"; }
				void simpleF::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint32 simpleG::raw32() {
					Uint32 old = a ^ (b >> 7);
					a = b + c + d;
					b = c ^ d;
					c = d + old;
					d = old;
					return a + c;
				}
				std::string simpleG::get_name() const { return "simpleG"; }
				void simpleG::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
					walker->handle(d);
				}

				static Uint64 shift_array64(Uint64 vec[2], unsigned long bits) {
					bits -= 64;
					if (!(bits % 64)) return vec[bits / 64];
					return (vec[bits / 64] << (bits & 63)) | (vec[1 + bits / 64] >> (64 - (bits & 63)));
				}
				Uint32 trivium_weakenedA::raw32() {
					Uint32 tmp_a = Uint32(b >> 2) ^ Uint32(b >> 17);
					Uint32 tmp_b = Uint32(a >> 2) ^ Uint32(a >> 23);
					Uint32 new_a = tmp_a ^ Uint32(a >> 16) ^ (Uint32(b >> 16) & Uint32(b >> 18));
					Uint32 new_b = tmp_b ^ Uint32(b >> 13) ^ (Uint32(a >> 22) & Uint32(a >> 24));
					a <<= 32; a |= new_a;
					b <<= 32; b |= new_b;

					return tmp_a ^ tmp_b;
				}
				std::string trivium_weakenedA::get_name() const { return "trivium_weakenedA"; }
				void trivium_weakenedA::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
				}
				Uint16 trivium_weakenedB::raw16() {
					Uint16 tmp_a = Uint16(c >> 2) ^ Uint16(c >> 15);
					Uint16 tmp_b = Uint16(a >> 2) ^ Uint16(a >> 13);
					Uint16 tmp_c = Uint16(b >> 5) ^ Uint16(b >> 10);
					Uint16 new_a = tmp_a ^ Uint16(a >> 9) ^ (Uint16(c >> 14) & Uint16(c >> 16));
					Uint16 new_b = tmp_b ^ Uint16(b >> 7) ^ (Uint16(a >> 12) & Uint16(a >> 14));
					Uint16 new_c = tmp_c ^ Uint16(c >> 11) ^ (Uint16(b >> 9) & Uint16(b >> 11));
					a <<= 16; a |= new_a;
					b <<= 16; b |= new_b;
					c <<= 16; c |= new_c;

					return tmp_a ^ tmp_b ^ tmp_c;
				}
				std::string trivium_weakenedB::get_name() const { return "trivium_weakenedB"; }
				void trivium_weakenedB::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}

				Uint32 mo_Lesr32::raw32() {
					state = (state << 7) - state; state = rotate32(state, 23);
					return state;
				}
				std::string mo_Lesr32::get_name() const { return "mo_Lesr32"; }
				void mo_Lesr32::walk_state(StateWalkingObject *walker) {
					walker->handle(state);
				}
				Uint32 mo_ResrRers32::raw32() {
					a = rotate32(a, 21) - a; a = rotate32(a, 26);
					b = rotate32(b, 20) - rotate32(b, 9);
					return a ^ b;
				}
				std::string mo_ResrRers32::get_name() const { return "mo_ResrRers32"; }
				void mo_ResrRers32::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
				}
				Uint32 mo_Rers32of64::raw32() {
					state = rotate64(state, 8) - rotate64(state, 29);
					return Uint32(state);
				}
				std::string mo_Rers32of64::get_name() const { return "mo_Rers32of64"; }
				void mo_Rers32of64::walk_state(StateWalkingObject *walker) {
					walker->handle(state);
				}
				Uint32 mo_Resr32of64::raw32() {
					state = rotate64(state, 21) - state; state = rotate64(state, 20);
					return Uint32(state);
				}
				std::string mo_Resr32of64::get_name() const { return "mo_Resr32of64"; }
				void mo_Resr32of64::walk_state(StateWalkingObject *walker) {
					walker->handle(state);
				}
				Uint32 mo_Resdra32of64::raw32() {
					state = rotate64(state, 42) - state;
					state += rotate64(state, 14);
					return Uint32(state);
				}
				std::string mo_Resdra32of64::get_name() const { return "mo_Resdra32of64"; }
				void mo_Resdra32of64::walk_state(StateWalkingObject *walker) {
					walker->handle(state);
				}
				Uint32 murmlacish::raw32() {
					Uint32 tmp = state1 + (state1 << 3);
					tmp ^= tmp >> 8;
					state1 = rotate32(state1, 11) + state2;
					state2 += state3 ^ (state3 >> 7) ^ tmp;
					state3 += tmp + (tmp << 3);
					return state1;
				}
				std::string murmlacish::get_name() const {
					std::ostringstream str;
					str << "murmlacish";
					return str.str();
				}
				void murmlacish::walk_state(StateWalkingObject *walker) {
					walker->handle(state1);
					walker->handle(state2);
					walker->handle(state3);
				}

				Uint16 gjishA::raw16() {
					b += a; c = rotate16(c, 4); a ^= b;
					c += b; a = rotate16(a, 7); b ^= c;
					a += c; b = rotate16(b, 11); c ^= a;
					return a;
				}
				std::string gjishA::get_name() const { return "gjishA"; }
				void gjishA::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint16 gjishB::raw16() {
					b += a; c = rotate16(c, 4); a ^= b;
					c += b; a = rotate16(a, 7); b ^= c;
					c += counter++;
					a += c; b = rotate16(b, 11); c ^= a;
					return a;
				}
				std::string gjishB::get_name() const { return "gjishB"; }
				void gjishB::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
					walker->handle(counter);
				}
				Uint32 gjishC::raw32() {
					b += a; c = rotate32(c, 21); a ^= b;
					c += b; a = rotate32(a, 13); b ^= c;
					a += c; b = rotate32(b, 0); c ^= a;
					return a;
				}
				std::string gjishC::get_name() const { return "gjishC"; }
				void gjishC::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint32 gjishD::raw32() {
					//			4,10,17		4,10,19		5,7,11		5,7,12		5,7,13		5,7,14		5,7,15		5,8,10		5,8,11		5,8,12		5,8,13		5,8,14		5,8,15
					//--big		3			5			1			1			1			1			4			6			2			4			3			1			4
					//--huge	78						32			10			31			23									67						79			62

					//			6,8,15		6,8,17		6,10,13		6,11,20		6,11,18		6,11,15		6,11,14		6,11,19		6,15,29		6,9,21		6,9,17		
					//--big		1			4			7			10			6			2			4			5			7			7			10			
					//--huge	41															81																		

					//			7,9,13		7,9,14	(7,14,9)	7,9,15	(7,15,9)	7,9,16		7,10,15		7,10,14		7,11,17		
					//--big		3			1		1			1		3			7			4			4			3			
					//--huge	50			43		35			40		52												76			

					//			9,13,25		8,13,29		
					//--big		7			2			
					//--huge				65			

					//			21,13,0		21,13,3		21,13,5		19,11,6		18,11,5		23,13,7		23,13,6		21,13,7		12,19,7		12,19,5		14,21,6		
					//--big		~1			~9			~9			~5			6			4			7			1?			4			6			11
					//--huge	61																					82									
					b += a; c = rotate32(c, 5); a ^= b;
					c += b; a = rotate32(a, 8); b ^= c;
					a += c; b = rotate32(b, 16); c ^= a;
					return a;
				}
				std::string gjishD::get_name() const { return "gjishD"; }
				void gjishD::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}

				Uint16 ara16::raw16() {
					a += rotate16(b + c, 3);
					b += rotate16(c + a, 5);
					c += rotate16(a + b, 7);
					return a;
				}
				std::string ara16::get_name() const { return "ara16"; }
				void ara16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint32 ara32::raw32() {
					a += rotate32(b + c, 7);
					b += rotate32(c + a, 11);
					c += rotate32(a + b, 15);
					return a;
				}
				std::string ara32::get_name() const { return "ara32"; }
				void ara32::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint16 arx16::raw16() {
					a ^= rotate16(b + c, 3);
					b ^= rotate16(c + a, 5);
					c ^= rotate16(a + b, 7);
					return a;
				}
				std::string arx16::get_name() const { return "arx16"; }
				void arx16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint32 arx32::raw32() {
					a ^= rotate32(b + c, 7);
					b ^= rotate32(c + a, 11);
					c ^= rotate32(a + b, 15);
					return a;
				}
				std::string arx32::get_name() const { return "arx32"; }
				void arx32::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint16 hara16::raw16() {
					a += rotate16(b + c, 3);
					b ^= rotate16(c + a, 5);
					c += rotate16(a + b, 7);
					return a;
				}
				std::string hara16::get_name() const { return "hara16"; }
				void hara16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint16 harx16::raw16() {
					a ^= rotate16(b + c, 3);
					b += rotate16(c + a, 5);
					c ^= rotate16(a + b, 7);
					return a;
				}
				std::string harx16::get_name() const { return "harx16"; }
				void harx16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}

				Uint16 learx16::raw16() {
					a ^= rotate16(b + c, 3);
					b ^= rotate16(c + (c << 3), 5);
					c ^= rotate16(a + (a << 3), 7);
					return a;
				}
				std::string learx16::get_name() const { return "learx16"; }
				void learx16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint16 hlearx16::raw16() {
					a ^= rotate16(b + c, 3);
					b ^= rotate16(c + (c << 3), 5);
					c += rotate16(a + (a << 3), 7);
					return a;
				}
				std::string hlearx16::get_name() const { return "hlearx16"; }
				void hlearx16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}

				Uint16 alearx16::raw16() {
					a ^= rotate16(b + c, 3);
					b ^= rotate16(c + (c << 3), 5);
					c ^= rotate16(a + (a << 3), 7) + b;
					return a;
				}
				std::string alearx16::get_name() const { return "alearx16"; }
				void alearx16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}

				Uint16 arac16::raw16() {
					a += rotate16(b + c, 3) + counter++;
					b += rotate16(c + a, 5);
					c += rotate16(a + b, 7);
					return a;
				}
				std::string arac16::get_name() const { return "arac16"; }
				void arac16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
					walker->handle(counter);
				}
				Uint16 arxc16::raw16() {
					a ^= rotate16(b + c, 3) + counter++;
					b ^= rotate16(c + a, 5);
					c ^= rotate16(a + b, 7);
					return a;
				}
				std::string arxc16::get_name() const { return "arxc16"; }
				void arxc16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
					walker->handle(counter);
				}

				Uint16 rarns16::raw16() {					//		378		131		124		111		F31		F11		F12		F21		FF1		FE1		FD1		F1A
					Uint16 old1 = xs1, old2 = xs2, old3 = xs3;//							*				*						*		*				
					Uint16 old = xs1;
					xs1 = xs2 ^ (xs2 >> S1);
					xs2 = xs3 ^ (xs3 << S2);
					xs3 = xs3 ^ (xs3 >> S3) ^ old;
					Uint16 rv = old1 + rotate16(old1, 5);
					return (rv) ^ rotate16(xs1 + rv, 7);  //		16 TB?	512 GB	1+ TB	1 GB	128 GB	256 MB	4 GB	16 GB	64 MB	128 MB	2 GB	2 GB	27
					//return rv^rotate16(rv+xs1,9)^rotate16(rv+xs2,3);  //					64 GB			256 GB					1 TB	128 GB					27
					//return (rv ^ xs2) + rotate16(rv + xs1, 3);//			1+ TB			8 GB			16 GB					2 GB	16 GB					31
					//Uint16 rv = old1 + rotate16(old1, 3);
					//return (rv ^ xs2) + rotate16(xs1 + rv, 7);//							16 GB			16 GB					8 GB	1 GB					30
					//Uint16 rv = old2+old1 + rotate16(old1, 3);
					//return (rv ^ xs2) + rotate16(xs1 + rv, 7);//			4+ TB			64 GB	4+ TB	2+ TB					32 GB	16 GB					34
					//Uint16 rv = old1 + rotate16(old1+old2, 3);
					//return (rv ^ xs2) + rotate16(xs1 + rv, 7);//							32 GB			~1+ TB					~1+ TB	512 GB					35
				}
				std::string rarns16::get_name() const { return "rarns16"; }
				void rarns16::walk_state(StateWalkingObject *walker) {
					walker->handle(xs1);
					walker->handle(xs2);
					walker->handle(xs3);
					walker->handle(history);
					if (!(xs1 | xs2 | xs3)) xs1 = 1;
				}
				void rarns16::seed(Uint64 s) {
					xs1 = Uint16(s >> 0);
					xs2 = Uint16(s >> 16);
					xs3 = Uint16(s >> 32);
					if (!(xs1 | xs2 | xs3)) {
						xs1 = Uint16(s >> 48);
						xs2 = 1;
					}
					raw16();
					return;
				}
			}
		}
	}
}
