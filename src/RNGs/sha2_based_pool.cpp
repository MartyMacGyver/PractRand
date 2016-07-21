#include <cstring>
#include <string>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"

#include "PractRand/sha2.h"

#include "PractRand/RNGs/sha2_based_pool.h"


using namespace PractRand;


PractRand::RNGs::Polymorphic::sha2_based_pool::~sha2_based_pool() {
	std::memset(state, 0, STATE_SIZE);
	std::memset(input_buffer, 0, INPUT_BUFFER_SIZE);
	std::memset(output_buffer, 0, OUTPUT_BUFFER_SIZE);
	input_buffer_left = output_buffer_left = state_phase = 0;
}
std::string PractRand::RNGs::Polymorphic::sha2_based_pool::get_name() const {
	return std::string("sha2_based_pool");
}
Uint64 PractRand::RNGs::Polymorphic::sha2_based_pool::get_flags() const {
	return FLAGS;
}
Uint8 PractRand::RNGs::Polymorphic::sha2_based_pool::raw8() {
	if (!output_buffer_left) refill_output_buffer();
	return output_buffer[--output_buffer_left];
}
void PractRand::RNGs::Polymorphic::sha2_based_pool::add_entropy8(Uint8 value) {
	input_buffer[--input_buffer_left] = value;
	if (!input_buffer_left) empty_input_buffer();
}
void PractRand::RNGs::Polymorphic::sha2_based_pool::add_entropy16(Uint16 value) {
	add_entropy8(Uint8(value   ));
	add_entropy8(Uint8(value>>8));
}
void PractRand::RNGs::Polymorphic::sha2_based_pool::add_entropy32(Uint32 value) {
	add_entropy8(Uint8(value    ));
	add_entropy8(Uint8(value>>8 ));
	add_entropy8(Uint8(value>>16));
	add_entropy8(Uint8(value>>24));
}
void PractRand::RNGs::Polymorphic::sha2_based_pool::add_entropy64(Uint64 value) {
	add_entropy8(Uint8(value   ));
	add_entropy8(Uint8(value>>8));
	add_entropy8(Uint8(value>>16));
	add_entropy8(Uint8(value>>24));
	add_entropy8(Uint8(value>>32));
	add_entropy8(Uint8(value>>40));
	add_entropy8(Uint8(value>>48));
	add_entropy8(Uint8(value>>56));
}

void PractRand::RNGs::Polymorphic::sha2_based_pool::flush_buffers() {
	if (input_buffer_left != INPUT_BUFFER_SIZE) empty_input_buffer();
	if (output_buffer_left != 0) output_buffer_left = 0;//refill_output_buffer();
}

void PractRand::RNGs::Polymorphic::sha2_based_pool::seed(Uint64 s) {
	unsigned long i;
	for (i = 0; i < 8; i++) state[i] = Uint8(s >> (i*8));
	for (; i < STATE_SIZE; i++) state[i] = 0;
	input_buffer_left = 128;
	output_buffer_left = 0;
	state_phase = 0;
}
void PractRand::RNGs::Polymorphic::sha2_based_pool::reset_state() {
	seed(Uint64(0));
}
void PractRand::RNGs::Polymorphic::sha2_based_pool::walk_state(StateWalkingObject *walker) {
	for (int i = 0; i < STATE_SIZE; i++) walker->handle(state[i]);
	for (int i = 0; i < 128; i++) walker->handle(input_buffer[i]);
	for (int i = 0; i < 64; i++) walker->handle(output_buffer[i]);
	walker->handle(input_buffer_left);
	walker->handle(output_buffer_left);
	walker->handle(state_phase);
	if (!input_buffer_left || input_buffer_left > INPUT_BUFFER_SIZE) input_buffer_left = INPUT_BUFFER_SIZE;
	if (output_buffer_left > OUTPUT_BUFFER_SIZE) output_buffer_left = OUTPUT_BUFFER_SIZE;
	if (state_phase >= STATE_SIZE) state_phase %= STATE_SIZE;
}
void PractRand::RNGs::Polymorphic::sha2_based_pool::empty_input_buffer() {
	if (input_buffer_left == INPUT_BUFFER_SIZE) return;
	state_phase++;
	if (state_phase >= STATE_SIZE) state_phase -= STATE_SIZE;
	PractRand::Crypto::SHA2_512 sha2;
	sha2.handle_input(&state[0], STATE_SIZE);
	sha2.handle_input(
		&input_buffer[input_buffer_left], 
		INPUT_BUFFER_SIZE - input_buffer_left);
	sha2.finish(&state[(state_phase&1) ? (STATE_SIZE-64) : 0] );
	input_buffer_left = INPUT_BUFFER_SIZE;
}
void PractRand::RNGs::Polymorphic::sha2_based_pool::refill_output_buffer() {
	PractRand::Crypto::SHA2_512 sha2;
	sha2.handle_input(&state[0], STATE_SIZE);
	sha2.finish(&output_buffer[0]);

	for (int i = 0; i < 64; i++) {
		state_phase++;
		while (state_phase >= STATE_SIZE) state_phase -= STATE_SIZE;
		state[state_phase] ^= output_buffer[i];
	}
	output_buffer_left = 64;
}
