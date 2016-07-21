#ifndef __PRACTRAND_SHA2_H__
#define __PRACTRAND_SHA2_H__

#ifndef __PRACTRAND_CONFIG_H__
#include "PractRand/config.h"
#endif //__PRACTRAND_CONFIG_H__

namespace PractRand {
	namespace Crypto {
		class SHA2_512 {
			typedef Uint64 Word;
			Word state[8];
			Uint64 length;
			union InputBlock {
				Word as_word[16];
				Uint8 as_byte[16 * sizeof(Word)];
			};
			InputBlock input_buffer;
			unsigned long leftover_input_bytes;
			void process_block();
			void process_final_block();
			Word endianness_word(Word);
			void endianness_input();
			void endianness_state();
		public:
			enum {RESULT_LENGTH = 64};
			void reset();
			void handle_input(const Uint8 *input, unsigned long length);
			void finish(Uint8 destination[RESULT_LENGTH]);
			SHA2_512() {reset();}
		};
	}//Crypto
}//PractRand

#endif /* __PRACTRAND_SHA2_H__ */

