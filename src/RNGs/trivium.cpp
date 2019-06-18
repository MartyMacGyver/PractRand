#include <cstring>
#include <string>
#include "PractRand/config.h"
#include "PractRand/endian.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"

#include "PractRand/RNGs/trivium.h"

using namespace PractRand;


//polymorphic:
PRACTRAND__POLYMORPHIC_RNG_BASICS_C64(trivium)
void PractRand::RNGs::Polymorphic::trivium::seed(Uint64 s) { implementation.seed(s); }
void PractRand::RNGs::Polymorphic::trivium::seed_fast(Uint64 s) { implementation.seed_fast(s, s); }
void PractRand::RNGs::Polymorphic::trivium::seed(vRNG *seeder_rng) { implementation.seed(seeder_rng); }
void PractRand::RNGs::Polymorphic::trivium::seed(const Uint8 *seed_and_iv, int length) { implementation.seed(seed_and_iv, length); }
std::string PractRand::RNGs::Polymorphic::trivium::get_name() const {return "trivium";}

static Uint64 shift_array64( Uint64 vec[2], unsigned long bits ) {
	bits -= 64;
	if (!(bits % 64)) return vec[bits/64];
	return (vec[bits / 64] << (bits & 63)) | (vec[1 + bits / 64] >> (64-(bits & 63)));
}
//raw:
PractRand::RNGs::Raw::trivium::~trivium() {std::memset(this, 0, sizeof(this));}
Uint64 PractRand::RNGs::Raw::trivium::raw64() {//LOCKED, do not change
	Uint64 tmp_a = shift_array64(c, 66) ^ shift_array64(c,111);
	Uint64 tmp_b = shift_array64(a, 66) ^ shift_array64(a, 93);
	Uint64 tmp_c = shift_array64(b, 69) ^ shift_array64(b, 84);
	Uint64 new_a = tmp_a ^ shift_array64(a, 69) ^ (shift_array64(c,110) & shift_array64(c,109));
	Uint64 new_b = tmp_b ^ shift_array64(b, 78) ^ (shift_array64(a, 92) & shift_array64(a, 91));
	Uint64 new_c = tmp_c ^ shift_array64(c, 87) ^ (shift_array64(b, 83) & shift_array64(b, 82));
	a[1] = a[0]; a[0] = new_a;
	b[1] = b[0]; b[0] = new_b;
	c[1] = c[0]; c[0] = new_c;

	return tmp_a ^ tmp_b ^ tmp_c;
}
void PractRand::RNGs::Raw::trivium::seed(Uint64 s) {//LOCKED, do not change
	//Triviums standard seeding algorithm adapted to PractRand interface
	Uint8 vec[8];
	for (int i = 0; i < 8; i++) {
		vec[i] = Uint8(s); 
		s >>= 8;
	}
	seed(vec, 8);
}
void PractRand::RNGs::Raw::trivium::seed_fast(Uint64 s1, Uint64 s2, int quality) {//LOCKED, do not change
	//a non-standard simplified 128-bit seeding algorithm
	a[0] = s1; a[1] = 0;
	b[0] = s2; b[1] = 0;
	c[0] = 0;
	c[1] = Uint64(7) << (128-111);
	for (int i = 0; i < quality; i++) raw64();
}
void PractRand::RNGs::Raw::trivium::seed(vRNG *seeder_rng) {//LOCKED, do not change
	Uint64 s1 = seeder_rng->raw64();
	Uint64 s2 = seeder_rng->raw64();
	seed_fast(s1, s2, 3);
}
void PractRand::RNGs::Raw::trivium::seed(const Uint8 *seed_and_iv, int length) {//LOCKED, do not change
	//standard algorithm for Trivium, not a good match for PractRand
	if (length > 20) issue_error("trivium seeded with invalid length");

	union SeedVec{
		Uint8 as8[16];
		Uint64 as64[2];
	};
	SeedVec s;
	int elen = length > 10 ? 10 : length;
	for (int i = 0; i < 6; i++) s.as8[i] = 0;
	for (int i = 0; i < elen; i++) s.as8[i+6] = seed_and_iv[i];
	for (int i = elen; i < 10; i++) s.as8[i+6] = 0;
	for (int i = 0; i < 2; i++) this->a[i] = little_endian_conversion64(s.as64[1-i]);

	length -= 10;
	seed_and_iv += 10;
	length = length > 0 ? length : 0;
	SeedVec iv;
	for (int i = 0; i < 16-length; i++) iv.as8[i] = 0;
	for (int i = 16-length; i < 16; i++) iv.as8[i] = seed_and_iv[i-(16-length)];
	for (int i = 0; i < 2; i++) this->b[i] = little_endian_conversion64(iv.as64[1-i]);

	c[0] = 0;
	c[1] = Uint64(7) << (128-111);

	//for (int i = 0; i < 1152; i+=1) raw1();
	for (int i = 0; i < 18; i++) raw64();
	/*
		(# of outputs discarded) vs (log2 of # of seeds needed to detect interseed correlation)
			4 - 13
			5 - 15
			6 - 25
			7 - 30
			8 - 
	*/
}
void PractRand::RNGs::Raw::trivium::walk_state(StateWalkingObject *walker) {
	//LOCKED, do not change
	walker->handle(a[0]);
	walker->handle(a[1]);
	walker->handle(b[0]);
	walker->handle(b[1]);
	walker->handle(c[0]);
	walker->handle(c[1]);
}

static void validate_trivium_result(Uint64 output, Uint64 reference) {
	//convert format of reference
	//	it will always be in the wrong endianness, regardless of platform
	//	due to how it was cut & pasted
	reference = PractRand::invert_endianness64(reference);
	//raise an error if they don't match
	if (output != reference) {
		//Uint64 r = output;
		//printf("%02x %02x %02x %02x %02x %02x %02x %02x\n", Uint8(r>>0), Uint8(r>>8), Uint8(r>>16), Uint8(r>>24), Uint8(r>>32), Uint8(r>>40), Uint8(r>>48), Uint8(r>>56));
		//for (int i = 0; i < 16; i++) printf("%X", int(Uint8(r >> (i*4)) & 15));
		//printf("\n");
		PractRand::issue_error("PractRand::RNGs::Raw::trivium failed validation");
	}
}
void PractRand::RNGs::Raw::trivium::self_test() {
	Raw::trivium rng;
	Uint8 seed_and_iv[10+10] = {0};
	rng.seed(seed_and_iv, 14);
	validate_trivium_result(rng.raw64(), 0xFBE0BF265859051Bull);
	seed_and_iv[0] = 0x80; rng.seed(seed_and_iv, 14); seed_and_iv[0] = 0x00; 
	validate_trivium_result(rng.raw64(), 0x38EB86FF730D7A9Cull);
	seed_and_iv[9] = 0x80; rng.seed(seed_and_iv, 14); seed_and_iv[9] = 0x00; 
	validate_trivium_result(rng.raw64(), 0x5D492E77F8FE62D7ull);
	seed_and_iv[13] = 0x10; rng.seed(seed_and_iv, 14); seed_and_iv[13] = 0x00; 
	validate_trivium_result(rng.raw64(), 0xB0820A503ABB0329ull);
	seed_and_iv[17] = 0x01; rng.seed(seed_and_iv, 18); seed_and_iv[17] = 0x00; 
	validate_trivium_result(rng.raw64(), 0x9A5C56169E7FA406ull);
}
