namespace PractRand {
	namespace RNGs {
		namespace Raw {
			class isaac32x256 {
			public:
				enum {
					OUTPUT_TYPE = OUTPUT_TYPES::NORMAL_1,
					OUTPUT_BITS = 32,
					FLAGS = FLAG::CRYPTOGRAPHIC_SECURITY | FLAG::OUTPUT_IS_BUFFERED | FLAG::ENDIAN_SAFE
				};
			protected:
				enum {
					SIZE_L2 = 8,
					SIZE = 1 << SIZE_L2
				};
				Uint32 results[SIZE];
				Uint32 used;
				Uint32 state[SIZE];
				Uint32 a, b, c;
				void _advance_state();
				void _seed(bool flag = true);
			public:
				~isaac32x256();
				void flush_buffers() {used = SIZE;}
				Uint32 raw32() {//LOCKED, do not change
					//note: this walks the buffer in the same direction as the buffer is filled
					//  whereas (some of) Bob Jenkins original code walked the buffer backwards
					if ( used >= SIZE ) _advance_state();
					return results[used++];
				}
				void seed(Uint64 s);
				void seed(Uint32 s[256]);
				void seed(vRNG *seeder_rng);
				void walk_state(StateWalkingObject *walker);
				static void self_test();
			};
		}
		
		namespace Polymorphic {
			class isaac32x256 : public vRNG32 {
				PRACTRAND__POLYMORPHIC_RNG_BASICS_H(isaac32x256)
				void seed(Uint64 s);
				void seed(vRNG *seeder_rng);
				void flush_buffers();
			};
		}
		PRACTRAND__LIGHT_WEIGHT_RNG(isaac32x256)
	}
}
