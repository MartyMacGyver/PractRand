#include <string>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"
#include "PractRand/endian.h"

#include "PractRand/RNGs/arbee.h"

using namespace PractRand;
using namespace PractRand::Internals;

//raw:
void PractRand::RNGs::Raw::arbee::reset_entropy() {
	//the default state is arrived at by setting all values to 1, then calling mix()
	a = 9873171087373218264ull;
	b = 10599573592049074392ull;
	c = 16865209178899817893ull;
	d = 5013818595375203225ull;
	i = 13;
}
Uint64 PractRand::RNGs::Raw::arbee::raw64() {
	Uint64 e = a + rotate(b,45);
	a = b ^ rotate(c, 13);
	b = c + rotate(d, 37);
	c = e + d + i++;
	d = e + a;
	return d;
}
void PractRand::RNGs::Raw::arbee::mix() {
	for (int x = 0; x < 12; x++) raw64();
	/*
		(# of outputs discarded) vs (log2 of # of seeds needed to detect interseed correlation)
			4 - 10-11
			5 - 15-16 5
			6 - 22-23 7
			7 - 34-35 12
	*/
}
void PractRand::RNGs::Raw::arbee::seed(Uint64 seed1, Uint64 seed2, Uint64 seed3, Uint64 seed4) {
	a = seed1;
	b = seed2;
	c = seed3;
	d = seed4;
	i = 1;
	mix();
}
void PractRand::RNGs::Raw::arbee::seed(Uint64 s) {
	a = s;
	b = 1;
	c = 2;
	d = 3;
	i = 1;
	mix();
}
void PractRand::RNGs::Raw::arbee::seed(vRNG *rng) {
	a = rng->raw64();
	b = rng->raw64();
	c = rng->raw64();
	d = rng->raw64();
	i = 13;
}
void PractRand::RNGs::Raw::arbee::walk_state(StateWalkingObject *walker) {
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
	walker->handle(d);
	walker->handle(i);
	if (walker->is_seeder()) i = 1;
}
void PractRand::RNGs::Raw::arbee::add_entropy_N(const void *_data, size_t length) {
	const Uint8 *data = (const Uint8*) _data;
#ifdef PRACTRAND_TARGET_IS_LITTLE_ENDIAN
	//maybe add an ifdef to enable misaligned reads at compile time where appropriate?
/*	if (!(7 & reinterpret_cast<Uint64>(data))) {
	//if (!(((unsigned long)data) & 7)) {
		while (length >= 8) {
			add_entropy64(*(Uint64*)data);
			data += 8;
			length -= 8;
		}
	}
	else*/
#endif
	while (length >= 8) {
		Uint64 in = Uint64(*(data++));
		in |= Uint64(*(data++)) << 8;
		in |= Uint64(*(data++)) << 16;
		in |= Uint64(*(data++)) << 24;
		in |= Uint64(*(data++)) << 32;
		in |= Uint64(*(data++)) << 40;
		in |= Uint64(*(data++)) << 48;
		in |= Uint64(*(data++)) << 56;
		add_entropy64(in);
		length -= 8;
	}
	while (length >= 2) {
		Uint16 in = Uint64(*(data++));
		in |= Uint16(*(data++)) << 8;
		add_entropy16(in);
		length -= 2;
	}
	if (length) add_entropy8(*data);
}
void PractRand::RNGs::Raw::arbee::add_entropy8(Uint8 value) {
	add_entropy16(value);
}
void PractRand::RNGs::Raw::arbee::add_entropy16(Uint16 value) {
	d ^= value;
	b += value;
	raw64();
	b += value;
}
void PractRand::RNGs::Raw::arbee::add_entropy32(Uint32 value) {
	d ^= value;
	b += value;
	raw64();
	raw64();
	b += value;
}
void PractRand::RNGs::Raw::arbee::add_entropy64(Uint64 value) {
	d ^= value;
	b += value;
	raw64();
	raw64();
	raw64();
	raw64();
	b += value;
}


//polymorphic:
Uint64 PractRand::RNGs::Polymorphic::arbee::get_flags() const {
	return FLAGS;
}
std::string PractRand::RNGs::Polymorphic::arbee::get_name() const {
	return std::string("arbee");
}
Uint8  PractRand::RNGs::Polymorphic::arbee::raw8 () {
	return Uint8(implementation.raw64());
}
Uint16 PractRand::RNGs::Polymorphic::arbee::raw16() {
	return Uint16(implementation.raw64());
}
Uint32 PractRand::RNGs::Polymorphic::arbee::raw32() {
	return Uint32(implementation.raw64());
}
Uint64 PractRand::RNGs::Polymorphic::arbee::raw64() {
	return Uint64(implementation.raw64());
}
void PractRand::RNGs::Polymorphic::arbee::add_entropy_N(const void *data, size_t length) {
	implementation.add_entropy_N(data, length);
}
void PractRand::RNGs::Polymorphic::arbee::add_entropy8 (Uint8  value) {
	implementation.add_entropy8 (value);
}
void PractRand::RNGs::Polymorphic::arbee::add_entropy16(Uint16 value) {
	implementation.add_entropy16(value);
}
void PractRand::RNGs::Polymorphic::arbee::add_entropy32(Uint32 value) {
	implementation.add_entropy32(value);
}
void PractRand::RNGs::Polymorphic::arbee::add_entropy64(Uint64 value) {
	implementation.add_entropy64(value);
}
void PractRand::RNGs::Polymorphic::arbee::seed(Uint64 s) {
	implementation.seed(s);
}
void PractRand::RNGs::Polymorphic::arbee::seed(vRNG *rng) {
	implementation.seed(rng);
}
void PractRand::RNGs::Polymorphic::arbee::seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4) {
	implementation.seed(s1, s2, s3, s4);
}
void PractRand::RNGs::Polymorphic::arbee::reset_entropy() {
	implementation.reset_entropy();
}
void PractRand::RNGs::Polymorphic::arbee::walk_state(StateWalkingObject *walker) {
	implementation.walk_state(walker);
}
void PractRand::RNGs::Polymorphic::arbee::flush_buffers() {
	implementation.flush_buffers();
}
