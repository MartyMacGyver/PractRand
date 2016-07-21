namespace PractRand {
	namespace RNGs {
		namespace Raw {
			//implemented in RNGs/xsm.cpp
			class xsm64 {
			public:
				enum {
					OUTPUT_TYPE = OUTPUT_TYPES::NORMAL_1,
					OUTPUT_BITS = 64,
					FLAGS = FLAG::ENDIAN_SAFE | FLAG::USES_SPECIFIED | FLAG::USES_MULTIPLICATION | FLAG::SUPPORTS_FASTFORWARD | FLAG::OUTPUT_IS_HASHED
				};
			protected:
				Uint64 lcg_low, lcg_high, lcg_adder, history;
				void step_backwards();
			public:
				Uint64 raw64();
				void seed(Uint64 s);
				void walk_state(StateWalkingObject *walker);
				void seek_forward (Uint64 how_far_low64, Uint64 how_far_high64);
				void seek_backward(Uint64 how_far_low64, Uint64 how_far_high64);
				//static void self_test();
			};
		}
		
		namespace Polymorphic {
			class xsm64 : public vRNG64 {
				PRACTRAND__POLYMORPHIC_RNG_BASICS_H(xsm64)
				void seed(Uint64 s);
				virtual void seek_forward128 (Uint64 how_far_low64, Uint64 how_far_high64);
				virtual void seek_backward128(Uint64 how_far_low64, Uint64 how_far_high64);
			};
		}
		PRACTRAND__LIGHT_WEIGHT_RNG(xsm64)
	}
}
