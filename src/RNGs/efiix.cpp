#include <cstring>
#include <string>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"

#include "PractRand/RNGs/efiix8x384.h"
#include "PractRand/RNGs/efiix16x384.h"
#include "PractRand/RNGs/efiix32x384.h"
#include "PractRand/RNGs/efiix64x384.h"

#include "PractRand/RNGs/arbee.h"
//#include "PractRand/RNGs/trivium.h"

using namespace PractRand;
using namespace PractRand::Internals;

PRACTRAND__POLYMORPHIC_RNG_BASICS_C8(efiix8x384)
void PractRand::RNGs::Polymorphic::efiix8x384::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::efiix8x384::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) {implementation.seed(s1,s2,s3,s4);}
std::string PractRand::RNGs::Polymorphic::efiix8x384::get_name() const {return "efiix8x384";}
PRACTRAND__POLYMORPHIC_RNG_BASICS_C16(efiix16x384)
void PractRand::RNGs::Polymorphic::efiix16x384::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::efiix16x384::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) {implementation.seed(s1,s2,s3,s4);}
std::string PractRand::RNGs::Polymorphic::efiix16x384::get_name() const {return "efiix16x384";}
PRACTRAND__POLYMORPHIC_RNG_BASICS_C32(efiix32x384)
void PractRand::RNGs::Polymorphic::efiix32x384::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::efiix32x384::seed(vRNG *source_rng) {implementation.seed(source_rng);}
//void PractRand::RNGs::Polymorphic::efiix32x384::seed(const Uint32 *seeds, int num_seeds, int quality) {implementation.seed(seeds, num_seeds, quality);}
//void PractRand::RNGs::Polymorphic::efiix32x384::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) {implementation.seed(s1,s2,s3,s4);}
std::string PractRand::RNGs::Polymorphic::efiix32x384::get_name() const {return "efiix32x384";}
PRACTRAND__POLYMORPHIC_RNG_BASICS_C64(efiix64x384)
void PractRand::RNGs::Polymorphic::efiix64x384::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::efiix64x384::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) {implementation.seed(s1,s2,s3,s4);}
std::string PractRand::RNGs::Polymorphic::efiix64x384::get_name() const {return "efiix64x384";}

#define EFIIX_ALGORITHM(BITS, SHIFT_AMOUNT) \
	Word iterated = iteration_table  [i % ITERATION_SIZE];\
	Word indirect = indirection_table[c % INDIRECTION_SIZE];\
	indirection_table[c % INDIRECTION_SIZE] = iterated + a;\
	iteration_table  [i % ITERATION_SIZE  ] = indirect;\
	Word old = a ^ b;\
	a = b + i++;\
	b = c + indirect;\
	c = old + rotate ## BITS (c, SHIFT_AMOUNT);\
	return b ^ iterated;
//current algorithm
//perfect statistically (unless ITERATION_SIZE and word size are both very small), reasonably fast
//safe from a variety of attacks
//but the "b ^ iterated" and "iterated + a" have too much in common, and the later can come right back out of the indirection pool right away
//not much weakness, but some
//alternatively, can return "b" alone - it's horrible at 8/16 bit, but acceptable at 32 and good at 64


//#define EFIIX_ALGORITHM(BITS, SHIFT_AMOUNT) \
	Word iterated = iteration_table  [i % ITERATION_SIZE];\
	Word indirect = indirection_table[c % INDIRECTION_SIZE];\
	indirection_table[c % INDIRECTION_SIZE] = iterated ^ a;\
	iteration_table  [i % ITERATION_SIZE  ] = indirect;\
	Word old = a + i++;\
	a = b + iterated;\
	b = c ^ indirect;\
	c = old + rotate ## BITS (c, SHIFT_AMOUNT);\
	return b;
//adequate statistically, good speed
//wider connection between mixing pool (a,b,c) and indirection pool (the two arrays) means it's now safe to use a simpler output function
//...I think, anyway
//"old" now being a function of (a,i) instead of (a,b) means worse mixing in the pool, I think


//#define EFIIX_ALGORITHM(BITS, SHIFT_AMOUNT) \
	Word iterated = iteration_table  [i % ITERATION_SIZE] ^ i;\
	Word indirect = indirection_table[c % INDIRECTION_SIZE] + a;\
	indirection_table[c % INDIRECTION_SIZE] = iterated;\
	iteration_table  [i % ITERATION_SIZE  ] = indirect;\
	Word old = a^b;\
	a = b + indirect;\
	b = c + iterated;\
	c = old + rotate ## BITS (c, SHIFT_AMOUNT);\
	i++;\
	return b;
//not good enough statistically (8x1+1: 128 GB, 8x2+2: 32 MB, 8x4+4: 8 GB, 8x8+8: 32 GB), adequate speed
//looks like the above one but better mixing in the pool?


//#define EFIIX_ALGORITHM(BITS, SHIFT_AMOUNT) \
	Word iterated = iteration_table  [i % ITERATION_SIZE] ^ a;\
	Word indirect = indirection_table[c % INDIRECTION_SIZE] + i;\
	indirection_table[c % INDIRECTION_SIZE] = iterated;\
	iteration_table  [i % ITERATION_SIZE  ] = indirect;\
	Word old = a + b;\
	a = b + iterated;\
	b = c + indirect;\
	c = old ^ rotate ## BITS (c, SHIFT_AMOUNT);\
	i++; return old;
//decent statistically, decent speed
//wider connection between mixing pool (a,b,c) and indirection pool (the two arrays) means it's now safe to use a simpler output function
//...I think, anyway


/*
	RNG             Quality     PractRand   TestU01     RaBiGeTe    Dieharder  Dieharder
	                subscores   standard    *Crush      Extended*1  -a         custom
*	efiix4x(2+2)     0/0/0/0    512 MB      2/?/?       pass(b)
*	efiix4x(1+4)     0/0/0/0    128 MB      3/?/?       128 Mb
*	efiix4x(4+1)     1/0/0/0    8 GB        0/0/12      pass
*	efiix4x(2+4)     0/0/1/0    4 GB        2/?/?       pass?
*	efiix4x(4+2)     2/0/1/0    16 GB       pass        pass?
*	efiix4x(4+4)     1/1/1/1    256 GB      0/1/2       pass?
*	efiix4x(1+8)     1/1/1/1    1 GB        0/14/?      512 Mb
*	efiix4x(8+1)     5/1/1/1    > 1 TB      > 1 TB      pass        pass?
*	efiix4x(2+8)     ?/1/1/1    ?           ?           0/13/?      pass?
*	efiix4x(1+16)    ?/3/2/1    ?           ?           0/1/6       pass
---
*	efiix8x(1+1)     4/1/1/1    > 8 TB      > 4 TB      pass        pass?
*	efiix8x(1+2)     5/1/1/1    > 2 TB      > 1 TB      pass        pass?
*	efiix8x(2+1)     ?/1/1/1                
*	efiix8x(2+2)     ?/2/2/2    

*	efiix4x(2+2)     0/0/0/0    2 GB*       256 MB      2/?/?       pass
*	efiix4x(1+4)     0/0/0/0    1 GB*       64 MB       3/?/?       128 Mb
*	efiix4x(4+1)     1/0/0/0    16 GB(pe)*  4 GB        0/0/12      pass
*	efiix4x(2+4)     0/0/1/0    16 GB*      4 GB        2/?/?       pass?
*	efiix4x(4+2)     2/0/1/0    256 GB*     16 GB       pass        pass?
*	efiix4x(4+4)     1/1/1/1    2 TB*       256 GB      0/1/2       pass?
*	efiix8x(1+1)     4/1/1/1    > 8 TB*     > 4 TB      pass        pass?


It looks like quality is suffering with 4 bit words... 
	one possibly is how few shift constants are possible... maybe they all suck

Some oddities related to empirical quality vs parameterization visible there at 4 bit
	impossible to tell if they remain at 8 bit


*/


/*	on the algorithm:
		examining small windows of the output is never sufficient to learn anything significant, 
			due to relatively large FIFO "iteration_table"
		examining small windows at a stride of ITERATION_SIZE apart might be sufficient, but...
			if the indirection pattern is not predicted/guessed that is meaningless
			if the indirection pattern is predicted/guessed, 
				if it doesn't reach back ITERATION_SIZE elements
					that will simply add more unknowns
				if it does reach back ITERATION_SIZE elements
					you still need to establish the contents of (a,b,c)
					and besides that's an enormous amount of indirection pattern to figure out
		figuring out any significant portion of the indirection pattern should be very very difficult
	consider:
		if we guess (a,b,c,i) and "indirect" correctly (a LOT of correct guesses)
		then we can use all that to calculate "iterated" from the output
		and then use all that plus a guess that the indirection index is identical
			(which isn't exactly a guess since that's a function of other values we guessed, but it doesn't happen all that often)
		to figure out all the same information at the next position
		it might look like we have it almost cracked - we can predict important future (partial) states with relatively high probabilities
		but that's useless if we can't TELL if our guesses were right
		1. 
			which we can't do from a short output window
				because we need the output to figure out "iterated" values feeding in
			so we keep guessing that the indirection index remains constant, ITERATIONS_SIZE times in a row
			THEN we can start to confirm our guesses
			but that's already on the order of 2**(W*5+ITERATION_SIZE*L) operations to get a success
			W is word size, in bits; L is log2(INDIRECTION_SIZE)
		2. 
			which we can't do from a single short output window
			...so we have to skip forward roughly ITERATION_SIZE positions to a 2nd window
			and do the whole thing over again (but we don't have to guess "i" again)
			and use the old predicted values of "indirect" fed in to the iteration table
			to check the new predicted values of "iterated"
			and then we can START confirming our guesses
			at region1 we guessed (a,b,c) and "i" and "indirect"
			at region2 we guessed (a,b,c) and "indirect"
		3. 
			we check if the "iterated" values observed actually match the constant-indirection-index required
			but even if they appear to, that doesn't do us much good, as it's still far more likely to be a false positive than a true positive
			unless they appear to for a very long sequence... but the chances of such a region even existing in a cyphertext length <2**64 is very small
	overall, I think the 32 bit word variant ought to be good for 128 bit security, and the 64 bit word variant ought to be good for 160 bit security
	I don't have much confidence in the 8 or 16 bit variants though
*/

#define EFIIX_SEED3(WORD_BITS, SEEDING_ITER, SEEDING_IND) 

/*class efiix8_seeding_helper {
public:
	typedef Uint8 Word;
	Word *iter_table;
	Word *ind_table;
	Word a, b, c, i;
	int iter_size;
	int ind_size;
	;
};*/

#define EFIIX_SEED2(WORD_BITS) \
	static const Word seeding_constants[8] = {/*pi in hex*/ \
		Word(0x3243F6A8885A308Dull),\
		Word(0x313198A2E0370734ull),\
		Word(0x4A4093822299F31Dull),\
		Word(0x0082EFA90082EFA9ull),\
		Word(0x8EC4E6C89452821Eull),\
		Word(0x638D01377BE5466Cull),\
		Word(0xF34E90C6CC0AC29Bull),\
		Word(0x7C97C50DD3F84D5Bull)\
	};\
	enum {SIMD_WIDTH=4,SIMD_WORDS=6,MIX_WORDS=SIMD_WIDTH*SIMD_WORDS,IN_WORDS=16,CONST_WORDS=MIX_WORDS-IN_WORDS};\
	Word mixing_pool[MIX_WORDS];\
	if (true) {/*first filling of mixing pool*/ \
		int i, n = num_seeds > IN_WORDS ? IN_WORDS : num_seeds;\
		for (i = 0; i < n; i++) mixing_pool[i] = *(seeds++);\
		for (; i < IN_WORDS; i++) mixing_pool[i] = 0;\
		for (i = 0; i < CONST_WORDS; i++) mixing_pool[IN_WORDS+i] = Word(seeding_constants[i]);\
	}\
	while (true) {\
		Word mixing_pool_copy[MIX_WORDS];\
		for (int i = 0; i < MIX_WORDS; i++) mixing_pool_copy[i] = mixing_pool[i];\
		for (int round = 0; round < seeding_quality; round++) {\
			/*inspirired by ChaCha, intended for SIMD implementation \
			but where ChaCha is based upon an arrangement of 4 4x32 SIMD registers in something like a 4x4 matrix \
			this is based upon an arrangement of 6 NxM (N must be a power of 2, M is normal int/long size) SIMD registers \
			while ChaCha has no need of additional SIMD registers for temporary values (except for the rotates on some platforms), this does*/ \
			if (round) {/*needed for diffusion between different quadrants of SIMD words, but pointless in round 0*/ \
				for (int delta_L2 = 0; 1<<delta_L2 < SIMD_WIDTH; delta_L2++) for (int x = 0; x < SIMD_WIDTH; x++)\
					mixing_pool[x + SIMD_WIDTH*delta_L2] ^= mixing_pool[((x+(1<<delta_L2))&(SIMD_WIDTH-1)) + SIMD_WIDTH*(SIMD_WORDS-1-delta_L2)];\
			}\
			for (int x = 0; x < SIMD_WIDTH; x++) {\
				Word &a = mixing_pool[x + SIMD_WIDTH*0]; Word &b = mixing_pool[x + SIMD_WIDTH*1];\
				Word &c = mixing_pool[x + SIMD_WIDTH*2]; Word &d = mixing_pool[x + SIMD_WIDTH*3];\
				Word &e = mixing_pool[x + SIMD_WIDTH*4]; Word &f = mixing_pool[x + SIMD_WIDTH*5];\
				/*shifts: 8   16  32   64 \
				+<<       2   3   7    13 \
				<<<       2   4   8    16 \
				^>>       3   5   11   21 \
				+<<       1   2   5    9  \
				^>>       2   3   6    11 \
				<<<       4   8   16   32 \
				that's + <<< ^ + ^ <<<     */ \
				\
				b += a ^ d;\
				a += a << ((WORD_BITS+3)/5);\
				\
				c += b ^ e;\
				b = rotate(b, WORD_BITS/4);\
				c ^= a;\
				\
				d ^= c + f;\
				c ^= c >> ((WORD_BITS+1)/3);\
				d += b;\
				\
				e += d ^ a;\
				d += d << ((WORD_BITS+3)/7);\
				e ^= c;\
				\
				f ^= e + b;\
				e ^= e >> ((WORD_BITS+4)/6);\
				f += d;\
				\
				a += f ^ c;\
				f = rotate(f, WORD_BITS/2);\
				a ^= e;\
				b += f;\
			}\
		}\
		for (int i = 0; i < MIX_WORDS; i++) mixing_pool[i] += mixing_pool_copy[i];\
		if (num_seeds < IN_WORDS) break;\
		/*additional filling of mixing pool*/ \
		num_seeds -= IN_WORDS;\
		int i, n = num_seeds > IN_WORDS ? IN_WORDS : num_seeds;\
		for (i = 0; i < n; i++) mixing_pool[i] ^= *(seeds++);\
		for (i = 0; i < CONST_WORDS; i++) mixing_pool[IN_WORDS+i] ^= Word(seeding_constants[i]);\
	}\
	\
	int mi = 0;\
	a = mixing_pool[mi++];\
	b = mixing_pool[mi++];\
	c = mixing_pool[mi++];\
	i = mixing_pool[mi++];\
	if (ITERATION_SIZE + INDIRECTION_SIZE + 4 <= MIX_WORDS) {\
		/*seed directly from mixing pool*/ \
		for (int x = 0; x < ITERATION_SIZE; x++) iteration_table[x] = mixing_pool[mi++];\
		for (int x = 0; x < INDIRECTION_SIZE; x++) iteration_table[x] = mixing_pool[mi++];\
	}\
	else {\
		/*seed a smaller EFIIX from mixing pool, then seed from that*/ \
		/*based upon the assumption that MIX_WORDS==24, otherwise the constants here need to be adjusted*/ \
		enum {LITER_SIZE = 16, LIND_SIZE = 4};\
		Word *local_iter_table = &mixing_pool[4];\
		Word *local_ind_table = &mixing_pool[4+LITER_SIZE];\
		for (int x = 0; x < ITERATION_SIZE + INDIRECTION_SIZE; x++) {\
			Uint8 iterated = local_iter_table[i % LITER_SIZE];\
			Uint8 indirect = local_ind_table [c % LIND_SIZE ];\
			local_ind_table [c % LIND_SIZE ] = iterated + a;\
			local_iter_table[i % LITER_SIZE] = indirect;\
			Uint8 old = a ^ b;\
			a = b + i++;\
			b = c + indirect;\
			c = old + rotate(c, 13);\
			if (x < ITERATION_SIZE)\
				iteration_table[x] =  b ^ iterated;\
			else indirection_table[x-ITERATION_SIZE] =  b ^ iterated;\
		}\
		for (int x = 16 * (seeding_quality-1); x > 0; x--) raw ## WORD_BITS();\
	}


#define EFIIX_SEED(BITS) \
	PractRand::RNGs::Raw::arbee seeder;\
	seeder.seed(s1, s2, s3, s4);\
	for (unsigned long w=0; w < INDIRECTION_SIZE; w++) indirection_table[w] = Word(seeder.raw64());\
	i = Word(seeder.raw64());\
	for (unsigned long w=0; w < ITERATION_SIZE  ; w++) iteration_table  [(w+i)%ITERATION_SIZE] = Word(seeder.raw64());\
	a = Word(seeder.raw64());\
	b = Word(seeder.raw64());\
	c = Word(seeder.raw64());\
	for (unsigned long w=0; w < 64; w++) raw ## BITS();\
	seeder.raw64(); s1 += seeder.raw64(); s2 += seeder.raw64(); s3 += seeder.raw64();\
	seeder.seed(s1^a, s2^b, s3^c, ~s4);\
	for (unsigned long w=0; w < INDIRECTION_SIZE; w++) indirection_table[w] ^= Word(seeder.raw64());\
	for (unsigned long w=0; w < ITERATION_SIZE+16; w++) raw ## BITS();
/*	on seeding:
		two different seedings of seeder are needed because:
			the first one gets things to a nice state, but might not be sufficiently secure
			simply skipping some output makes part of the state secure, but...
				parts of indirection_table remain unchanged no matter how much output is skipped
		the second seed to seeder uses
			(a,b,c) because it needs a function of the initial seed that is too complex for attacks
				the 64 steps taken before that make (a,b,c) very complex because of the indirection
			~s4 to guarantee that it can never be the same seed as was used in the first seeding
			a function of the first seeders final state
				because (a,b,c) may not be enough bits on smaller word sizes
				not sure that really helps much though
*/


PractRand::RNGs::Raw::efiix8x384::~efiix8x384() {std::memset(this, 0, sizeof(this));}
PractRand::RNGs::Raw::efiix16x384::~efiix16x384() {std::memset(this, 0, sizeof(this));}
PractRand::RNGs::Raw::efiix32x384::~efiix32x384() {std::memset(this, 0, sizeof(this));}
PractRand::RNGs::Raw::efiix64x384::~efiix64x384() {std::memset(this, 0, sizeof(this));}


Uint8 PractRand::RNGs::Raw::efiix8x384::raw8() {
	typedef Word check_efiix_array_sizes[(ITERATION_SIZE & (ITERATION_SIZE-1)) || (INDIRECTION_SIZE & (INDIRECTION_SIZE-1)) ? -1 : 1];
	EFIIX_ALGORITHM(8, 3)
}
void PractRand::RNGs::Raw::efiix8x384::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) {
	EFIIX_SEED( 8 )
}
void PractRand::RNGs::Raw::efiix8x384::walk_state(StateWalkingObject *walker) {
	for (unsigned long w = 0; w < ITERATION_SIZE; w++) walker->handle(iteration_table[w]);
	for (unsigned long w = 0; w < INDIRECTION_SIZE; w++) walker->handle(indirection_table[w]);
	walker->handle(i);
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
}


Uint16 PractRand::RNGs::Raw::efiix16x384::raw16() {
	typedef Word check_efiix_array_sizes[(ITERATION_SIZE & (ITERATION_SIZE-1)) || (INDIRECTION_SIZE & (INDIRECTION_SIZE-1)) ? -1 : 1];
	EFIIX_ALGORITHM(16, 7)
}
void PractRand::RNGs::Raw::efiix16x384::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) {
	EFIIX_SEED( 16 )
}
void PractRand::RNGs::Raw::efiix16x384::walk_state(StateWalkingObject *walker) {
	for (unsigned long w = 0; w < ITERATION_SIZE; w++) walker->handle(iteration_table[w]);
	for (unsigned long w = 0; w < INDIRECTION_SIZE; w++) walker->handle(indirection_table[w]);
	walker->handle(i);
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
}


Uint32 PractRand::RNGs::Raw::efiix32x384::raw32() {
	typedef Word check_efiix_array_sizes[(ITERATION_SIZE & (ITERATION_SIZE-1)) || (INDIRECTION_SIZE & (INDIRECTION_SIZE-1)) ? -1 : 1];
	EFIIX_ALGORITHM(32, 13)
}
static void mix4x32(Uint32 &a, Uint32 &b, Uint32 &c, Uint32 &d) {
	b ^= a + (a << 13);
	c += b ^ (b >> 5);
	d ^= c + b;
	a += d ^ ((c << 8) | (c >> 24));
	b ^= a + (a << 11);
	c += b ^ (b >> 9);
	d ^= c + b;
	a += d ^ ((c << 8) | (c >> 24));
}
void PractRand::RNGs::Raw::efiix32x384::seed(PractRand::RNGs::vRNG *source_rng) {
	a = source_rng->raw32();
	b = source_rng->raw32();
	c = source_rng->raw32();
	i = source_rng->raw32();
	// number of possible seeded states is kept much smaller than the number of valid states
	// in order to make it extremely unlikely that any bad seeds exist
	// as opposed to how it would be otherwise, in which finding a bad seed would be *extremely* difficult, but a few would likely exist
	for (int x = 0; x < ITERATION_SIZE; x++) if (!(x & 3)) iteration_table[x] = source_rng->raw32(); else iteration_table[x] = iteration_table[x & ~3];
	for (int x = 0; x < INDIRECTION_SIZE; x++) if (!(x & 3)) indirection_table[x] = source_rng->raw32(); else indirection_table[x] = iteration_table[x & ~3];

	//ought to be a secure seeding if source_rng->get_flags() includes FLAG::CRYPTOGRAPHIC_SECURITY, so...
	for (int x = 0; x < ITERATION_SIZE*2+INDIRECTION_SIZE; x++) raw32();
}
void PractRand::RNGs::Raw::efiix32x384::seed(const Word *seeds, int num_seeds, int seeding_quality) {
	/*long ss = 0;
	if (num_seeds) {
		int si = num_seeds;
		a = seeds[--si]; si = si ? si : num_seeds;
		b = seeds[--si]; si = si ? si : num_seeds;
		c = seeds[--si]; si = si ? si : num_seeds;
		i = seeds[--si]; si = si ? si : num_seeds;
		if (num_seeds > 4) {
			ss = num_seeds - 5;
			ss |= ss >> 1; ss |= ss >> 2; ss |= ss >> 4; ss |= ss >> 8; ss |= ss >> 16;
			ss += 1;
			if (ss > INDIRECTION_SIZE) ss = INDIRECTION_SIZE;
			if (ss > ITERATION_SIZE) ss = ITERATION_SIZE;
			for (int x = 0; x <= ss; x++) {iteration_table[x] = seeds[--si]; si = si ? si : num_seeds;}
			for (int x = 0; x <= ss; x++) {indirection_table[x] = seeds[--si]; si = si ? si : num_seeds;}
		}
		else {iteration_table[0] = indirection_table[0] = 0;}
	}
	else a = b = c = i = iteration_table[0] = indirection_table[0] = 1;

	enum {MIN_SIZE = INDIRECTION_SIZE < ITERATION_SIZE ? INDIRECTION_SIZE : ITERATION_SIZE };
	for (int x = 0; x < INDIRECTION_SIZE*2 + ITERATION_SIZE + num_seeds

	for (int x = 0; x < SEEDING_ITER; x++) {seeding_ind[x] = seeds[si++]; if (si==num_seeds) si=0;}\
	for (int x = 0; x < SEEDING_IND; x++) seeding_iter[x] = x;\
	a = b = c = i;\
	for (int x = SEEDING_ITER*3 + SEEDING_IND*2 + num_seeds; x > 0; x--) {\
	*/

	EFIIX_SEED2( 32 )

	/*static const Word seeding_constants[8] = {//pi in hex
		Word(0x3243F6A8885A308Dull), 
		Word(0x313198A2E0370734ull), 
		Word(0x4A4093822299F31Dull), 
		Word(0x0082EFA90082EFA9ull),
		Word(0x8EC4E6C89452821Eull),
		Word(0x638D01377BE5466Cull),
		Word(0xF34E90C6CC0AC29Bull),
		Word(0x7C97C50DD3F84D5Bull)
	};
	enum {WORD_BITS = sizeof(Word) * 8,SIMD_WIDTH=4,SIMD_WORDS=6,MIX_WORDS=SIMD_WIDTH*SIMD_WORDS,IN_WORDS=16,CONST_WORDS=MIX_WORDS-IN_WORDS};
	Word mixing_pool[MIX_WORDS];
	if (true) {//first filling of mixing pool
		int i, n = num_seeds > IN_WORDS ? IN_WORDS : num_seeds;
		for (i = 0; i < n; i++) mixing_pool[i] = *(seeds++);
		for (; i < IN_WORDS; i++) mixing_pool[i] = 0;
		for (i = 0; i < CONST_WORDS; i++) mixing_pool[IN_WORDS+i] = Word(seeding_constants[i]);
	}
	while (true) {
		Word mixing_pool_copy[MIX_WORDS];
		for (int i = 0; i < MIX_WORDS; i++) mixing_pool_copy[i] = mixing_pool[i];
		for (int round = 0; round < seeding_quality; round++) {
			//inspirired by ChaCha, intended for SIMD implementation
			//but where ChaCha is based upon an arrangement of 4 4x32 SIMD registers in something like a 4x4 matrix
			//this is based upon an arrangement of 6 NxM (N must be a power of 2, M is normal int/long size) SIMD registers
			//while ChaCha has no need of additional SIMD registers for temporary values (except for the rotates on some platforms), this does

			//needed for diffusion between different quadrants of SIMD words
			if (round) {//pointless in round 0
				for (int delta_L2 = 0; 1<<delta_L2 < SIMD_WIDTH; delta_L2++) for (int x = 0; x < SIMD_WIDTH; x++) 
					mixing_pool[x + SIMD_WIDTH*delta_L2] ^= mixing_pool[((x+(1<<delta_L2))&(SIMD_WIDTH-1)) + SIMD_WIDTH*(SIMD_WORDS-1-delta_L2)];
			}

			//mix:
			for (int x = 0; x < SIMD_WIDTH; x++) {
				Word &a = mixing_pool[x + SIMD_WIDTH*0]; Word &b = mixing_pool[x + SIMD_WIDTH*1];
				Word &c = mixing_pool[x + SIMD_WIDTH*2]; Word &d = mixing_pool[x + SIMD_WIDTH*3];
				Word &e = mixing_pool[x + SIMD_WIDTH*4]; Word &f = mixing_pool[x + SIMD_WIDTH*5];
				//shifts:   8   16  32   64
				// +<<      2   3   7    13
				//  <<<     2   4   8    16
				// ^>>      3   5   11   21
				// +<<      1   2   5    9
				// ^>>      2   3   6    11
				//  <<<     4   8   16   32

				// + <<< ^ + ^ <<<

				b += a ^ d;
				a += a << ((WORD_BITS+3)/5);

				c += b ^ e;
				b = rotate(b, WORD_BITS/4);
				c ^= a;
				
				d ^= c + f;
				c ^= c >> ((WORD_BITS+1)/3);
				d += b;
				
				e += d ^ a;
				d += d << ((WORD_BITS+3)/7);
				e ^= c;
				
				f ^= e + b;
				e ^= e >> ((WORD_BITS+4)/6);
				f += d;

				a += f ^ c;
				f = rotate(f, WORD_BITS/2);
				a ^= e;
				b += f;
			}//x
		}//rounds
		for (int i = 0; i < MIX_WORDS; i++) mixing_pool[i] += mixing_pool_copy[i];
		if (num_seeds < IN_WORDS) break;
		//additional filling of mixing pool
		num_seeds -= IN_WORDS;
		int i, n = num_seeds > IN_WORDS ? IN_WORDS : num_seeds;
		for (i = 0; i < n; i++) mixing_pool[i] ^= *(seeds++);
		for (i = 0; i < CONST_WORDS; i++) mixing_pool[IN_WORDS+i] ^= Word(seeding_constants[i]);
	}

	int mi = 0;
	a = mixing_pool[mi++];
	b = mixing_pool[mi++];
	c = mixing_pool[mi++];
	i = mixing_pool[mi++];
	if (ITERATION_SIZE + INDIRECTION_SIZE + 4 <= MIX_WORDS) {
		//seed directly from mixing pool
		for (int x = 0; x < ITERATION_SIZE; x++) iteration_table[x] = mixing_pool[mi++];
		for (int x = 0; x < INDIRECTION_SIZE; x++) iteration_table[x] = mixing_pool[mi++];
	}
	else {
		//seed a smaller EFIIX from mixing pool, then seed from that
		//based upon the assumption that MIX_WORDS==24, otherwise the constants here need to be adjusted
		enum {LITER_SIZE = 16, LIND_SIZE = 4};
		Word *local_iter_table = &mixing_pool[4];
		Word *local_ind_table = &mixing_pool[4+LITER_SIZE];
		for (int x = 0; x < ITERATION_SIZE + INDIRECTION_SIZE; x++) {
			Uint8 iterated = local_iter_table[i % LITER_SIZE];
			Uint8 indirect = local_ind_table [c % LIND_SIZE ];
			local_ind_table [c % LIND_SIZE ] = iterated + a;
			local_iter_table[i % LITER_SIZE] = indirect;
			Uint8 old = a ^ b;
			a = b + i++;
			b = c + indirect;
			c = old + rotate(c, 13);
			if (x < ITERATION_SIZE)
				iteration_table[x] =  b ^ iterated;
			else indirection_table[x-ITERATION_SIZE] =  b ^ iterated;
		}
		for (int x = 0; x < 16 * seeding_quality; x++) raw32();
	}*/
}
void PractRand::RNGs::Raw::efiix32x384::walk_state(StateWalkingObject *walker) {
	for (unsigned long w = 0; w < ITERATION_SIZE; w++) walker->handle(iteration_table[w]);
	for (unsigned long w = 0; w < INDIRECTION_SIZE; w++) walker->handle(indirection_table[w]);
	walker->handle(i);
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
}


Uint64 PractRand::RNGs::Raw::efiix64x384::raw64() {
	typedef Word check_efiix_array_sizes[(ITERATION_SIZE & (ITERATION_SIZE-1)) || (INDIRECTION_SIZE & (INDIRECTION_SIZE-1)) ? -1 : 1];
	EFIIX_ALGORITHM(64, 25)
}
void PractRand::RNGs::Raw::efiix64x384::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) {
	EFIIX_SEED( 64 )
}
void PractRand::RNGs::Raw::efiix64x384::walk_state(StateWalkingObject *walker) {
	for (unsigned long w = 0; w < ITERATION_SIZE; w++) walker->handle(iteration_table[w]);
	for (unsigned long w = 0; w < INDIRECTION_SIZE; w++) walker->handle(indirection_table[w]);
	walker->handle(i);
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
}

