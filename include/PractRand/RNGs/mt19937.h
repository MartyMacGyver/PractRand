namespace PractRand {
	namespace RNGs {
		namespace Raw {
			class mt19937 {
			public:
				enum {
					OUTPUT_TYPE = OUTPUT_TYPES::NORMAL_1,
					OUTPUT_BITS = 32,
					FLAGS = FLAG::OUTPUT_IS_BUFFERED | FLAG::OUTPUT_IS_HASHED | FLAG::ENDIAN_SAFE
				};
			protected:
				enum {ARRAY_SIZE=624, OFFSET=397};
				Uint32 state[ARRAY_SIZE];
				Uint32 used;
				void _advance_state();
			public:
				void flush_buffers() {used = ARRAY_SIZE;}
				Uint32 raw32();
				void walk_state(StateWalkingObject *walker);

				//seeds < 2**32 use the standard MT19937 seeding algorithm
				//seeds >= 2**32 use a nonstandard MT19937 seeding algorithm
				void seed(Uint64 s);
				void seed(Uint32 s[], int seed_length);//alternate seeding algorithm added to MT in 2002

				Uint32 untempered_raw32() {
					if ( used >= ARRAY_SIZE ) {
						_advance_state();
						return state[used++];
					}
					else return state[used++];
				}
				static void self_test();
			};
		}
		
		namespace Polymorphic {
			class mt19937 : public vRNG32 {
				PRACTRAND__POLYMORPHIC_RNG_BASICS_H(mt19937)
				mt19937 (Uint32 s[], int seed_length) {seed(s, seed_length);}
				void seed(Uint64 s);
				void seed(Uint32 s[], int seed_length);//alternate seeding algorithm added to MT in 2002
				void flush_buffers();
			};
		}
		PRACTRAND__LIGHT_WEIGHT_RNG(mt19937)
	}
}
