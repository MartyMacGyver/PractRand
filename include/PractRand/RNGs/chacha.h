namespace PractRand {
	namespace RNGs {
		namespace Raw {
			//implemented in RNGs/chacha.cpp
#ifdef PRACTRAND_ALIGN_128
			class __declspec(align(16)) chacha {
#else
			class chacha {
#endif
			public:
				enum {
					OUTPUT_TYPE = OUTPUT_TYPES::NORMAL_1,
					OUTPUT_BITS = 32,
					FLAGS = FLAG::USES_SPECIFIED | FLAG::OUTPUT_IS_BUFFERED | FLAG::OUTPUT_IS_HASHED | FLAG::ENDIAN_SAFE | FLAG::CRYPTOGRAPHIC_SECURITY
				};
			protected:
				Uint32 outbuf[16];
				Uint32 state[12];
				Uint32 used;
				Uint32 position_overflow;
				Uint8 rounds;
				bool extend_cycle;//true allows carries from the position field to overflow in to the upper word of the IV
				bool short_seed;
				Uint8 padding[5];//just to make the size a round number

				void _advance_1();
				//void _reverse_1();
				void _set_position(Uint64 low, Uint64 high);
				void _get_position(Uint64 &low, Uint64 &high) const;

				void _core();
				Uint32 _refill_and_raw32();
			public:
				chacha() : rounds(20) {}
				~chacha();
				Uint32 raw32() {
					if (used < 16) return outbuf[used++];
					return _refill_and_raw32();
				}
				void seed(Uint64 s);
				void seed( const Uint32 seed_and_iv[10], bool extend_cycle_ = false );
				void seed_short( const Uint32 seed_and_iv[6], bool extend_cycle_ = false );
				void walk_state(StateWalkingObject *walker);
				void seek_forward (Uint64 how_far_low, Uint64 how_far_high);
				void seek_backward(Uint64 how_far_low, Uint64 how_far_high);

				//normally rounds is 8, 12, or 20, but it can be anywhere from 1 to 255
				//the default is 20
				//the author recommends 8 for weak crypto or non-crypto, 12 for moderate crypto, and 20 for strong crypto
				//I'd say that you can go as low as 4 or 5 rounds without adversely effecting the output for non-cryptographic purposes
				//when using an odd number of rounds the final transposition gets skipped - this may or may not match other implementations
				//therefore it is recommended that only even numbers of rounds be used
				void set_rounds(int rounds_);
				int get_rounds() const {return rounds;}

				static void self_test();
			};
		}
		namespace Polymorphic {
			class chacha : public vRNG32 {
				PRACTRAND__POLYMORPHIC_RNG_BASICS_H(chacha)
				chacha(Uint32 seed_and_iv[10], bool extend_cycle_ = false) {seed(seed_and_iv, extend_cycle_);}
				void seed(Uint64 s);
				void seed(Uint32 seed_and_iv[10], bool extend_cycle_ = false);
				void seed_short(Uint32 seed_and_iv[6], bool extend_cycle_ = false);
				void seek_forward128 (Uint64 how_far_low64, Uint64 how_far_high64);
				void seek_backward128(Uint64 how_far_low64, Uint64 how_far_high64);

				//normally rounds is 8, 12, or 20, but lower and higher values are also possible
				//default is 20
				//for non-crypto applications, 4 rounds is sufficient to qualify for a 3 star quality rating, 6 rounds for a 5 star quality rating
				//for crypto applications, 8 rounds is sufficient to qualify for a 1 star crypto rating, 12 for a 3 star crypto rating, 20 for a 4 star crypto rating
				void set_rounds(int rounds_);
				int get_rounds() const;
			};
		}
		PRACTRAND__LIGHT_WEIGHT_RNG(chacha)
	}
}
