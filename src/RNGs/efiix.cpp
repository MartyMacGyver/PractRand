#include <cstring>
#include <string>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"

#include "PractRand/RNGs/efiix8x48.h"
#include "PractRand/RNGs/efiix16x48.h"
#include "PractRand/RNGs/efiix32x48.h"
#include "PractRand/RNGs/efiix64x48.h"

#include "PractRand/RNGs/arbee.h"
//#include "PractRand/RNGs/trivium.h"

using namespace PractRand;
using namespace PractRand::Internals;

PRACTRAND__POLYMORPHIC_RNG_BASICS_C8(efiix8x48)
void PractRand::RNGs::Polymorphic::efiix8x48::seed(Uint64 s) { implementation.seed(s); }
void PractRand::RNGs::Polymorphic::efiix8x48::seed(vRNG *seeder_rng) { implementation.seed(seeder_rng); }
void PractRand::RNGs::Polymorphic::efiix8x48::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) { implementation.seed(s1, s2, s3, s4); }
std::string PractRand::RNGs::Polymorphic::efiix8x48::get_name() const { return "efiix8x48"; }
PRACTRAND__POLYMORPHIC_RNG_BASICS_C16(efiix16x48)
void PractRand::RNGs::Polymorphic::efiix16x48::seed(Uint64 s) { implementation.seed(s); }
void PractRand::RNGs::Polymorphic::efiix16x48::seed(vRNG *seeder_rng) { implementation.seed(seeder_rng); }
void PractRand::RNGs::Polymorphic::efiix16x48::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) { implementation.seed(s1, s2, s3, s4); }
std::string PractRand::RNGs::Polymorphic::efiix16x48::get_name() const { return "efiix16x48"; }
PRACTRAND__POLYMORPHIC_RNG_BASICS_C32(efiix32x48)
void PractRand::RNGs::Polymorphic::efiix32x48::seed(Uint64 s) { implementation.seed(s); }
void PractRand::RNGs::Polymorphic::efiix32x48::seed(vRNG *seeder_rng) { implementation.seed(seeder_rng); }
//void PractRand::RNGs::Polymorphic::efiix32x48::seed(const Uint32 *seeds, int num_seeds, int quality) {implementation.seed(seeds, num_seeds, quality);}
void PractRand::RNGs::Polymorphic::efiix32x48::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) { implementation.seed(s1, s2, s3, s4); }
std::string PractRand::RNGs::Polymorphic::efiix32x48::get_name() const { return "efiix32x48"; }
PRACTRAND__POLYMORPHIC_RNG_BASICS_C64(efiix64x48)
void PractRand::RNGs::Polymorphic::efiix64x48::seed(Uint64 s) { implementation.seed(s); }
void PractRand::RNGs::Polymorphic::efiix64x48::seed(vRNG *seeder_rng) { implementation.seed(seeder_rng); }
void PractRand::RNGs::Polymorphic::efiix64x48::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) { implementation.seed(s1, s2, s3, s4); }
std::string PractRand::RNGs::Polymorphic::efiix64x48::get_name() const { return "efiix64x48"; }

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
//but directly returning "old" means that if they guess (a,b,c) correctly once, they can figure out all other (a,b,c) sets from looking at the output


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


PractRand::RNGs::Raw::efiix8x48::~efiix8x48() { std::memset(this, 0, sizeof(this)); }
PractRand::RNGs::Raw::efiix16x48::~efiix16x48() { std::memset(this, 0, sizeof(this)); }
PractRand::RNGs::Raw::efiix32x48::~efiix32x48() { std::memset(this, 0, sizeof(this)); }
PractRand::RNGs::Raw::efiix64x48::~efiix64x48() { std::memset(this, 0, sizeof(this)); }


Uint8 PractRand::RNGs::Raw::efiix8x48::raw8() {
	typedef Word check_efiix_array_sizes[(ITERATION_SIZE & (ITERATION_SIZE-1)) || (INDIRECTION_SIZE & (INDIRECTION_SIZE-1)) ? -1 : 1];
	EFIIX_ALGORITHM(8, 3)
}
void PractRand::RNGs::Raw::efiix8x48::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) {
	//EFIIX_SEED(8)
	a = b = c = i = 0;
	iteration_table[0] = 0;
	indirection_table[0] = 0;
	//for (int x = 0; x < INDIRECTION_SIZE; x++) indirection_table[x] = x;
	for (int x = 0; x < 1 + (64 / (8 * sizeof(Word))); x++) {
		a += Word(s1); b += Word(s2); c += Word(s3); iteration_table[0] += Word(s4);
		s1 >>= 8 * sizeof(Word); s2 >>= 8 * sizeof(Word); s3 >>= sizeof(Word); s4 >>= sizeof(Word);
		enum { LESSER_SIZE = ITERATION_SIZE > INDIRECTION_SIZE ? INDIRECTION_SIZE : ITERATION_SIZE };
		enum { GREATER_SIZE = ITERATION_SIZE > INDIRECTION_SIZE ? ITERATION_SIZE : INDIRECTION_SIZE };
		if (!x) {
			unsigned long mask = 1;
			while (mask < LESSER_SIZE - 1) {
				for (unsigned long y = 0; y <= mask; y++) {
					Word iterated = iteration_table[i & mask & (ITERATION_SIZE - 1)];
					Word indirect = indirection_table[c & mask & (INDIRECTION_SIZE - 1)];
					indirection_table[c & mask] = iterated + a;
					iteration_table[i & mask] = indirect;
					Word old = a ^ b;
					a = b + i++;
					b = c + indirect;
					c = old + rotate8(c, 3);
					indirection_table[y + mask + 1] = b ^ iterated;
				}
				mask = (mask << 1) | 1;
			}
			while (mask < GREATER_SIZE - 1) {
				for (unsigned long y = 0; y <= mask; y++) {
					Word iterated = iteration_table[i & mask & (ITERATION_SIZE - 1)];
					Word indirect = indirection_table[c & mask & (INDIRECTION_SIZE - 1)];
					indirection_table[c & mask] = iterated + a;
					iteration_table[i & mask] = indirect;
					Word old = a ^ b;
					a = b + i++;
					b = c + indirect;
					c = old + rotate8(c, 3);
					//return b ^ iterated;
				}
				mask = (mask << 1) | 1;
			}
		}
		else {
			for (unsigned long y = 0; y < GREATER_SIZE; y++) raw8();
		}
	}
}
void PractRand::RNGs::Raw::efiix8x48::seed(PractRand::RNGs::vRNG *source_rng) {
	a = source_rng->raw8();
	b = source_rng->raw8();
	c = source_rng->raw8();
	i = source_rng->raw8();
	// number of possible seeded states is kept much smaller than the number of valid states
	// in order to make it extremely unlikely that any bad seeds exist
	// as opposed to how it would be otherwise, in which finding a bad seed would be *extremely* difficult, but a few would likely exist
	for (int x = 0; x < ITERATION_SIZE; x++) if (!(x & 3)) iteration_table[x] = source_rng->raw8(); else iteration_table[x] = iteration_table[x & ~3];
	for (int x = 0; x < INDIRECTION_SIZE; x++) indirection_table[x] = source_rng->raw8();

	//ought to be a secure seeding if source_rng->get_flags() includes FLAG::CRYPTOGRAPHIC_SECURITY, so...
	for (int x = 0; x < ITERATION_SIZE; x++) raw8();
}
void PractRand::RNGs::Raw::efiix8x48::walk_state(StateWalkingObject *walker) {
	for (unsigned long w = 0; w < ITERATION_SIZE; w++) walker->handle(iteration_table[w]);
	for (unsigned long w = 0; w < INDIRECTION_SIZE; w++) walker->handle(indirection_table[w]);
	walker->handle(i);
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
}


Uint16 PractRand::RNGs::Raw::efiix16x48::raw16() {
	typedef Word check_efiix_array_sizes[(ITERATION_SIZE & (ITERATION_SIZE-1)) || (INDIRECTION_SIZE & (INDIRECTION_SIZE-1)) ? -1 : 1];
	EFIIX_ALGORITHM(16, 7)
}
void PractRand::RNGs::Raw::efiix16x48::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) {
	EFIIX_SEED( 16 )
}
void PractRand::RNGs::Raw::efiix16x48::seed(PractRand::RNGs::vRNG *source_rng) {
	a = source_rng->raw16();
	b = source_rng->raw16();
	c = source_rng->raw16();
	i = source_rng->raw16();
	// number of possible seeded states is kept much smaller than the number of valid states
	// in order to make it extremely unlikely that any bad seeds exist
	// as opposed to how it would be otherwise, in which finding a bad seed would be *extremely* difficult, but a few would likely exist
	for (int x = 0; x < ITERATION_SIZE; x++) if (!(x & 3)) iteration_table[x] = source_rng->raw16(); else iteration_table[x] = iteration_table[x & ~3];
	for (int x = 0; x < INDIRECTION_SIZE; x++) indirection_table[x] = source_rng->raw16();

	//ought to be a secure seeding if source_rng->get_flags() includes FLAG::CRYPTOGRAPHIC_SECURITY, so...
	for (int x = 0; x < ITERATION_SIZE; x++) raw16();
}
void PractRand::RNGs::Raw::efiix16x48::walk_state(StateWalkingObject *walker) {
	for (unsigned long w = 0; w < ITERATION_SIZE; w++) walker->handle(iteration_table[w]);
	for (unsigned long w = 0; w < INDIRECTION_SIZE; w++) walker->handle(indirection_table[w]);
	walker->handle(i);
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
}


Uint32 PractRand::RNGs::Raw::efiix32x48::raw32() {
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
void PractRand::RNGs::Raw::efiix32x48::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) {
	EFIIX_SEED(32)
}
void PractRand::RNGs::Raw::efiix32x48::seed(PractRand::RNGs::vRNG *source_rng) {
	a = source_rng->raw32();
	b = source_rng->raw32();
	c = source_rng->raw32();
	i = source_rng->raw32();
	// number of possible seeded states is kept much smaller than the number of valid states
	// in order to make it extremely unlikely that any bad seeds exist
	// as opposed to how it would be otherwise, in which finding a bad seed would be *extremely* difficult, but a few would likely exist
	for (int x = 0; x < ITERATION_SIZE; x++) if (!(x & 3)) iteration_table[x] = source_rng->raw32(); else iteration_table[x] = iteration_table[x & ~3];
	for (int x = 0; x < INDIRECTION_SIZE; x++) indirection_table[x] = source_rng->raw32();

	//ought to be a secure seeding if source_rng->get_flags() includes FLAG::CRYPTOGRAPHIC_SECURITY, so...
	for (int x = 0; x < ITERATION_SIZE; x++) raw32();
}
//void PractRand::RNGs::Raw::efiix32x48::seed(const Word *seeds, int num_seeds, int seeding_quality) {
//}
void PractRand::RNGs::Raw::efiix32x48::walk_state(StateWalkingObject *walker) {
	for (unsigned long w = 0; w < ITERATION_SIZE; w++) walker->handle(iteration_table[w]);
	for (unsigned long w = 0; w < INDIRECTION_SIZE; w++) walker->handle(indirection_table[w]);
	walker->handle(i);
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
}


Uint64 PractRand::RNGs::Raw::efiix64x48::raw64() {
	typedef Word check_efiix_array_sizes[(ITERATION_SIZE & (ITERATION_SIZE-1)) || (INDIRECTION_SIZE & (INDIRECTION_SIZE-1)) ? -1 : 1];
	EFIIX_ALGORITHM(64, 25)
}
void PractRand::RNGs::Raw::efiix64x48::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) {
	EFIIX_SEED( 64 )
}
void PractRand::RNGs::Raw::efiix64x48::seed(PractRand::RNGs::vRNG *source_rng) {
	a = source_rng->raw32();
	b = source_rng->raw32();
	c = source_rng->raw32();
	i = source_rng->raw32();
	// number of possible seeded states is kept much smaller than the number of valid states
	// in order to make it extremely unlikely that any bad seeds exist
	// as opposed to how it would be otherwise, in which finding a bad seed would be *extremely* difficult, but a few would likely exist
	for (int x = 0; x < ITERATION_SIZE; x++) if (!(x & 3)) iteration_table[x] = source_rng->raw32(); else iteration_table[x] = iteration_table[x & ~3];
	for (int x = 0; x < INDIRECTION_SIZE; x++) indirection_table[x] = source_rng->raw32();

	//ought to be a secure seeding if source_rng->get_flags() includes FLAG::CRYPTOGRAPHIC_SECURITY, so...
	for (int x = 0; x < ITERATION_SIZE; x++) raw64();
}
void PractRand::RNGs::Raw::efiix64x48::walk_state(StateWalkingObject *walker) {
	for (unsigned long w = 0; w < ITERATION_SIZE; w++) walker->handle(iteration_table[w]);
	for (unsigned long w = 0; w < INDIRECTION_SIZE; w++) walker->handle(indirection_table[w]);
	walker->handle(i);
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
}

