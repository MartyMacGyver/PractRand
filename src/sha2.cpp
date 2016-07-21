#include <cstring>
#include "PractRand/config.h"
#include "PractRand/endian.h"
#include "PractRand/sha2.h"

namespace PractRand {
	namespace Crypto {
		class SHA2_512_constants {
		public:
			typedef Uint64 Word;
			enum { ROUNDS = 80 };
			enum { INPUT_WORDS = 16 };
			enum { OUTPUT_WORDS = 8 };
			enum { INPUT_SIZE = INPUT_WORDS * sizeof(Word) };
#ifdef PRACTRAND_TARGET_IS_LITTLE_ENDIAN
			enum { REVERSE_ENDIANNESS = 1 };
#else
			enum { REVERSE_ENDIANNESS = 0 };
#endif
			enum {WORD_BITS = 8 * sizeof(Word)};
			static const Uint64 round_constants[ROUNDS];
			static const Uint64 initial_values[8];
			static inline Word PREPROCESS_SHIFT_S0_1(Word value) {
				enum {SHIFT=1};
				return (value >> SHIFT) | (value << (WORD_BITS-SHIFT));}
			static inline Word PREPROCESS_SHIFT_S0_2(Word value) {
				enum {SHIFT=8};
				return (value >> SHIFT) | (value << (WORD_BITS-SHIFT));}
			static inline Word PREPROCESS_SHIFT_S0_3(Word value) {
				enum {SHIFT=7};
				return value >> SHIFT;}
			static inline Word PREPROCESS_SHIFT_S1_1(Word value) {
				enum {SHIFT=19};
				return (value >> SHIFT) | (value << (WORD_BITS-SHIFT));}
			static inline Word PREPROCESS_SHIFT_S1_2(Word value) {
				enum {SHIFT=61};
				return (value >> SHIFT) | (value << (WORD_BITS-SHIFT));}
			static inline Word PREPROCESS_SHIFT_S1_3(Word value) {
				enum {SHIFT=6};
				return value >> SHIFT;}
			static Word MAINLOOP_SHIFT_S0_1(Word value) {
				enum {SHIFT=28};
				return (value >> SHIFT) | (value << (WORD_BITS-SHIFT));}
			static inline Word MAINLOOP_SHIFT_S0_2(Word value) {
				enum {SHIFT=34};
				return (value >> SHIFT) | (value << (WORD_BITS-SHIFT));}
			static inline Word MAINLOOP_SHIFT_S0_3(Word value) {
				enum {SHIFT=39};
				return (value >> SHIFT) | (value << (WORD_BITS-SHIFT));}
			static inline Word MAINLOOP_SHIFT_S1_1(Word value) {
				enum {SHIFT=14};
				return (value >> SHIFT) | (value << (WORD_BITS-SHIFT));}
			static inline Word MAINLOOP_SHIFT_S1_2(Word value) {
				enum {SHIFT=18};
				return (value >> SHIFT) | (value << (WORD_BITS-SHIFT));}
			static inline Word MAINLOOP_SHIFT_S1_3(Word value) {
				enum {SHIFT=41};
				return (value >> SHIFT) | (value << (WORD_BITS-SHIFT));}
		};
		const Uint64 SHA2_512_constants::initial_values[8] = {
			0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL,
			0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
			0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL,
			0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL
		};
		const Uint64 SHA2_512_constants::round_constants[SHA2_512_constants::ROUNDS] = {
			0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL, 0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
			0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL, 0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
			0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL, 0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
			0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL, 0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
			0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL, 0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
			0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL, 0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
			0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL, 0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
			0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL, 0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
			0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL, 0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
			0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL, 0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
		};
		void SHA2_512::reset() {
			typedef SHA2_512_constants Constants;
			for (int i = 0; i < 8; i++) state[i] = Constants::initial_values[i];
			length = 0;
			leftover_input_bytes = 0;
		}
		void SHA2_512::process_block() {
			typedef SHA2_512_constants Constants;
			//do preprocessing on the block
			Word preprocessed[Constants::ROUNDS];
			long i;
			for (i = 0; i < Constants::INPUT_WORDS; i++) preprocessed[i] = input_buffer.as_word[i];
			for (i = Constants::INPUT_WORDS; i < Constants::ROUNDS; i++) {
				Word s0, s1;
				enum {
					OFFS_A = Constants::INPUT_WORDS - 1,
					OFFS_B = 2,
					OFFS_C = Constants::INPUT_WORDS - 0,
					OFFS_D = Constants::INPUT_WORDS/2 - 1,
				};
				s0 = 
					Constants::PREPROCESS_SHIFT_S0_1(preprocessed[i-OFFS_A]) ^
					Constants::PREPROCESS_SHIFT_S0_2(preprocessed[i-OFFS_A]) ^
					Constants::PREPROCESS_SHIFT_S0_3(preprocessed[i-OFFS_A]);
				s1 = 
					Constants::PREPROCESS_SHIFT_S1_1(preprocessed[i-OFFS_B]) ^
					Constants::PREPROCESS_SHIFT_S1_2(preprocessed[i-OFFS_B]) ^
					Constants::PREPROCESS_SHIFT_S1_3(preprocessed[i-OFFS_B]);
				preprocessed[i] = preprocessed[i-OFFS_C] + s0 + preprocessed[i-OFFS_D] + s1;
			}
			//preprocessing complete
			//prepare for main loop
			Word temp_state[8];
			for (i = 0; i < 8; i++) temp_state[i] = state[i];
			//preparations complete
			//starting main loop
			for (i = 0; i < Constants::ROUNDS; i+=2) {
				Word s0, s1, maj, t1, t2, ch;
				s0 = 
					Constants::MAINLOOP_SHIFT_S0_1(temp_state[0]) ^
					Constants::MAINLOOP_SHIFT_S0_2(temp_state[0]) ^
					Constants::MAINLOOP_SHIFT_S0_3(temp_state[0]);
				s1 = 
					Constants::MAINLOOP_SHIFT_S1_1(temp_state[4]) ^
					Constants::MAINLOOP_SHIFT_S1_2(temp_state[4]) ^
					Constants::MAINLOOP_SHIFT_S1_3(temp_state[4]);
				maj = (temp_state[0] & temp_state[1]) ^ (temp_state[0] & temp_state[2]) ^ (temp_state[1] & temp_state[2]);
				t2 = s0 + maj;
				ch = (temp_state[4] & temp_state[5]) ^ ((~temp_state[4]) & temp_state[6]);
				t1 = temp_state[7] + s1 + ch + Constants::round_constants[i] + preprocessed[i];
				/*temp_state[7] = temp_state[6];
				temp_state[6] = temp_state[5];
				temp_state[5] = temp_state[4];
				temp_state[4] = temp_state[3] + t1;
				temp_state[3] = temp_state[2];
				temp_state[2] = temp_state[1];
				temp_state[1] = temp_state[0];
				temp_state[0] = t1 + t2;*/

				//loop unrolled x2

				Word n0 = t1 + t2;
				Word n4 = temp_state[3] + t1;
				s0 = 
					Constants::MAINLOOP_SHIFT_S0_1(n0) ^
					Constants::MAINLOOP_SHIFT_S0_2(n0) ^
					Constants::MAINLOOP_SHIFT_S0_3(n0);
				s1 = 
					Constants::MAINLOOP_SHIFT_S1_1(n4) ^
					Constants::MAINLOOP_SHIFT_S1_2(n4) ^
					Constants::MAINLOOP_SHIFT_S1_3(n4);
				maj = (n0 & temp_state[0]) ^ (n0 & temp_state[1]) ^ (temp_state[0] & temp_state[1]);
				t2 = s0 + maj;
				ch = (n4 & temp_state[4]) ^ ((~n4) & temp_state[5]);
				t1 = temp_state[6] + s1 + ch + Constants::round_constants[i+1] + preprocessed[i+1];
				temp_state[7] = temp_state[5];
				temp_state[6] = temp_state[4];
				temp_state[5] = n4;
				temp_state[4] = temp_state[2] + t1;
				temp_state[3] = temp_state[1];
				temp_state[2] = temp_state[0];
				temp_state[1] = n0;
				temp_state[0] = t1 + t2;
			}
			//main loop complete
			//update cumulative state
			for (i = 0; i < 8; i++) state[i] += temp_state[i];
			//finished with block
		}
		SHA2_512::Word SHA2_512::endianness_word(Word a) {
			typedef SHA2_512_constants Constants;
			if (Constants::REVERSE_ENDIANNESS) {
				if (0) ;
				else if (sizeof(Word)==8) return invert_endianness64(Uint64(a));
				else if (sizeof(Word)==4) return invert_endianness32(Uint32(a));
				else if (sizeof(Word)==2) return invert_endianness16(Uint16(a));
			}
			return a;
		}
		void SHA2_512::endianness_state() {
			typedef SHA2_512_constants Constants;
			if (Constants::REVERSE_ENDIANNESS) {
				for (int i = 0; i < Constants::OUTPUT_WORDS; i++) {
					state[i] = endianness_word(state[i]);
				}
			}
		}
		void SHA2_512::endianness_input() {
			typedef SHA2_512_constants Constants;
			if (Constants::REVERSE_ENDIANNESS) {
				for (int i = 0; i < Constants::INPUT_WORDS; i++) {
					input_buffer.as_word[i] = endianness_word(input_buffer.as_word[i]);
				}
			}
		}
		void SHA2_512::handle_input ( const Uint8 *input, unsigned long input_length ) {
			typedef SHA2_512_constants Constants;
			unsigned long input_left = input_length;
			length += input_length;
			unsigned long space_left = Constants::INPUT_SIZE - leftover_input_bytes;
			//start in middle of block, finish it
			if (input_left >= space_left) {
				std::memcpy(input_buffer.as_byte + leftover_input_bytes, input, space_left);
				input += space_left;
				input_left -= space_left;
				endianness_input();
				process_block();
				leftover_input_bytes = 0;
			}
			//start at begining of block, finish it
			while (input_left >= Constants::INPUT_SIZE) {
				std::memcpy(input_buffer.as_byte, input, Constants::INPUT_SIZE);
				input += Constants::INPUT_SIZE;
				input_left -= Constants::INPUT_SIZE;
				endianness_input();
				process_block();
			}
			//buffer any leftovers
			if (input_left) {
				std::memcpy(input_buffer.as_byte + leftover_input_bytes, input, input_left);
				leftover_input_bytes += (int)input_left;
			}
		}
		void SHA2_512::process_final_block () {
			typedef SHA2_512_constants Constants;
			enum { EXTRAS = sizeof(Word) * 2 + 1 };
			input_buffer.as_byte[leftover_input_bytes] = 0x80;
			std::memset( input_buffer.as_byte + leftover_input_bytes + 1, 0, Constants::INPUT_SIZE - leftover_input_bytes - 1);
			if (leftover_input_bytes > Constants::INPUT_SIZE - EXTRAS) {
				endianness_input();
				process_block();
				std::memset(input_buffer.as_byte, 0, Constants::INPUT_SIZE - EXTRAS + 1);
			}
			input_buffer.as_word[Constants::INPUT_WORDS-1] = endianness_word(Word(length << 3));
			if (sizeof(Word) < sizeof(Uint64)) {
				input_buffer.as_word[Constants::INPUT_WORDS-2] = 
					endianness_word(Word(length>>(sizeof(Word)*8-3)));
			}
			else input_buffer.as_word[Constants::INPUT_WORDS-2] = 0;
			endianness_input();
			process_block();
		}
		void SHA2_512::finish (Uint8 destination[SHA2_512::RESULT_LENGTH]) {
			typedef SHA2_512_constants Constants;
			process_final_block();
			endianness_state();
			std::memcpy(destination, state, RESULT_LENGTH);
		}
	}//Crypto
}//PractRand


