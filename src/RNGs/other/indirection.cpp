#include <string>
#include <sstream>
#include <cstdlib>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"

#include "PractRand/RNGs/other/indirection.h"
#include "PractRand/RNGs/arbee.h"
//#include "PractRand/test_helpers.h"

using namespace PractRand::Internals;

namespace PractRand {
	namespace RNGs {
		namespace Polymorphic {
			namespace NotRecommended {
				Uint8 rc4::raw8() {
					b += arr[a];
					Uint8 tmp = arr[b];
					arr[b] = arr[a];
					arr[a] = tmp;
					return arr[Uint8(arr[a++] + arr[b])];
				}
				std::string rc4::get_name() const {return "rc4";}
				void rc4::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					if (walker->is_clumsy() && !walker->is_read_only()) {
						Uint64 seed;
						walker->handle(seed);
						PractRand::RNGs::Raw::arbee seeder(seed);
						for (int i = 0; i < 256; i++) arr[i] = i;
						for (int i = 0; i < 256; i++) {
							Uint8 ai = i, bi = seeder.raw8();
							Uint8 tmp = arr[ai]; arr[ai] = arr[bi]; arr[bi] = tmp;
						}
					}
					else {
						for (int i = 0; i < 256; i++) walker->handle(arr[i]);
					}
				}

				Uint8 rc4_weakened::raw8() {
					b += arr[a];
					Uint8 tmp = arr[b];
					arr[b] = arr[a];
					arr[a++] = tmp;
					return tmp;
				}
				std::string rc4_weakened::get_name() const {return "rc4_weakened";}
				void rc4_weakened::walk_state(StateWalkingObject *walker) {
					walker->handle(a);
					walker->handle(b);
					if (walker->is_clumsy() && !walker->is_read_only()) {
						Uint64 seed;
						walker->handle(seed);
						PractRand::RNGs::Raw::arbee seeder(seed);
						seeder.walk_state(walker);
						for (int i = 0; i < 256; i++) arr[i] = i;
						for (int i = 0; i < 256; i++) {
							Uint8 ai = i, bi = seeder.raw8();
							Uint8 tmp = arr[ai]; arr[ai] = arr[bi]; arr[bi] = tmp;
						}
					}
					else {
						for (int i = 0; i < 256; i++) walker->handle(arr[i]);
					}
				}

				Uint8 ibaa8::raw8() {
					if (left) {
						return table[--left];
					}
					const int half_size = 1<<(table_size_L2-1);
					const int mask = (1<<table_size_L2)-1;
					Uint8 *base = &table[mask+1];
					for (int i = 0; i <= mask; i++) {
						Uint8 x, y;
						x = base[i];
						a = ((a << 5) | (a >> 3)) + base[(i+half_size) & mask];
						y = base[x & mask] + a + b;
						base[i] = y;
						b = base[(y >> table_size_L2) & mask] + x;
						table[i] = b;
					}
					left = mask;
					return table[mask];
				}
				std::string ibaa8::get_name() const {
					std::ostringstream tmp;
					int table_size = 1<<table_size_L2;
					tmp << "ibaa8x" << table_size;
					return tmp.str();
				}
				void ibaa8::walk_state(StateWalkingObject *walker) {
					walker->handle(a);walker->handle(b);
					int table_size = 1<<table_size_L2;
					for (int i = 0; i < table_size * 2; i++) walker->handle(table[i]);
					walker->handle(left);
					if (left >= table_size) left = 0;
				}
				ibaa8::ibaa8(int table_size_L2_) : table_size_L2(table_size_L2_) {
					table = new Uint8[2 << table_size_L2];
				}
				ibaa8::~ibaa8() {delete[] table;}

				Uint16 ibaa16::raw16() {
					if (left) {
						return table[--left];
					}
					const int half_size = 1<<(table_size_L2-1);
					const int mask = (1<<table_size_L2)-1;
					Uint16 *base = &table[mask+1];
					for (int i = 0; i <= mask; i++) {
						Uint16 x, y;
						x = base[i];
						a = ((a << 11) | (a >> 5)) + base[(i+half_size) & mask];
						y = base[x & mask] + a + b;
						base[i] = y;
						b = base[(y >> table_size_L2) & mask] + x;
						table[i] = b;
					}
					left = mask;
					return table[mask];
				}
				std::string ibaa16::get_name() const {
					std::ostringstream tmp;
					int table_size = 1<<table_size_L2;
					tmp << "ibaa16x" << table_size;
					return tmp.str();
				}
				void ibaa16::walk_state(StateWalkingObject *walker) {
					walker->handle(a);walker->handle(b);
					int table_size = 1<<table_size_L2;
					for (int i = 0; i < table_size * 2; i++) walker->handle(table[i]);
					walker->handle(left);
					if (left >= table_size) left = 0;
				}
				ibaa16::ibaa16(int table_size_L2_) : table_size_L2(table_size_L2_) {
					table = new Uint16[2 << table_size_L2];
				}
				ibaa16::~ibaa16() {delete[] table;}

				Uint32 ibaa32::raw32() {
					if (left) {
						return table[--left];
					}
					const int half_size = 1<<(table_size_L2-1);
					const int mask = (1<<table_size_L2)-1;
					Uint32 *base = &table[mask+1];
					for (int i = 0; i <= mask; i++) {
						Uint32 x, y;
						x = base[i];
						a = ((a << 19) | (a >> 13)) + base[(i+half_size) & mask];
						y = base[x & mask] + a + b;
						base[i] = y;
						b = base[(y >> table_size_L2) & mask] + x;
						table[i] = b;
					}
					left = mask;
					return table[mask];
				}
				std::string ibaa32::get_name() const {
					std::ostringstream tmp;
					int table_size = 1<<table_size_L2;
					tmp << "ibaa32x" << table_size;
					return tmp.str();
				}
				void ibaa32::walk_state(StateWalkingObject *walker) {
					walker->handle(a);walker->handle(b);
					unsigned long table_size = 1<<table_size_L2;
					for (unsigned long i = 0; i < table_size * 2; i++) walker->handle(table[i]);
					walker->handle(left);
					if (left >= table_size) left = 0;
				}
				ibaa32::ibaa32(int table_size_L2_) : table_size_L2(table_size_L2_) {
					table = new Uint32[2 << table_size_L2];
				}
				ibaa32::~ibaa32() {delete[] table;}

				
				#define ind32(mm,x)  (*(Uint32 *)(((Uint8 *)(mm)) + ((x) & ((MASK)<<2))))
				#define rngstep32(mix,a,b,mm,m,m2,r,x) \
				{ \
				  x = *m;  \
				  a = (a^(mix)) + *(m2++); \
				  *(m++) = y = ind32(mm,x) + a + b; \
				  *(r++) = b = ind32(mm,y>>table_size_L2) + x; \
				}
				Uint32 isaac32_varqual::raw32() {
					if (left) {
						return table[--left];
					}
					const int HALF_SIZE = 1<<(table_size_L2-1);
					const int MASK = (1<<table_size_L2)-1;
					Uint32 *base = &table[MASK+1];
					Uint32 *m, *m2, *mend, *r;
					Uint32 x, y;
					m = base;
					r = table;
					b += ++c;
					if (table_size_L2 != 2) {
						for (m = base, mend = m2 = m+HALF_SIZE; m<mend; )
						{
							rngstep32( a<<13, a, b, base, m, m2, r, x);
							rngstep32( a>> 6, a, b, base, m, m2, r, x);
							rngstep32( a<< 2, a, b, base, m, m2, r, x);
							rngstep32( a>>16, a, b, base, m, m2, r, x);
						}
						for (m2 = base; m2<mend; )
						{
							rngstep32( a<<13, a, b, base, m, m2, r, x);
							rngstep32( a>> 6, a, b, base, m, m2, r, x);
							rngstep32( a<< 2, a, b, base, m, m2, r, x);
							rngstep32( a>>16, a, b, base, m, m2, r, x);
						}
					}
					else {
						for (m = base, mend = m2 = m+HALF_SIZE; m<mend; )
						{
							rngstep32( a<<13, a, b, base, m, m2, r, x);
							rngstep32( a>> 6, a, b, base, m, m2, r, x);
						}
						for (m2 = base; m2<mend; )
						{
							rngstep32( a<< 2, a, b, base, m, m2, r, x);
							rngstep32( a>>16, a, b, base, m, m2, r, x);
						}
					}
					left = MASK;
					return table[left];
				}
				std::string isaac32_varqual::get_name() const {
					std::ostringstream tmp;
					int table_size = 1<<table_size_L2;
					tmp << "isaac32x" << table_size;
					return tmp.str();
				}
				void isaac32_varqual::walk_state(StateWalkingObject *walker) {
					walker->handle(a);walker->handle(b);walker->handle(c);
					unsigned long table_size = 1<<table_size_L2;
					for (unsigned long i = 0; i < table_size*2; i++) walker->handle(table[i]);
					walker->handle(left);
					if (left >= table_size) left = 0;
				}
				isaac32_varqual::isaac32_varqual(int table_size_L2_) : table_size_L2(table_size_L2_) {
					if (table_size_L2 < 2) PractRand::issue_error("invalid table size for isaac32_small");
					table = new Uint32[2 << table_size_L2];
				}
				isaac32_varqual::~isaac32_varqual() {delete[] table;}
				#undef ind32
				#undef rngstep32


				
				#define ind16(mm,x)  (*(Uint16 *)((Uint8 *)(mm) + ((x) & ((MASK)<<1))))
				#define rngstep16(mix,a,b,mm,m,m2,r,x) \
				{ \
				  x = *m;  \
				  a = (a^(mix)) + *(m2++); \
				  *(m++) = y = ind16(mm,x) + a + b; \
				  *(r++) = b = ind16(mm,y>>table_size_L2) + x; \
				}
				Uint16 isaac16_varqual::raw16() {
					if (left) {
						return table[--left];
					}
					const int HALF_SIZE = 1<<(table_size_L2-1);
					const int MASK = (1<<table_size_L2)-1;
					Uint16 *base = &table[MASK+1];
					Uint16 *m, *m2, *mend, *r;
					Uint16 x, y;
					m = base;
					r = table;
					b += ++c;
					if (table_size_L2 != 2) {
						for (m = base, mend = m2 = m+HALF_SIZE; m<mend; )
						{
							//13, 6, 2, 16 -> 7, 3, 2, 5
							rngstep16( a<<7, a, b, base, m, m2, r, x);
							rngstep16( a>>3, a, b, base, m, m2, r, x);
							rngstep16( a<<2, a, b, base, m, m2, r, x);
							rngstep16( a>>5, a, b, base, m, m2, r, x);
						}
						for (m2 = base; m2<mend; )
						{
							rngstep16( a<<7, a, b, base, m, m2, r, x);
							rngstep16( a>>3, a, b, base, m, m2, r, x);
							rngstep16( a<<2, a, b, base, m, m2, r, x);
							rngstep16( a>>5, a, b, base, m, m2, r, x);
						}
					}
					else {
						for (m = base, mend = m2 = m+HALF_SIZE; m<mend; )
						{
							rngstep16( a<<7, a, b, base, m, m2, r, x);
							rngstep16( a>>3 , a, b, base, m, m2, r, x);
						}
						for (m2 = base; m2<mend; )
						{
							rngstep16( a<<2, a, b, base, m, m2, r, x);
							rngstep16( a>>5, a, b, base, m, m2, r, x);
						}
					}
					left = MASK;
					return table[left];
				}
				std::string isaac16_varqual::get_name() const {
					std::ostringstream tmp;
					int table_size = 1<<table_size_L2;
					tmp << "isaac16x" << table_size;
					return tmp.str();
				}
				void isaac16_varqual::walk_state(StateWalkingObject *walker) {
					walker->handle(a);walker->handle(b);walker->handle(c);
					unsigned long table_size = 1<<table_size_L2;
					for (unsigned long i = 0; i < table_size*2; i++) walker->handle(table[i]);
					walker->handle(left);
					if (left >= table_size) left = 0;
				}
				isaac16_varqual::isaac16_varqual(int table_size_L2_) : table_size_L2(table_size_L2_) {
					if (table_size_L2 < 2) PractRand::issue_error("invalid table size for isaac16_small");
					table = new Uint16[2 << table_size_L2];
				}
				isaac16_varqual::~isaac16_varqual() {delete[] table;}
				#undef ind16
				#undef rngstep16


				Uint8 efiix8_varqual::raw8() {
					/*Uint8 iterated = iteration_table  [i & iteration_table_size_m1];
					Uint8 indirect = indirection_table[c & indirection_table_size_m1];
					indirection_table[c & indirection_table_size_m1] = iterated + a;
					iteration_table  [i & iteration_table_size_m1  ] = indirect;
					Uint8 old = a ^ b;
					a = b + i;
					b = c + indirect;
					c = old + rotate8(c, 3);
					i++;
					return b ^ iterated;//*/

					//1+1: , 1+2: 38-39, 2+2: 38, 1+4: ?, 2+4: 41-42, 
					/*Uint8 iterated = iteration_table  [i & iteration_table_size_m1  ];
					Uint8 indirect = indirection_table[c & indirection_table_size_m1];
					indirection_table[c & indirection_table_size_m1] = iterated ^ a;
					iteration_table  [i & iteration_table_size_m1  ] = indirect;
					Uint8 old = a + i++;
					a = b + iterated;
					b = c ^ indirect;
					c = old + rotate( c, 3 );
					return b;//*/
					
					//"^b" - 1+1: 38, 1+2: >36
					//"^a" - 1+1: 38
					Uint8 iterated = iteration_table  [i & iteration_table_size_m1  ] ^ a;
					Uint8 indirect = indirection_table[c & indirection_table_size_m1] + i;
					indirection_table[c & indirection_table_size_m1] = iterated;
					iteration_table  [i & iteration_table_size_m1  ] = indirect;
					Uint8 old = a + b;
					a = b + iterated;
					b = c + indirect;
					c = old ^ rotate( c, 3 );
					i++; return old;//*/

					//1+1: 36?, 2+2: 25, 4+4: 33, 8+8: 35, 
					/*Uint8 iterated = iteration_table  [i & iteration_table_size_m1  ] ^ i;
					Uint8 indirect = indirection_table[c & indirection_table_size_m1] + a;
					indirection_table[c & indirection_table_size_m1] = iterated;
					iteration_table  [i & iteration_table_size_m1  ] = indirect;
					Uint8 old = a ^ b;
					a = b + indirect;
					b = c + iterated;
					c = old + rotate( c, 3 );
					return b;//*/
				}
				void efiix8_varqual::walk_state(StateWalkingObject *walker) {
					walker->handle(a); walker->handle(b); walker->handle(c); walker->handle(i);
					for (int x = 0; x <= iteration_table_size_m1  ; x++) walker->handle(iteration_table[  x]);
					for (int x = 0; x <= indirection_table_size_m1; x++) walker->handle(indirection_table[x]);
				}
				std::string efiix8_varqual::get_name() const {
					std::ostringstream tmp;
					tmp << "efiix8x(" << (iteration_table_size_m1+1) << "+" << (indirection_table_size_m1+1) << ")";
					return tmp.str();
				}
				efiix8_varqual::efiix8_varqual(int iteration_table_size_L2, int indirection_table_size_L2) {
					if (iteration_table_size_L2 > 8) PractRand::issue_error("iteration table size log2 too large for efiix8_varqual");
					if (indirection_table_size_L2 > 8) PractRand::issue_error("indirection table size log2 too large for efiix8_varqual");
					int iteration_table_size = 1 << iteration_table_size_L2;
					int indirection_table_size = 1 << indirection_table_size_L2;
					iteration_table_size_m1 = iteration_table_size - 1;
					indirection_table_size_m1 = indirection_table_size - 1;
					iteration_table   = new Uint8[iteration_table_size  ];
					indirection_table = new Uint8[indirection_table_size];
				}
				efiix8_varqual::~efiix8_varqual() {
					delete[] iteration_table;
					delete[] indirection_table;
				}


				Uint8 efiix4_varqual::rotate4(Uint8 value, int bits) {
					value &= 15;
					return (value << bits) | (value >> (4-bits));
				}
				Uint8 efiix4_varqual::raw4() {
					/*Uint8 iterated = iteration_table  [i & iteration_table_size_m1];
					Uint8 indirect = indirection_table[c & indirection_table_size_m1];
					indirection_table[c & indirection_table_size_m1] = iterated + a;
					iteration_table  [i & iteration_table_size_m1  ] = indirect;
					Uint8 old = a ^ b;
					a = b + i;
					b = c + indirect;
					c = old + rotate4(c, 2);
					i++;
					return b ^ iterated;//*/

					//8 - 1+1: , 1+2: 38-39, 2+2: 38, 1+4: ?, 2+4: 41-42, 
					/*Uint8 iterated = iteration_table  [i & iteration_table_size_m1  ];
					Uint8 indirect = indirection_table[c & indirection_table_size_m1];
					indirection_table[c & indirection_table_size_m1] = iterated ^ a;
					iteration_table  [i & iteration_table_size_m1  ] = indirect;
					Uint8 old = a + i++;
					a = b + iterated;
					b = c ^ indirect;
					c = old + rotate4( c, 2 );
					return b;//*/
					
					//8 "^b" - 1+1: 38, 1+2: >36
					/*
					"^b" / "^a" on iterated, at 4 bit, shift 3
								1		2		4		8		16
						1		19/21	23/26	24/28	27/29	31/31
						2		26/23	29/29	30/30	32/31	34/35
						4		31/32	32/33	34/36	36/36	
						8		39/42-43
					"^b" / "^a" on iterated, at 4 bit, shift 2
								1		2		4		8		16
						1		19/21	23/18	27/23	29/28	32/31
						2		23/26	24/24	28/28	29/29	32/
						4		32/26	29/30	33/34	34/36
						8		34/34	36/
					"^b" / "^a" on iterated, at 4 bit, shift 1
								1		2		4		8		16
						1		21/21	26/26	26/26	30/29	31/31
						2		25/26	28/28	29/28	32/31	34/35
						4		32/32	31/33	33/36	36/37
						8		37/42?
					*/
					Uint8 iterated = iteration_table  [i & iteration_table_size_m1  ] ^ a;
					Uint8 indirect = indirection_table[c & indirection_table_size_m1] + i;
					indirection_table[c & indirection_table_size_m1] = iterated;
					iteration_table  [i & iteration_table_size_m1  ] = indirect;
					Uint8 old = a + b;
					a = b + iterated;
					b = c + indirect;
					c = old ^ rotate4( c, 3 );
					i++; return old;//*/

					//8 - 1+1: 36?, 2+2: 25, 4+4: 33, 8+8: 35, 
					/*Uint8 iterated = iteration_table  [i & iteration_table_size_m1  ] ^ i;
					Uint8 indirect = indirection_table[c & indirection_table_size_m1] + a;
					indirection_table[c & indirection_table_size_m1] = iterated;
					iteration_table  [i & iteration_table_size_m1  ] = indirect;
					Uint8 old = a ^ b;
					a = b + indirect;
					b = c + iterated;
					c = old + rotate4( c, 2 );
					return b;//*/
				}
				Uint8 efiix4_varqual::raw8() {
					Uint8 rv = raw4() & 15; return rv | ((raw4() & 15) << 4);
				}
				void efiix4_varqual::walk_state(StateWalkingObject *walker) {
					walker->handle(a); walker->handle(b); walker->handle(c); walker->handle(i);
					for (int x = 0; x <= iteration_table_size_m1  ; x++) walker->handle(iteration_table[  x]);
					for (int x = 0; x <= indirection_table_size_m1; x++) walker->handle(indirection_table[x]);
				}
				std::string efiix4_varqual::get_name() const {
					std::ostringstream tmp;
					tmp << "efiix4x(" << (iteration_table_size_m1+1) << "+" << (indirection_table_size_m1+1) << ")";
					return tmp.str();
				}
				efiix4_varqual::efiix4_varqual(int iteration_table_size_L2, int indirection_table_size_L2) {
					if (iteration_table_size_L2 > 4) PractRand::issue_error("iteration table size log2 too large for efiix4_varqual");
					if (indirection_table_size_L2 > 4) PractRand::issue_error("indirection table size log2 too large for efiix4_varqual");
					int iteration_table_size = 1 << iteration_table_size_L2;
					int indirection_table_size = 1 << indirection_table_size_L2;
					iteration_table_size_m1 = iteration_table_size - 1;
					indirection_table_size_m1 = indirection_table_size - 1;
					iteration_table   = new Uint8[iteration_table_size  ];
					indirection_table = new Uint8[indirection_table_size];
				}
				efiix4_varqual::~efiix4_varqual() {
					delete[] iteration_table;
					delete[] indirection_table;
				}
			}//NotRecommended
		}//Polymorphic
	}//RNGs
}//PractRand
