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
				std::string xsalta16x3::get_name() const {return "xsalta16x3";}
				void xsalta16x3::walk_state(StateWalkingObject *walker) {
					walker->handle(a); walker->handle(b); walker->handle(c);
					if (!a && !b && !c) a = 1;
				}
				Uint16 xsaltb16x3::raw16() {//
					Uint16 tmp, old;
					tmp = (a & b) | (b & c) | (c & a);
					old = a;
					a = b ^ (b >> 3);
					b = c ^ (c << 7);
					c = c ^ (c >> 8) ^ old;
					return c + ((tmp << 5) | (tmp >> 11));
				}
				std::string xsaltb16x3::get_name() const {return "xsaltb16x3";}
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
					return a+b;
				}
				std::string xsaltc16x3::get_name() const {return "xsaltc16x3";}
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
				std::string xorshift32::get_name() const {return "xorshift32";}
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
				std::string xorshift64::get_name() const {return "xorshift64";}
				void xorshift64::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					if (!a) a = 1;
				}
				void xorshift64of128::xrs(int bits) {
					if (bits < 64) {
						low ^= low >> bits;
						low ^= high << (64-bits);
						high ^= high >> bits;
					}
					else if (bits > 64) low ^= high >> (bits - 64);
					else low ^= high;
				}
				void xorshift64of128::xls(int bits) {
					if (bits < 64) {
						high ^= high << bits;
						high ^= low >> (64-bits);
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
				std::string xorshift64of128::get_name() const {return "xorshift64of128";}
				void xorshift64of128::walk_state(StateWalkingObject *walker) {
					walker->handle(low);walker->handle(high);
					if (!high && !low) low = 1;
				}

				std::string xorshift32of128::get_name() const {return "xorshift32of128";}
				void xorshift32of128::walk_state(StateWalkingObject *walker) {impl.walk_state(walker);}
				std::string xorshift32of64::get_name() const {return "xorshift32of64";}
				void xorshift32of64::walk_state(StateWalkingObject *walker) {impl.walk_state(walker);}
				std::string xorshift16of32::get_name() const {return "xorshift16of32";}
				void xorshift16of32::walk_state(StateWalkingObject *walker) {impl.walk_state(walker);}

				Uint32 xorshift32x4::raw32() {
					Uint32 tmp = x ^ (x << 15);
					x = y;
					y = z;
					z = w;
					tmp ^= tmp >> 4;
					w ^= (w >> 21) ^ tmp;
					return w;
				}
				std::string xorshift32x4::get_name() const {return "xorshift32x4";}
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
				std::string xorwow32of96::get_name() const {return "xorwow32of96";}
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
					return v+d;
				}
				std::string xorwow32x6::get_name() const {return "xorwow32x6";}
				void xorwow32x6::walk_state(StateWalkingObject *walker) {
					walker->handle(x);
					walker->handle(y);
					walker->handle(z);
					walker->handle(w);
					walker->handle(v);
					walker->handle(d);
					if (!(x || y || z || w || v)) x = 1;
				}

				Uint32 sapparot::raw32() {
					Uint32 tmp;
					tmp = a + 0x9e3779b9;
					tmp = (tmp<<7)|(tmp>>25);
					a = b ^ (~tmp)^(tmp<<3);
					a=(a<<7)|(a>>25);
					b = tmp;
					return a ^ b;
				}
				std::string sapparot::get_name() const {return "sapparot";}
				void sapparot::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
				}

				Uint16 sap16of48::raw16() {
					Uint16 tmp;
					tmp = a + 0x79b9 + c;
					tmp = (tmp<<5)|(tmp>>11);
					a = b ^ (~tmp)^(tmp<<3);
					a=(a<<5)|(a>>11);
					b = tmp;
					c = (c + a) ^ b;
					return b;
				}
				std::string sap16of48::get_name() const {return "sap16of48";}
				void sap16of48::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint32 sap32of96::raw32() {
					Uint32 tmp;
					tmp = a + 0x9e3779b9 + c;
					tmp = (tmp<<7)|(tmp>>25);
					a = b ^ (~tmp)^(tmp<<3);
					a=(a<<7)|(a>>25);
					b = tmp;
					c = (c + a) ^ b;
					return b;
				}
				std::string sap32of96::get_name() const {return "sap32of96";}
				void sap32of96::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}

				Uint32 flea32x1::raw32() {
					enum {SHIFT1=15, SHIFT2=27};
					Uint32 e = a[d % SIZE];
					a[d % SIZE] = ((b<<SHIFT1)|(b>>(32-SHIFT1)));
					b = c + ((d<<SHIFT2)|(d>>(32-SHIFT2)));
					c = d + a[i++ % SIZE];
					d = e + c;
					return b;
				}
				std::string flea32x1::get_name() const {return "flea32x1";}
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
					enum {BARREL_SHIFT = 5};
					b = ((b << BARREL_SHIFT) | (b >> (16-BARREL_SHIFT))) + tmp;
					return a;
				}
				std::string sfc_v1_16::get_name() const {return "sfc_v1_16";}
				void sfc_v1_16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(counter);
				}
				Uint32 sfc_v1_32::raw32() {
					Uint32 tmp = a ^ counter++;
					a = b + (b << 5);
					enum {BARREL_SHIFT = 12};
					b = ((b << BARREL_SHIFT) | (b >> (32-BARREL_SHIFT))) + tmp;
					return a;
				}
				std::string sfc_v1_32::get_name() const {return "sfc_v1_32";}
				void sfc_v1_32::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(counter);
				}
				Uint16 sfc_v2_16::raw16() {
					Uint16 tmp = a ^ b;
					a = b + (b << 2);
					enum {BARREL_SHIFT = 5};
					b = ((b << BARREL_SHIFT) | (b >> (16-BARREL_SHIFT))) + tmp + counter++;
					return tmp;
				}
				std::string sfc_v2_16::get_name() const {return "sfc_v2_16";}
				void sfc_v2_16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(counter);
				}
				Uint32 sfc_v2_32::raw32() {
					Uint32 tmp = a ^ b;
					a = b + (b << 5);
					enum {BARREL_SHIFT = 12};
					b = ((b << BARREL_SHIFT) | (b >> (32-BARREL_SHIFT))) + tmp + counter++;
					return tmp;
				}
				std::string sfc_v2_32::get_name() const {return "sfc_v2_32";}
				void sfc_v2_32::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(counter);
				}
				Uint16 sfc_v3_16::raw16() {
					Uint16 tmp = a + b + counter++;
					a = b ^ (b >> 2);
					enum {BARREL_SHIFT = 5};
					b = ((b << BARREL_SHIFT) | (b >> (16-BARREL_SHIFT))) + tmp;
					return tmp;
				}
				std::string sfc_v3_16::get_name() const {return "sfc_v3_16";}
				void sfc_v3_16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(counter);
				}
				Uint32 sfc_v3_32::raw32() {
					Uint32 tmp = a + b + counter++;
					a = b ^ (b >> 5);
					enum {BARREL_SHIFT = 12};
					b = ((b << BARREL_SHIFT) | (b >> (32-BARREL_SHIFT))) + tmp;
					return tmp;
				}
				std::string sfc_v3_32::get_name() const {return "sfc_v3_32";}
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
				std::string jsf16::get_name() const {return "jsf16";}
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
					if (!(a|b) && !(c|d)) d++;
				}

				Uint32 simpleA::raw32() {
					enum {BARREL_SHIFT1 = 19};
					Uint32 tmp = b ^ ((a << BARREL_SHIFT1) | (a >> (32-BARREL_SHIFT1)));
					a = ~b + c;
					b = c;
					c += tmp;
					return b;
				}
				std::string simpleA::get_name() const {return "simpleA";}
				void simpleA::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint16 simpleB::raw16() {
					enum {BARREL_SHIFT1 = 3, BARREL_SHIFT2 = 5};
					Uint16 tmp = ((a << BARREL_SHIFT1) | (a >> (16-BARREL_SHIFT1))) ^ ~b;
					a = b + c;
					b = c;
					c = tmp + ((c << BARREL_SHIFT2) | (c >> (16-BARREL_SHIFT2)));
					return tmp;
				}
				std::string simpleB::get_name() const {return "simpleB";}
				void simpleB::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint16 simpleC::raw16() {
					enum {BARREL_SHIFT1 = 3, BARREL_SHIFT2 = 5};
					Uint16 tmp = ((a << BARREL_SHIFT1) | (a >> (16-BARREL_SHIFT1))) ^ ~b;
					a = b + ((c << BARREL_SHIFT2) | (c >> (16-BARREL_SHIFT2)));
					b = c ^ (c >> 2);
					c += tmp;
					return tmp;
				}
				std::string simpleC::get_name() const {return "simpleC";}
				void simpleC::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					walker->handle(c);
				}
				Uint32 simpleD::raw32() {
					enum {BARREL_SHIFT1 = 19};
					Uint32 old = a;
					Uint32 tmp = b ^ ((a << BARREL_SHIFT1) | (a >> (32-BARREL_SHIFT1)));
					a = b + c;
					b = c ^ old;
					c = old + tmp;
					return tmp;
				}
				std::string simpleD::get_name() const {return "simpleD";}
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
				std::string simpleE::get_name() const {return "simpleE";}
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
				std::string simpleF::get_name() const {return "simpleF";}
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
					return a+c;
				}
				std::string simpleG::get_name() const {return "simpleG";}
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
					Uint32 tmp_a = (b >> 2) ^ (b >> 17);
					Uint32 tmp_b = (a >> 2) ^ (a >> 23);
					Uint32 new_a = tmp_a ^ (a >> 16) ^ ((b >> 16) & (b >> 18));
					Uint32 new_b = tmp_b ^ (b >> 13) ^ ((a >> 22) & (a >> 24));
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
					Uint16 tmp_a = (c >> 2) ^ (c >> 15);
					Uint16 tmp_b = (a >> 2) ^ (a >> 13);
					Uint16 tmp_c = (b >> 5) ^ (b >> 10);
					Uint16 new_a = tmp_a ^ (a >> 9) ^ ((c >> 14) & (c >> 16));
					Uint16 new_b = tmp_b ^ (b >> 7) ^ ((a >> 12) & (a >> 14));
					Uint16 new_c = tmp_c ^ (c >> 11) ^ ((b >> 9) & (b >> 11));
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
				std::string mo_Lesr32::get_name() const {return "mo_Lesr32";}
				void mo_Lesr32::walk_state(StateWalkingObject *walker) {
					walker->handle(state);
				}
				Uint32 mo_ResrRers32::raw32() {
					a = rotate32(a, 21) - a; a = rotate32(a,26);
					b = rotate32(b, 20) - rotate32(b, 9);
					return a ^ b;
				}
				std::string mo_ResrRers32::get_name() const {return "mo_ResrRers32";}
				void mo_ResrRers32::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
				}
				Uint32 mo_Rers32of64::raw32() {
					state = rotate64(state, 8) - rotate64(state, 29);
					return Uint32(state);
				}
				std::string mo_Rers32of64::get_name() const {return "mo_Rers32of64";}
				void mo_Rers32of64::walk_state(StateWalkingObject *walker) {
					walker->handle(state);
				}
				Uint32 mo_Resr32of64::raw32() {
					state = rotate64(state, 21) - state; state = rotate64(state, 20);
					return Uint32(state);
				}
				std::string mo_Resr32of64::get_name() const {return "mo_Resr32of64";}
				void mo_Resr32of64::walk_state(StateWalkingObject *walker) {
					walker->handle(state);
				}
				Uint32 mo_Resdra32of64::raw32() {
					state = rotate64(state, 42) - state;
					state += rotate64(state, 14);
					return Uint32(state);
				}
				std::string mo_Resdra32of64::get_name() const {return "mo_Resdra32of64";}
				void mo_Resdra32of64::walk_state(StateWalkingObject *walker) {
					walker->handle(state);
				}
			}
		}
	}
}
