#include <string>
#include <sstream>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"

#include "PractRand/RNGs/mt19937.h"
#include "PractRand/RNGs/other/fibonacci.h"

using namespace PractRand;
using namespace PractRand::Internals;

namespace PractRand {
	namespace RNGs {
		namespace Polymorphic {
			namespace NotRecommended {
				bigbadlcg64X::bigbadlcg64X(int discard_bits_, int shift_bits_) : discard_bits(discard_bits_) {
					int max_discard_bits = MAX_N * 64 - 64;
					if (discard_bits_ < 0 || discard_bits_ > max_discard_bits) issue_error("bigbadlcg64 - discard_bits out of range (0 <= discard_bits <= ?960?)");
					n = (discard_bits_ + 63) / 64 + 1;
					shift_i = shift_bits_ / 64;
					shift_b = shift_bits_ & 63;
					if (shift_bits_ < 1 || shift_bits_ > discard_bits_) issue_error("bigbadlcg64 - shift_bits out of range (1 <= shift_bits <= discard_bits)");
				}
				//bigbadlcgX::~bigbadlcgX() { delete[] state; }
				Uint64 bigbadlcg64X::raw64() {
					static const Uint64 K = 0xB5;//0xA3EC647659359ACDull;
					Uint64 olda[MAX_N];
					for (int i = 0; i < n; i++) olda[i] = state[i];
					Uint64 rv = state[n - 1];
					if (discard_bits & 63) {
						int b = discard_bits & 63;
						rv = (rv << (64 - b)) | (state[n - 2] >> b);
					}
					bool carry = false;
					if (shift_b) {
						if (true) {
							Uint64 old = olda[0] << shift_b;
							state[shift_i] += old;
							carry = state[shift_i] < old;
						}
						for (int i = shift_i+1; i < n; i++) {
							Uint64 old = (olda[i - shift_i] << shift_b) | (olda[i - shift_i - 1] >> (64 - shift_b));
							state[i] += old;
							bool c1 = state[i] < old;
							state[i] += carry ? 1 : 0;
							bool c2 = carry && !state[i];
							carry = c1 || c2;
						}
					}
					else {
						if (true) {
							Uint64 old = olda[0];
							state[shift_i] += old;
							bool carry = state[shift_i] < old;
						}
						for (int i = shift_i+1; i < n; i++) {
							Uint64 old = olda[i - shift_i];
							state[i] += old;
							bool c1 = state[i] < old;
							state[i] += carry ? 1 : 0;
							bool c2 = carry && !state[i];
							carry = c1 || c2;
						}
					}
					state[0] += K;
					state[1] += (state[0] < K) ? 1 : 0;
					if (state[1] == 0 && state[0] < K) {
						for (int i = 2; i < n && !state[i - 1]; i++) state[i]++;
					}
					return rv;
				}
				std::string bigbadlcg64X::get_name() const {
					std::ostringstream str;
					str << "bigbadlcgX(64," << (discard_bits + OUTPUT_BITS) << "," << (shift_i * 64 + shift_b) << ")";
					return str.str();
				}
				void bigbadlcg64X::walk_state(StateWalkingObject *walker) {
					for (int i = 0; i < n; i++) walker->handle(state[i]);
				}
				bigbadlcg32X::bigbadlcg32X(int discard_bits_, int shift_) : base_lcg(discard_bits_, shift_) {}
				Uint32 bigbadlcg32X::raw32() { return Uint32(base_lcg.raw32()); }
				std::string bigbadlcg32X::get_name() const {
					std::ostringstream str;
					str << "bigbadlcgX(32," << (base_lcg.discard_bits + OUTPUT_BITS) << "," << (base_lcg.shift_i * 64 + base_lcg.shift_b) << ")";
					return str.str();
				}
				void bigbadlcg32X::walk_state(StateWalkingObject *walker) { base_lcg.walk_state(walker); }
				bigbadlcg16X::bigbadlcg16X(int discard_bits_, int shift_) : base_lcg(discard_bits_, shift_) {}
				Uint16 bigbadlcg16X::raw16() { return Uint16(base_lcg.raw64()); }
				std::string bigbadlcg16X::get_name() const {
					std::ostringstream str;
					str << "bigbadlcgX(16," << (base_lcg.discard_bits + OUTPUT_BITS) << "," << (base_lcg.shift_i * 64 + base_lcg.shift_b) << ")";
					return str.str();
				}
				void bigbadlcg16X::walk_state(StateWalkingObject *walker) { base_lcg.walk_state(walker); }
				bigbadlcg8X::bigbadlcg8X(int discard_bits_, int shift_) : base_lcg(discard_bits_, shift_) {}
				Uint8 bigbadlcg8X::raw8() { return Uint8(base_lcg.raw32()); }
				std::string bigbadlcg8X::get_name() const {
					std::ostringstream str;
					str << "bigbadlcgX(8," << (base_lcg.discard_bits + OUTPUT_BITS) << "," << (base_lcg.shift_i * 64 + base_lcg.shift_b) << ")";
					return str.str();
				}
				void bigbadlcg8X::walk_state(StateWalkingObject *walker) { base_lcg.walk_state(walker); }

				Uint8 lfsr_medium::raw8() {
					if (used < SIZE) return cbuf[used++];
					for (int i = 0; i < LAG; i++) {
						cbuf[i] ^= cbuf[i+(SIZE-LAG)] ^ table1[cbuf[i+1]] ^ table2[cbuf[i+2]];
					}
					for (int i = LAG; i < SIZE-2; i++) {
						cbuf[i] ^= cbuf[i-LAG] ^ table1[cbuf[i+1]] ^ table2[cbuf[i+2]];
					}
					cbuf[SIZE-2] ^= cbuf[SIZE-2-LAG] ^ table1[cbuf[SIZE-1]] ^ table2[cbuf[0]];
					cbuf[SIZE-1] ^= cbuf[SIZE-1-LAG] ^ table1[cbuf[0]] ^ table2[1];
					used = 1;
					return cbuf[0];
				}
				std::string lfsr_medium::get_name() const {return "lfsr_medium";}
				void lfsr_medium::walk_state(StateWalkingObject *walker) {
					for (int i = 0; i < SIZE; i++) walker->handle(cbuf[i]);
					walker->handle(used);
					if (used >= SIZE) used = 0;
				}
				lfsr_medium::lfsr_medium() {
					used = 0;
					Uint8 vartaps = 1+2;//255 - 16;
					for (Uint32 i = 0; i < 256; i++) {
						Uint8 low = 0;
						Uint8 high = 0;
						for (int b = 0; b < 8; b++) {
							if ((vartaps >> b) & 1) {
								low ^= i >> b;
								if (b) high ^= i << (8-b);
							}
						}
						table1[i] = low;
						table2[i] = high;
					}
				}

				//Mitchell-Moore: LFib32(Uint32, 55, 24, ADD)
				Uint32 mm32::raw32() {
					Uint32 tmp;
					tmp = cbuf[index1] += cbuf[index2];
					if ( ++index1 == 55 ) index1 = 0;
					if ( ++index2 == 55 ) index2 = 0;
					return tmp;
				}
				std::string mm32::get_name() const {return "mm32";}
				void mm32::walk_state(StateWalkingObject *walker) {
					walker->handle(index1);
					for (int i = 0; i < 55; i++) walker->handle(cbuf[i]);
					if (index1 >= 55) index1 %= 55;
					index2 = index1 - 24;
					if (index2 >= 55) index2 += 55;//it's an unsigned value
				}
				//Mitchell-Moore modified: LFib16(Uint32, 55, 24, ADD) >> 16
				Uint16 mm16of32::raw16() {
					Uint32 tmp;
					tmp = cbuf[index1] += cbuf[index2];
					if ( ++index1 == 55 ) index1 = 0;
					if ( ++index2 == 55 ) index2 = 0;
					return tmp >> 16;
				}
				std::string mm16of32::get_name() const {return "mm16of32";}
				void mm16of32::walk_state(StateWalkingObject *walker) {
					walker->handle(index1);
					for (int i = 0; i < 55; i++) walker->handle(cbuf[i]);
					if (index1 >= 55) index1 %= 55;
					index2 = index1 - 24;
					if (index2 >= 55) index2 += 55;//it's an unsigned value
				}
				//Mitchell-Moore modified: LFib32(Uint32, 55, 24, ADD) >> 16
				Uint32 mm32_awc::raw32() {
					Uint32 tmp1, tmp2, tmp3;
					tmp1 = cbuf[index1];
					tmp2 = cbuf[index2];
					tmp3 = tmp1 + tmp2 + carry;
					cbuf[index1] = tmp3;
					carry = (tmp3 - tmp1) >> 31;
					if ((tmp3 == tmp1) && tmp2) carry = 1;
					if (++index1 == 55) index1 = 0;
					if (++index2 == 55) index2 = 0;
					return tmp3;
				}
				std::string mm32_awc::get_name() const { return "mm32_awc"; }
				void mm32_awc::walk_state(StateWalkingObject *walker) {
					walker->handle(carry);
					walker->handle(index1);
					for (int i = 0; i < 55; i++) walker->handle(cbuf[i]);
					if (index1 >= 55) index1 %= 55;
					index2 = index1 - 24;
					if (index2 >= 55) index2 += 55;//it's an unsigned value
					carry &= 1;
				}
				//Mitchell-Moore modified: LFib32(Uint32, 55, 24, ADC) >> 16
				Uint16 mm16of32_awc::raw16() {
					Uint32 tmp1, tmp2, tmp3;
					tmp1 = cbuf[index1];
					tmp2 = cbuf[index2];
					tmp3 = tmp1 + tmp2 + carry;
					cbuf[index1] = tmp3;
					carry = (tmp3 - tmp1) >> 31;
					if ((tmp3 == tmp1) && tmp2) carry = 1;
					if (++index1 == 55) index1 = 0;
					if (++index2 == 55) index2 = 0;
					return Uint16(tmp3);
				}
				std::string mm16of32_awc::get_name() const { return "mm16of32_awc"; }
				void mm16of32_awc::walk_state(StateWalkingObject *walker) {
					walker->handle(carry);
					walker->handle(index1);
					for (int i = 0; i < 55; i++) walker->handle(cbuf[i]);
					if (index1 >= 55) index1 %= 55;
					index2 = index1 - 24;
					if (index2 >= 55) index2 += 55;//it's an unsigned value
					carry &= 1;
				}

				//used by Marsaglia in KISS4691 (2010)
				Uint32 mwc4691::raw32() {
					index = (index < 4691-1) ? index + 1 : 0;
					Uint32 x, t;
					x = cbuf[index];
					t = (x << 13) + carry + x;
					carry = (x>>19) + (t<=x);
					if (!t && !x) carry--;
					cbuf[index] = t;
					return t;
				}
				std::string mwc4691::get_name() const {return "mwc4691";}
				void mwc4691::walk_state(StateWalkingObject *walker) {
					walker->handle(index);
					walker->handle(carry);
					for (int i = 0; i < 4691; i++) walker->handle(cbuf[i]);
				}
				
				//
				Uint32 cbuf_accum::raw32() {
					Uint32 tmp = cbuf[--index];
					accum = ((accum << 11) | (accum >> 21)) + ~tmp;
					cbuf[index] = accum;
					if (!index) index = L;
					return accum;
				}
				std::string cbuf_accum::get_name() const { return "cbuf_accum"; }
				void cbuf_accum::walk_state(StateWalkingObject *walker) {
					walker->handle(index);
					walker->handle(accum);
					for (int i = 0; i < L; i++) walker->handle(cbuf[i]);
					if (index >= L) index %= L;
					if (!index) index = L;
				}
				Uint32 cbuf_accum_big::raw32() {
					Uint32 tmp = cbuf[--index];
					accum = ((accum << 11) | (accum >> 21)) + ~tmp;
					cbuf[index] = accum;
					if (!index) index = L;
					return accum;
				}
				std::string cbuf_accum_big::get_name() const { return "cbuf_accum_big"; }
				void cbuf_accum_big::walk_state(StateWalkingObject *walker) {
					walker->handle(index);
					walker->handle(accum);
					for (int i = 0; i < L; i++) walker->handle(cbuf[i]);
					if (index >= L) index %= L;
					if (!index) index = L;
				}
				Uint32 cbuf_2accum_small::raw32() {
					Uint32 tmp = cbuf[--index] + accum2;
					accum2 += accum1;
					enum { SHIFT = 11 };// 3,11 for small, 12,11 for medium
					accum1 = ((accum1 << SHIFT) | (accum1 >> (32-SHIFT))) ^ tmp;
					//		1	2	3	4	5	6	7	8	9	10	11	12	13	14	15	16	17	18	19	20	21	22	23	24	25	26	27	28	29	30	31
					//	3	22	27	28	28	29	30	31	37	43	42	39	43	37	37	37	16	36				42			37	31	30	29	28	28	26	23
					//	4	23	28	28	29	29	31	32	44?							40	17	40							>40	31	30	29	28	28	26	24
					//	5	24	28	29	29	30	31	33									17
					//	6	25	29	30	30	30	31	33									17
					//	7	27	30	31	31	31	33	34									17
					//	8	34	38	40	41												18
					cbuf[index] = accum2;
					if (!index) index = L;
					return accum2;
				}
				std::string cbuf_2accum_small::get_name() const { return "cbuf_2accum_small"; }
				void cbuf_2accum_small::walk_state(StateWalkingObject *walker) {
					walker->handle(index);
					walker->handle(accum1);
					walker->handle(accum2);
					for (int i = 0; i < L; i++) walker->handle(cbuf[i]);
					if (index >= L) index %= L;
					if (!index) index = L;
				}
				Uint32 cbuf_2accum::raw32() {
					Uint32 tmp = cbuf[--index] + accum2;
					accum2 += accum1;
					accum1 = ((accum1 << 11) | (accum1 >> 21)) ^ tmp;
					cbuf[index] = accum2;
					if (!index) index = L;
					return accum2;
				}
				std::string cbuf_2accum::get_name() const { return "cbuf_2accum"; }
				void cbuf_2accum::walk_state(StateWalkingObject *walker) {
					walker->handle(index);
					walker->handle(accum1);
					walker->handle(accum2);
					for (int i = 0; i < L; i++) walker->handle(cbuf[i]);
					if (index >= L) index %= L;
					if (!index) index = L;
				}
				Uint32 dual_cbuf_small::raw32() {
					Uint32 tmp1, tmp2;
					tmp1 = cbuf1[--index1];
					tmp2 = cbuf2[--index2];
					cbuf1[index1] = tmp1 + tmp2;
					cbuf2[index2] = ((tmp1 << 11) | (tmp1 >> 21)) ^ tmp2;
					if (!index1) index1 = L1;
					if (!index2) index2 = L2;
					return tmp1 + tmp2;
				}
				std::string dual_cbuf_small::get_name() const { return "dual_cbuf_small"; }
				void dual_cbuf_small::walk_state(StateWalkingObject *walker) {
					walker->handle(index1);
					walker->handle(index2);
					for (int i = 0; i < L1; i++) walker->handle(cbuf1[i]);
					for (int i = 0; i < L2; i++) walker->handle(cbuf2[i]);
					if (index1 > L1) index1 %= L1;
					if (!index1) index1 = L1;
					if (index2 > L2) index2 %= L2;
					if (!index2) index2 = L2;
				}
				Uint32 dual_cbuf::raw32() {
					Uint32 tmp1, tmp2;
					tmp1 = cbuf1[--index1];
					tmp2 = cbuf2[--index2];
					cbuf1[index1] = tmp1 + tmp2;
					cbuf2[index2] = ((tmp1 << 11) | (tmp1 >> 21)) ^ tmp2;
					if (!index1) index1 = L1;
					if (!index2) index2 = L2;
					return tmp1 + tmp2;
				}
				std::string dual_cbuf::get_name() const { return "dual_cbuf"; }
				void dual_cbuf::walk_state(StateWalkingObject *walker) {
					walker->handle(index1);
					walker->handle(index2);
					for (int i = 0; i < L1; i++) walker->handle(cbuf1[i]);
					for (int i = 0; i < L2; i++) walker->handle(cbuf2[i]);
					if (index1 > L1) index1 %= L1;
					if (!index1) index1 = L1;
					if (index2 > L2) index2 %= L2;
					if (!index2) index2 = L2;
				}
				Uint32 dual_cbufa_small::raw32() {
					Uint32 tmp1, tmp2;
					tmp1 = cbuf1[--index1];
					tmp2 = cbuf2[--index2];
					accum = ((accum << 11) | (accum >> 21)) + tmp1;
					cbuf1[index1] = tmp1 ^ tmp2;
					cbuf2[index2] = accum;
					if ( !index1 ) index1 = L1;
					if ( !index2 ) index2 = L2;
					return accum;
				}
				std::string dual_cbufa_small::get_name() const { return "dual_cbufa_small"; }
				void dual_cbufa_small::walk_state(StateWalkingObject *walker) {
					walker->handle(index1);
					walker->handle(index2);
					walker->handle(accum);
					for (int i = 0; i < L1; i++) walker->handle(cbuf1[i]);
					for (int i = 0; i < L2; i++) walker->handle(cbuf2[i]);
					if (index1 > L1) index1 %= L1;
					if (index2 > L2) index2 %= L2;
					if ( !index1 ) index1 = L1;
					if ( !index2 ) index2 = L2;
				}
				Uint32 dual_cbuf_accum::raw32() {
					Uint32 tmp1, tmp2;
					tmp1 = cbuf1[--index1];
					tmp2 = cbuf2[--index2];
					accum = ((accum << 11) | (accum >> 21)) + tmp1;
					cbuf1[index1] = tmp1 ^ tmp2;
					cbuf2[index2] = accum;
					if (!index1) index1 = L1;
					if (!index2) index2 = L2;
					return accum;
				}
				std::string dual_cbuf_accum::get_name() const { return "dual_cbuf_accum"; }
				void dual_cbuf_accum::walk_state(StateWalkingObject *walker) {
					walker->handle(index1);
					walker->handle(index2);
					walker->handle(accum);
					for (int i = 0; i < L1; i++) walker->handle(cbuf1[i]);
					for (int i = 0; i < L2; i++) walker->handle(cbuf2[i]);
					if (index1 > L1) index1 %= L1;
					if (index2 > L2) index2 %= L2;
					if (!index1) index1 = L1;
					if (!index2) index2 = L2;
				}



				Uint32 ranrot32small::raw32() {
					if (position) return buffer[--position];
					for (unsigned long i = 0; i < LAG2; i++) {
						buffer[i] =
							((buffer[i + LAG1 - LAG1] << ROT1) | (buffer[i + LAG1 - LAG1] >> (sizeof(buffer[0]) * 8 - ROT1))) +
							((buffer[i + LAG1 - LAG2] << ROT2) | (buffer[i + LAG1 - LAG2] >> (sizeof(buffer[0]) * 8 - ROT2)));
					}
					for (unsigned long i = LAG2; i < LAG1; i++) {
						buffer[i] =
							((buffer[i - 0] << ROT1) | (buffer[i - 0] >> (sizeof(buffer[0]) * 8 - ROT1))) +
							((buffer[i - LAG2] << ROT2) | (buffer[i - LAG2] >> (sizeof(buffer[0]) * 8 - ROT2)));
					}
					position = LAG1;
					return buffer[--position];
				}
				std::string ranrot32small::get_name() const { return "ranrot32small"; }
				void ranrot32small::walk_state(StateWalkingObject *walker) {
					walker->handle(position);
					for (int i = 0; i < LAG1; i++) walker->handle(buffer[i]);
					if (position >= LAG1) position %= LAG1;
				}
				Uint32 ranrot32::raw32() {
					if (position) return buffer[--position];
					for (unsigned long i = 0; i < LAG2; i++) {
						buffer[i] =
							((buffer[i + LAG1 - LAG1] << ROT1) | (buffer[i + LAG1 - LAG1] >> (sizeof(buffer[0]) * 8 - ROT1))) +
							((buffer[i + LAG1 - LAG2] << ROT2) | (buffer[i + LAG1 - LAG2] >> (sizeof(buffer[0]) * 8 - ROT2)));
					}
					for (unsigned long i = LAG2; i < LAG1; i++) {
						buffer[i] =
							((buffer[i - 0] << ROT1) | (buffer[i - 0] >> (sizeof(buffer[0]) * 8 - ROT1))) +
							((buffer[i - LAG2] << ROT2) | (buffer[i - LAG2] >> (sizeof(buffer[0]) * 8 - ROT2)));
					}
					position = LAG1;
					return buffer[--position];
				}
				std::string ranrot32::get_name() const { return "ranrot32"; }
				void ranrot32::walk_state(StateWalkingObject *walker) {
					walker->handle(position);
					for (int i = 0; i < LAG1; i++) walker->handle(buffer[i]);
					if (position >= LAG1) position %= LAG1;
				}
				Uint32 ranrot32big::raw32() {
					if (position) return buffer[--position];
					for (unsigned long i = 0; i < LAG2; i++) {
						buffer[i] =
							((buffer[i + LAG1 - LAG1] << ROT1) | (buffer[i + LAG1 - LAG1] >> (sizeof(buffer[0]) * 8 - ROT1))) +
							((buffer[i + LAG1 - LAG2] << ROT2) | (buffer[i + LAG1 - LAG2] >> (sizeof(buffer[0]) * 8 - ROT2)));
					}
					for (unsigned long i = LAG2; i < LAG1; i++) {
						buffer[i] =
							((buffer[i - 0] << ROT1) | (buffer[i - 0] >> (sizeof(buffer[0]) * 8 - ROT1))) +
							((buffer[i - LAG2] << ROT2) | (buffer[i - LAG2] >> (sizeof(buffer[0]) * 8 - ROT2)));
					}
					position = LAG1;
					return buffer[--position];
				}
				std::string ranrot32big::get_name() const { return "ranrot32big"; }
				void ranrot32big::walk_state(StateWalkingObject *walker) {
					walker->handle(position);
					for (int i = 0; i < LAG1; i++) walker->handle(buffer[i]);
					if (position >= LAG1) position %= LAG1;
				}
				Uint32 ranrot3tap32small::func(Uint32 a, Uint32 b, Uint32 c) {
					return rotate(a, ROT1) + rotate(b, ROT2) + rotate(c, ROT3);//30 @ 7/5, 36 @ 11/7, 36 @ 17/9
				}
				Uint32 ranrot3tap32small::raw32() {
					if (position) return buffer[--position];
					Uint32 old = buffer[LAG1 - 1];
					for (unsigned long i = 0; i < LAG2; i++) {
						buffer[i] = old = func(buffer[i + LAG1 - LAG1], buffer[i + LAG1 - LAG2], old);
					}
					for (unsigned long i = LAG2; i < LAG1; i++) {
						buffer[i] = old = func(buffer[i], buffer[i - LAG2], old);
					}
					position = LAG1;
					return buffer[--position];
				}
				std::string ranrot3tap32small::get_name() const { return "ranrot3tap32small"; }
				void ranrot3tap32small::walk_state(StateWalkingObject *walker) {
					walker->handle(position);
					for (int i = 0; i < LAG1; i++) walker->handle(buffer[i]);
					if (position >= LAG1) position %= LAG1;
				}
				Uint32 ranrot3tap32::func(Uint32 a, Uint32 b, Uint32 c) {
					return rotate(a, ROT1) + rotate(b, ROT2) + rotate(c, ROT3);//30 @ 7/5, 36 @ 11/7, 36 @ 17/9
				}
				Uint32 ranrot3tap32::raw32() {
					if (position) return buffer[--position];
					Uint32 old = buffer[LAG1 - 1];
					for (unsigned long i = 0; i < LAG2; i++) {
						buffer[i] = old = func(buffer[i + LAG1 - LAG1], buffer[i + LAG1 - LAG2], old);
					}
					for (unsigned long i = LAG2; i < LAG1; i++) {
						buffer[i] = old = func(buffer[i], buffer[i - LAG2], old);
					}
					position = LAG1;
					return buffer[--position];
				}
				std::string ranrot3tap32::get_name() const { return "ranrot3tap32"; }
				void ranrot3tap32::walk_state(StateWalkingObject *walker) {
					walker->handle(position);
					for (int i = 0; i < LAG1; i++) walker->handle(buffer[i]);
					if (position >= LAG1) position %= LAG1;
				}
				Uint32 ranrot3tap32big::func(Uint32 a, Uint32 b, Uint32 c) {
					return rotate(a, ROT1) + rotate(b, ROT2) + rotate(c, ROT3);//30 @ 7/5, 36 @ 11/7, 36 @ 17/9
				}
				Uint32 ranrot3tap32big::raw32() {
					if (position) return buffer[--position];
					Uint32 old = buffer[LAG1 - 1];
					for (unsigned long i = 0; i < LAG2; i++) {
						buffer[i] = old = func(buffer[i + LAG1 - LAG1], buffer[i + LAG1 - LAG2], old);
					}
					for (unsigned long i = LAG2; i < LAG1; i++) {
						buffer[i] = old = func(buffer[i], buffer[i - LAG2], old);
					}
					position = LAG1;
					return buffer[--position];
				}
				std::string ranrot3tap32big::get_name() const { return "ranrot3tap32big"; }
				void ranrot3tap32big::walk_state(StateWalkingObject *walker) {
					walker->handle(position);
					for (int i = 0; i < LAG1; i++) walker->handle(buffer[i]);
					if (position >= LAG1) position %= LAG1;
				}
				Uint32 ranrot32hetsmall::func(Uint32 a, Uint32 b, Uint32 c) {
					a = rotate(a, ROT1); b = rotate(b, ROT2); c = rotate(c, ROT3);
					return (a + b) ^ c;
				}
				Uint32 ranrot32hetsmall::raw32() {
					if (position) return buffer[--position];
					Uint32 old = buffer[LAG1 - 1];
					for (unsigned long i = 0; i < LAG2; i++) {
						buffer[i] = old = func(buffer[i + LAG1 - LAG1], buffer[i + LAG1 - LAG2], old);
					}
					for (unsigned long i = LAG2; i < LAG1; i++) {
						buffer[i] = old = func(buffer[i], buffer[i - LAG2], old);
					}
					position = LAG1;
					return buffer[--position];
				}
				std::string ranrot32hetsmall::get_name() const { return "ranrot32hetsmall"; }
				void ranrot32hetsmall::walk_state(StateWalkingObject *walker) {
					walker->handle(position);
					for (int i = 0; i < LAG1; i++) walker->handle(buffer[i]);
					if (position >= LAG1) position %= LAG1;
				}
				Uint32 ranrot32het::func(Uint32 a, Uint32 b, Uint32 c) {
					a = rotate(a, ROT1); b = rotate(b, ROT2); c = rotate(c, ROT3);
					return (a + b) ^ c;
				}
				Uint32 ranrot32het::raw32() {
					if (position) return buffer[--position];
					Uint32 old = buffer[LAG1 - 1];
					for (unsigned long i = 0; i < LAG2; i++) {
						buffer[i] = old = func(buffer[i + LAG1 - LAG1], buffer[i + LAG1 - LAG2], old);
					}
					for (unsigned long i = LAG2; i < LAG1; i++) {
						buffer[i] = old = func(buffer[i], buffer[i - LAG2], old);
					}
					position = LAG1;
					return buffer[--position];
				}
				std::string ranrot32het::get_name() const { return "ranrot32het"; }
				void ranrot32het::walk_state(StateWalkingObject *walker) {
					walker->handle(position);
					for (int i = 0; i < LAG1; i++) walker->handle(buffer[i]);
					if (position >= LAG1) position %= LAG1;
				}
				Uint32 ranrot32hetbig::func(Uint32 a, Uint32 b, Uint32 c) {
					a = rotate(a, ROT1); b = rotate(b, ROT2); c = rotate(c, ROT3);
					return (a + b) ^ c;
				}
				Uint32 ranrot32hetbig::raw32() {
					if (position) return buffer[--position];
					Uint32 old = buffer[LAG1 - 1];
					for (unsigned long i = 0; i < LAG2; i++) {
						buffer[i] = old = func(buffer[i + LAG1 - LAG1], buffer[i + LAG1 - LAG2], old);
					}
					for (unsigned long i = LAG2; i < LAG1; i++) {
						buffer[i] = old = func(buffer[i], buffer[i - LAG2], old);
					}
					position = LAG1;
					return buffer[--position];
				}
				std::string ranrot32hetbig::get_name() const { return "ranrot32hetbig"; }
				void ranrot32hetbig::walk_state(StateWalkingObject *walker) {
					walker->handle(position);
					for (int i = 0; i < LAG1; i++) walker->handle(buffer[i]);
					if (position >= LAG1) position %= LAG1;
				}

				Uint16 fibmul16of32::raw16() {
					if (position) return Uint16(buffer[--position] >> 16);
					for (unsigned long i = 0; i < LAG2; i++) {
						buffer[i] = buffer[i+LAG1-LAG1] * buffer[i+LAG1-LAG2];
					}
					for (unsigned long i = LAG2; i < LAG1; i++) {
						buffer[i] = buffer[i] * buffer[i-LAG2];
					}
					position = LAG1;
					return Uint16(buffer[--position] >> 16);
				}
				std::string fibmul16of32::get_name() const {return "fibmul16of32";}
				void fibmul16of32::walk_state(StateWalkingObject *walker) {
					walker->handle(position);
					for (int i = 0; i < LAG1; i++) walker->handle(buffer[i]);
					if (position >= LAG1) position %= LAG1;
					for (int i = 0; i < LAG1; i++) buffer[i] |= 1;
				}
				Uint32 fibmul32of64::raw32() {
					if (position) return Uint32(buffer[--position] >> 32);
					for (unsigned long i = 0; i < LAG2; i++) {
						buffer[i] = buffer[i+LAG1-LAG1] * buffer[i+LAG1-LAG2];
					}
					for (unsigned long i = LAG2; i < LAG1; i++) {
						buffer[i] = buffer[i] * buffer[i-LAG2];
					}
					position = LAG1;
					return Uint32(buffer[--position] >> 32);
				}
				std::string fibmul32of64::get_name() const {return "fibmul32of64";}
				void fibmul32of64::walk_state(StateWalkingObject *walker) {
					walker->handle(position);
					for (int i = 0; i < LAG1; i++) walker->handle(buffer[i]);
					if (position >= LAG1) position %= LAG1;
					for (int i = 0; i < LAG1; i++) buffer[i] |= 1;
				}


				Uint32 mt19937_unhashed::raw32() {
					return implementation.untempered_raw32();
				}
				std::string mt19937_unhashed::get_name() const {return "mt19937_unhashed";}
				void mt19937_unhashed::walk_state(StateWalkingObject *walker) {
					implementation.walk_state(walker);
				}
			}
		}
	}
}
