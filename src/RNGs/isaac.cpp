#include <cstring>
#include <string>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"

#include "PractRand/RNGs/isaac32x256.h"
#include "PractRand/RNGs/isaac64x256.h"

using namespace PractRand;

//polymorphic:
PRACTRAND__POLYMORPHIC_RNG_BASICS_C32(isaac32x256)
void PractRand::RNGs::Polymorphic::isaac32x256::seed(Uint64 s) { implementation.seed(s); }
void PractRand::RNGs::Polymorphic::isaac32x256::seed(vRNG *seeder_rng) { implementation.seed(seeder_rng); }
void PractRand::RNGs::Polymorphic::isaac32x256::flush_buffers() { implementation.flush_buffers(); }
std::string PractRand::RNGs::Polymorphic::isaac32x256::get_name() const {return "isaac32x256";}

PRACTRAND__POLYMORPHIC_RNG_BASICS_C64(isaac64x256)
void PractRand::RNGs::Polymorphic::isaac64x256::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::isaac64x256::seed(vRNG *seeder_rng) { implementation.seed(seeder_rng); }
void PractRand::RNGs::Polymorphic::isaac64x256::flush_buffers() { implementation.flush_buffers(); }
std::string PractRand::RNGs::Polymorphic::isaac64x256::get_name() const {return "isaac64x256";}

//raw:
PractRand::RNGs::Raw::isaac32x256::~isaac32x256() {std::memset(this, 0, sizeof(this));}
PractRand::RNGs::Raw::isaac64x256::~isaac64x256() {std::memset(this, 0, sizeof(this));}

#define ind32(mm,x)  (*(Uint32 *)((Uint8 *)(mm) + ((x) & ((SIZE-1)<<2))))
#define rngstep32(mix,a,b,mm,m,m2,r,x) \
{ \
  x = *m;  \
  a = (a^(mix)) + *(m2++); \
  *(m++) = y = ind32(mm,x) + a + b; \
  *(r++) = b = ind32(mm,y>>SIZE_L2) + x; \
}
void PractRand::RNGs::Raw::isaac32x256::_advance_state() {//do not change
	Uint32 *m, *m2, *mend, *r;
	Uint32 x, y;
	m = state;
	r = results;
	b += ++c;
	for (m = state, mend = m2 = m+(SIZE/2); m<mend; )
	{
		rngstep32( a<<13, a, b, state, m, m2, r, x);
		rngstep32( a>>6 , a, b, state, m, m2, r, x);
		rngstep32( a<<2 , a, b, state, m, m2, r, x);
		rngstep32( a>>16, a, b, state, m, m2, r, x);
	}
	for (m2 = state; m2<mend; )
	{
		rngstep32( a<<13, a, b, state, m, m2, r, x);
		rngstep32( a>>6 , a, b, state, m, m2, r, x);
		rngstep32( a<<2 , a, b, state, m, m2, r, x);
		rngstep32( a>>16, a, b, state, m, m2, r, x);
	}
	used = 0;
}
//Uint32 PractRand::RNGs::Raw::isaac32x256::raw32() {//LOCKED, do not change
//	if ( used >= SIZE ) _advance_state();
//	return results[used++];
//}
static void mix32x8(Uint32 t[8]) {
	Uint32 &a = t[0], &b = t[1], &c = t[2], &d = t[3];
	Uint32 &e = t[4], &f = t[5], &g = t[6], &h = t[7];
	a^=b<<11; d+=a; b+=c;
	b^=c>>2;  e+=b; c+=d;
	c^=d<<8;  f+=c; d+=e;
	d^=e>>16; g+=d; e+=f;
	e^=f<<10; h+=e; f+=g;
	f^=g>>4;  a+=f; g+=h;
	g^=h<<8;  b+=g; h+=a;
	h^=a>>9;  c+=h; a+=b;
}
void PractRand::RNGs::Raw::isaac32x256::_seed(bool flag) {//LOCKED, do not change
	//the reference seeding algorithm for ISAAC
	//not used directly by PractRand since it isn't a good match for any PractRand interface
	//but the visible seeding methods map to this
	Uint32 tmp[8];
	a = b = c = 0;
	for (int i = 0; i < 8; i++) tmp[i] = 0x9e3779b9UL;  // the golden ratio

	for (int i=0; i<4; ++i)          // scramble it
	{
		mix32x8(tmp);
	}

	if (flag) {
		// initialize using the contents of state[] as the seed
		// do a second pass to make all of the seed affect all of m
		for (int pass = 0; pass < 2; pass++) {
			for (int i=0; i<SIZE; i+=8) {
				for (int x = 0; x < 8; x++) tmp[x] += state[i+x];
				mix32x8(tmp);
				for (int x = 0; x < 8; x++) state[i+x] = tmp[x];
			}
		}
	}
	else {
		// fill in mm[] with messy stuff
		for (int i=0; i<SIZE; i+=8)
		{
			mix32x8(tmp);
			for (int x = 0; x < 8; x++) state[i+x] = tmp[x];
		}
	}

	used = SIZE;
}
void PractRand::RNGs::Raw::isaac32x256::seed(Uint32 s[256]) {//LOCKED, do not change
	for (int i = 0; i < 256; i++) state[i] = s[i];
	_seed(true);
}
void PractRand::RNGs::Raw::isaac32x256::seed(Uint64 s) {//LOCKED, do not change
	//changed in 0.85 to improve seeding quality
	//and to make it more similar to the reference seeding algorithm
	//(which can't be used directly since it takes a non-standard seed structure)
	state[0] = Uint32(s);
	state[1] = Uint32(s>>32);
	for (int i = 2; i < SIZE; i++) state[i] = 0;
	_seed(true);
}
void PractRand::RNGs::Raw::isaac32x256::seed(vRNG *seeder_rng) {//LOCKED, do not change
	for (int i = 0; i < SIZE; i += 2) {
		state[i] = seeder_rng->raw32();
		state[i + 1] = 0;
	}
	_seed(true);
}
void PractRand::RNGs::Raw::isaac32x256::walk_state(StateWalkingObject *walker) {
	//LOCKED, do not change
	//exception - changed in 0.85 to fix broken seeding
	for (unsigned int i = 0; i < SIZE; i++) walker->handle(state[i]);
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
	if (walker->is_seeder()) used = SIZE;
	else {
		for (unsigned int i = 0; i < SIZE; i++) walker->handle(results[i]);
		walker->handle(used);
		if (used > SIZE) used = SIZE;
	}
}
void PractRand::RNGs::Raw::isaac32x256::self_test() {
	Raw::isaac32x256 rng;
	rng.seed(0ull);
	Uint64 checksum = 0;
	for (int i = 0; i < 8192; i++) {
		checksum ^= checksum << 24;
		checksum ^= checksum >> 27;
		checksum += rng.raw32();
	}
	if (checksum != 0x3ff1d629d8878831ull) issue_error("isaac32x256::self_test() failed");
}



#define ind64(mm,x)  (*(Uint64 *)((Uint8 *)(mm) + ((x) & ((SIZE-1)<<3))))
#define rngstep64(mix,a,b,mm,m,m2,r,x) \
{ \
  x = *m;  \
  a = (a^(mix)) + *(m2++); \
  *(m++) = y = ind64(mm,x) + a + b; \
  *(r++) = b = ind64(mm,y>>SIZE_L2) + x; \
}
void PractRand::RNGs::Raw::isaac64x256::_advance_state() {//do not change
	Uint64 *m, *m2, *mend, *r;
	Uint64 x, y;
	m = state;
	r = results;
	b += ++c;
	for (m = state, mend = m2 = m+(SIZE/2); m<mend; )
	{
		rngstep64(~(a^(a<<21)), a, b, state, m, m2, r, x);
		rngstep64(  a^(a>> 5) , a, b, state, m, m2, r, x);
		rngstep64(  a^(a<<12) , a, b, state, m, m2, r, x);
		rngstep64(  a^(a>>33) , a, b, state, m, m2, r, x);
	}
	for (m2 = state; m2<mend; )
	{
		rngstep64(~(a^(a<<21)), a, b, state, m, m2, r, x);
		rngstep64(  a^(a>> 5) , a, b, state, m, m2, r, x);
		rngstep64(  a^(a<<12) , a, b, state, m, m2, r, x);
		rngstep64(  a^(a>>33) , a, b, state, m, m2, r, x);
	}
	used = 0;
}
//Uint64 PractRand::RNGs::Raw::isaac64x256::raw64() {//LOCKED, do not change
//	if ( used >= SIZE ) _advance_state();
//	return results[used++];
//}
static void mix64x8(Uint64 t[8]) {
	Uint64 &a = t[0], &b = t[1], &c = t[2], &d = t[3];
	Uint64 &e = t[4], &f = t[5], &g = t[6], &h = t[7];
	a-=e; f^=h>>9;  h+=a;
	b-=f; g^=a<<9;  a+=b;
	c-=g; h^=b>>23; b+=c;
	d-=h; a^=c<<15; c+=d;
	e-=a; b^=d>>14; d+=e;
	f-=b; c^=e<<20; e+=f;
	g-=c; d^=f>>17; f+=g;
	h-=d; e^=g<<14; g+=h;
}
void PractRand::RNGs::Raw::isaac64x256::_seed(bool flag) {//LOCKED, do not change
	//the reference seeding algorithm for ISAAC64
	//not used directly by PractRand since it isn't a good match for any PractRand interface
	//but the visible seeding methods map to this
	Uint64 tmp[8];
	a = b = c = 0;
	for (int i = 0; i < 8; i++) tmp[i] = 0x9e3779b97f4a7c13uLL;  // the golden ratio

	for (int i=0; i<4; ++i)          // scramble it
	{
		mix64x8(tmp);
	}

	if (flag) {
		// initialize using the contents of state[] as the seed
		// do a second pass to make all of the seed affect all of m
		for (int pass = 0; pass < 2; pass++) {
			for (int i=0; i<SIZE; i+=8) {
				for (int x = 0; x < 8; x++) tmp[x] += state[i+x];
				mix64x8(tmp);
				for (int x = 0; x < 8; x++) state[i+x] = tmp[x];
			}
		}
	}
	else {
		// fill in mm[] with messy stuff
		for (int i=0; i<SIZE; i+=8)
		{
			mix64x8(tmp);
			for (int x = 0; x < 8; x++) state[i+x] = tmp[x];
		}
	}

	used = SIZE;
}
void PractRand::RNGs::Raw::isaac64x256::seed(Uint64 s[256]) {//LOCKED, do not change
	for (int i = 0; i < 256; i++) state[i] = s[i];
	_seed(true);
}
void PractRand::RNGs::Raw::isaac64x256::seed(Uint64 s) {//LOCKED, do not change
	//changed in 0.85 to improve seeding quality
	//and to make it more similar to the reference seeding algorithm
	//(which can't be used directly since it takes a non-standard seed structure)
	state[0] = s;
	for (int i = 1; i < SIZE; i++) state[i] = 0;
	_seed(true);
}
void PractRand::RNGs::Raw::isaac64x256::seed(vRNG *seeder_rng) {//LOCKED, do not change
	for (int i = 0; i < SIZE; i += 2) {
		state[i] = seeder_rng->raw64();
		state[i + 1] = 0;
	}
	_seed(true);
}
void PractRand::RNGs::Raw::isaac64x256::walk_state(StateWalkingObject *walker) {
	//LOCKED, do not change
	//exception - changed in 0.85 to fix broken seeding
	for (unsigned int i = 0; i < SIZE; i++) walker->handle(state[i]);
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
	if (walker->is_seeder()) used = SIZE;
	else {
		for (unsigned int i = 0; i < SIZE; i++) walker->handle(results[i]);
		walker->handle(used);
		if (used > SIZE) used = SIZE;
	}
}
