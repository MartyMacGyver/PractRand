#include <string>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"

//#include "PractRand/RNGs/jsf8.h"
//#include "PractRand/RNGs/jsf16.h"
#include "PractRand/RNGs/jsf32.h"
#include "PractRand/RNGs/jsf64.h"

using namespace PractRand;

//polymorphic:
PRACTRAND__POLYMORPHIC_RNG_BASICS_C64(jsf64)
void PractRand::RNGs::Polymorphic::jsf64::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::jsf64::seed_fast(Uint64 s) {implementation.seed_fast(s);}
std::string PractRand::RNGs::Polymorphic::jsf64::get_name() const {return "jsf64";}

PRACTRAND__POLYMORPHIC_RNG_BASICS_C32(jsf32)
void PractRand::RNGs::Polymorphic::jsf32::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::jsf32::seed_fast(Uint64 s) {implementation.seed_fast(s);}
std::string PractRand::RNGs::Polymorphic::jsf32::get_name() const {return "jsf32";}

//raw:
Uint32 PractRand::RNGs::Raw::jsf32::raw32() {//LOCKED, do not change
	Uint32 e = a - ((b << 27) | (b >> 5));
	a = b ^ ((c << 17) | (c >> 15));
	b = c + d;
	c = d + e;
	d = e + a;
	return d;
}
void PractRand::RNGs::Raw::jsf32::seed(Uint64 s) {//LOCKED, do not change
	//LOCKED, do not change
	//exception: changed in 0.81 to match Robert Jenkins code
	//note: only actually matches Robert Jenkins code for seeds that fit in a 32 bit value, as he only took a 32 bit seed
	a = 0xf1ea5eed ^ Uint32(s >> 32);
	b = Uint32(s);
	c = b ^ Uint32(s >> 32);
	d = b;
	for (int i = 0; i < 20; i++) raw32();//20
	/*
		number of outputs skipped vs number of seeds needed to detect bias via expanded battery w/ extra folding:
		2: 2**10
		3: 2**12
		4: 2**18
		5: 2**23
		6: 2**35
		7: >= 2**42
		conclusion:
			The 20 outputs skipped by the standard algorithm are more than sufficient.  
			8 should be enough for most purposes, 
			12 should be enough for any purpose that could possibly be satisfied by seeding from a 64 bit integer.  
			(this PRNG won't be secure no matter how good seeding is, so the best we can do is 2**64 seeds producing uncorrelated results)
	*/
}
void PractRand::RNGs::Raw::jsf32::seed_fast(Uint64 s) {
	a = 0xf1ea5eed ^ Uint32(s >> 32);
	b = Uint32(s);
	c = b ^ Uint32(s >> 32);
	d = b;
	for (int i = 0; i < 8; i++) raw32();
}
void PractRand::RNGs::Raw::jsf32::seed(vRNG *seeder_rng) {//custom seeding
	a = b = seeder_rng->raw32();
	c = d = seeder_rng->raw32();
	for (int i = 0; i < 4; i++) raw32();//4
}
void PractRand::RNGs::Raw::jsf32::seed(Uint32 seed1, Uint32 seed2, Uint32 seed3, Uint32 seed4) {//custom seeding
		//LOCKED, do not change
	//exception to the locked status - 
	//   when more bad cycles are found, more code might be added to prohibit them
	//exception: changed in 0.87 to to reduce correlation between similar seeds
	a = seed1;
	b = seed2;
	c = seed3;
	d = seed4;
	if ((d&0x80093300) == 0) {
		//these block the cycles of length 1
		//should search for cycles of lengths 2 to 2**64 to block them as well, but that's impractical at this time
		if (!a && !b && !c && !d) d++;
		if (a==0x77777777 && b==0x55555555 && c==0x11111111 && d==0x44444444 ) d++;
		if (a==0x5591F2E3 && b==0x69EBA6CD && c==0x2A171E3D && d==0x3FD48890 ) d++;
		if (a==0x47CB8D56 && b==0xAE9B35A7 && c==0x5C78F4A8 && d==0x522240FF ) d++;
	}
	for (int i = 0; i < 12; i++) raw32();//12
}
void PractRand::RNGs::Raw::jsf32::walk_state(StateWalkingObject *walker) {
	//LOCKED, do not change
	//exception to the locked status - 
	//   when more bad cycles are found, more code might be added to prohibit them
	//exception: in 0.87 changed how the all-zeroes case is handled for consistency
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
	walker->handle(d);
	if (!(walker->get_properties() & StateWalkingObject::FLAG_READ_ONLY) && (d&0x80093300) == 0) {
		//these block the cycles of length 1
		//should search for all cycles with length less than 2**56, but that's impractical at this time
		if (!a && !b && !c && !d) d++;
		if (a==0x77777777 && b==0x55555555 && c==0x11111111 && d==0x44444444 ) d++;
		if (a==0x5591F2E3 && b==0x69EBA6CD && c==0x2A171E3D && d==0x3FD48890 ) d++;
		if (a==0x47CB8D56 && b==0xAE9B35A7 && c==0x5C78F4A8 && d==0x522240FF ) d++;
	}
}
Uint64 PractRand::RNGs::Raw::jsf64::raw64() {
	//LOCKED, do not change
	Uint64 e = a - ((b << 39) | (b >> 25));
	a = b ^ ((c << 11) | (c >> 53));
	b = c + d;
	c = d + e;
	d = e + a;
	return d;
}
void PractRand::RNGs::Raw::jsf64::seed(Uint64 s) {
	//LOCKED, do not change
	a = 0xf1ea5eed;
	b = c = d = s;
	for (int i = 0; i < 20; i++) raw64();
}
void PractRand::RNGs::Raw::jsf64::seed_fast(Uint64 s) {
	a = 0xf1ea5eed;
	b = c = d = s;
	for (int i = 0; i < 8; i++) raw64();
}
void PractRand::RNGs::Raw::jsf64::walk_state(StateWalkingObject *walker) {
	//LOCKED, do not change
	//exception: changed in 0.87 to to reduce correlation between similar seeds
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
	walker->handle(d);
	if (!(a|b) && !(c|d)) d = 1;
	for (int i = 0; i < 12; i++) raw64();//12
}



