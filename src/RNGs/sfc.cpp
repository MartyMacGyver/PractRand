#include <string>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"

#include "PractRand/RNGs/sfc16.h"
#include "PractRand/RNGs/sfc32.h"
#include "PractRand/RNGs/sfc64.h"

using namespace PractRand;

//polymorphic:
PRACTRAND__POLYMORPHIC_RNG_BASICS_C64(sfc64)
void PractRand::RNGs::Polymorphic::sfc64::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::sfc64::seed_fast(Uint64 s) {implementation.seed_fast(s);}
void PractRand::RNGs::Polymorphic::sfc64::seed(Uint64 s1, Uint64 s2, Uint64 s3) {implementation.seed(s1,s2,s3);}
std::string PractRand::RNGs::Polymorphic::sfc64::get_name() const {return "sfc64";}

PRACTRAND__POLYMORPHIC_RNG_BASICS_C32(sfc32)
void PractRand::RNGs::Polymorphic::sfc32::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::sfc32::seed_fast(Uint64 s) {implementation.seed_fast(s);}
void PractRand::RNGs::Polymorphic::sfc32::seed(Uint32 s1, Uint32 s2, Uint32 s3) {implementation.seed(s1,s2,s3);}
std::string PractRand::RNGs::Polymorphic::sfc32::get_name() const {return "sfc32";}

PRACTRAND__POLYMORPHIC_RNG_BASICS_C16(sfc16)
void PractRand::RNGs::Polymorphic::sfc16::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::sfc16::seed_fast(Uint64 s) {implementation.seed_fast(s);}
void PractRand::RNGs::Polymorphic::sfc16::seed(Uint16 s1, Uint16 s2, Uint16 s3) {implementation.seed(s1,s2,s3);}
std::string PractRand::RNGs::Polymorphic::sfc16::get_name() const {return "sfc16";}

//raw:
Uint16 PractRand::RNGs::Raw::sfc16::raw16() {
	enum {BARREL_SHIFT = 7, RSHIFT = 3, LSHIFT = 2};
	Uint16 tmp = a + b + counter++;
	a = b ^ (b >> RSHIFT);
	b = c + (c << LSHIFT);
	c = ((c << BARREL_SHIFT) | (c >> (16-BARREL_SHIFT))) + tmp;
	return tmp;
}
void PractRand::RNGs::Raw::sfc16::seed(Uint64 s) {
	a = Uint16(s);
	b = Uint16(s >> 16);
	c = Uint16(s >> 32);
	counter = Uint16(s >> 48);
	for (int i = 0; i < 12; i++) raw16();
}
void PractRand::RNGs::Raw::sfc16::seed_fast(Uint64 s) {
	a = Uint16(s);
	b = Uint16(s >> 16);
	c = Uint16(s >> 32);
	counter = Uint16(s >> 48);
	for (int i = 0; i < 8; i++) raw16();
}
void PractRand::RNGs::Raw::sfc16::seed(Uint16 s1, Uint16 s2, Uint16 s3) {
	a = s1;
	b = s2;
	c = s3;
	counter = 1;
	for (int i = 0; i < 12; i++) raw16();
}
void PractRand::RNGs::Raw::sfc16::walk_state(StateWalkingObject *walker) {
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
	walker->handle(counter);
}
Uint32 PractRand::RNGs::Raw::sfc32::raw32() {
	enum {BARREL_SHIFT = 25, RSHIFT = 8, LSHIFT = 3};
	Uint32 tmp = a + b + counter++;
	a = b ^ (b >> RSHIFT);
	b = c + (c << LSHIFT);
	c = ((c << BARREL_SHIFT) | (c >> (32-BARREL_SHIFT))) + tmp;
	return tmp;
}
void PractRand::RNGs::Raw::sfc32::seed(Uint64 s) {
	a = Uint32(s >> 0);
	b = Uint32(s >> 32);
	c = 0;
	counter = 1;
	for (int i = 0; i < 12; i++) raw32();
	/*			e0f0	e0f1	e0f2		e1f0	e1f1	e1f2
sfc	32	4		12		12		10			12		12		10
sfc	32	5		19		17		13			17		17		12
sfc	32	6		26		16		16			24		17		16
sfc	32	7		25		25		19			26		22		17
sfc	32	8		>30		>29		>29			>26		?		25
sfc	32	9		?		?		?			?		?		32

sfc	16	4		25		14		13			25		14		14
sfc	16	5		27		23		20			27		21		20
sfc	16	6		30?		22		23			30		21		21

jsf	32	3											14		12
jsf	32	4											19		18
jsf	32	5									24		24		23
jsf	32	6													35
	*/
}
void PractRand::RNGs::Raw::sfc32::seed_fast(Uint64 s) {
	a = Uint32(s >> 0);
	b = Uint32(s >> 32);
	c = 0;
	counter = 1;
	for (int i = 0; i < 8; i++) raw32();
}
void PractRand::RNGs::Raw::sfc32::seed(Uint32 s1, Uint32 s2, Uint32 s3) {
	a = s1;
	b = s2;
	c = s3;
	counter = 1;
	for (int i = 0; i < 12; i++) raw32();
}
void PractRand::RNGs::Raw::sfc32::walk_state(StateWalkingObject *walker) {
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
	walker->handle(counter);
}
Uint64 PractRand::RNGs::Raw::sfc64::raw64() {
	enum {BARREL_SHIFT = 25, RSHIFT = 12, LSHIFT = 3};
	Uint64 tmp = a + b + counter++;
	a = b ^ (b >> RSHIFT);
	b = c + (c << LSHIFT);
	c = ((c << BARREL_SHIFT) | (c >> (64-BARREL_SHIFT))) + tmp;
	return tmp;
}
void PractRand::RNGs::Raw::sfc64::seed(Uint64 s) {
	a = b = c = s;
	counter = 1;
	for (int i = 0; i < 12; i++) raw64();
}
void PractRand::RNGs::Raw::sfc64::seed_fast(Uint64 s) {
	a = b = c = s;
	counter = 1;
	for (int i = 0; i < 8; i++) raw64();
}
void PractRand::RNGs::Raw::sfc64::seed(Uint64 s1, Uint64 s2, Uint64 s3) {
	a = s1;
	b = s2;
	c = s3;
	counter = 1;
	for (int i = 0; i < 12; i++) raw64();
}
void PractRand::RNGs::Raw::sfc64::walk_state(StateWalkingObject *walker) {
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
	walker->handle(counter);
}


