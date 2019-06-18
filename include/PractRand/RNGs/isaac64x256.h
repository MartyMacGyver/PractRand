namespace PractRand {
	namespace RNGs {
		namespace Raw {
			class isaac64x256 {
			public:
				enum {
					OUTPUT_TYPE = OUTPUT_TYPES::NORMAL_1,
					OUTPUT_BITS = 64,
					FLAGS = FLAG::CRYPTOGRAPHIC_SECURITY | FLAG::OUTPUT_IS_BUFFERED | FLAG::ENDIAN_SAFE
				};
			protected:
				enum {
					SIZE_L2 = 8,
					SIZE = 1 << SIZE_L2
				};
				Uint64 results[SIZE];
				Uint64 state[SIZE];
				Uint64 a, b, c;
				Uint32 used;
				void _advance_state();
				void _seed(bool flag=true);
			public:
				~isaac64x256();
				void flush_buffers() {used = SIZE;}
				Uint64 raw64() {//LOCKED, do not change
					//note: this walks the buffer in the same direction as the buffer is filled
					//  whereas (some of) Bob Jenkins original code walked the buffer backwards
					if ( used >= SIZE ) _advance_state();
					return results[used++];
				}
				void seed(Uint64 s);
				void seed(Uint64 s[256]);
				void seed(vRNG *seeder_rng);
				void walk_state(StateWalkingObject *walker);
				//static void self_test();
			};
		}
		
		namespace Polymorphic {
			class isaac64x256 : public vRNG64 {
				PRACTRAND__POLYMORPHIC_RNG_BASICS_H(isaac64x256)
				void seed(Uint64 s);
				void seed(vRNG *seeder_rng);
				void flush_buffers();
			};
		}
		PRACTRAND__LIGHT_WEIGHT_RNG(isaac64x256)
	}
}
