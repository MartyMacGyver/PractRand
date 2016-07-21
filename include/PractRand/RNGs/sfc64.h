namespace PractRand {
	namespace RNGs {
		namespace Raw {
			//implemented in RNGs/sfc.cpp
			class sfc64 {
			public:
				enum {
					OUTPUT_TYPE = OUTPUT_TYPES::NORMAL_1,
					OUTPUT_BITS = 64,
					FLAGS = FLAG::ENDIAN_SAFE | FLAG::USES_SPECIFIED
				};
			protected:
				Uint64 a, b, c, counter;
			public:
				Uint64 raw64();
				void seed(Uint64 s);
				void seed_fast(Uint64 s);
				void seed(Uint64 s1, Uint64 s2, Uint64 s3);
				void walk_state(StateWalkingObject *walker);
			};
		}
		
		namespace Polymorphic {
			class sfc64 : public vRNG64 {
				PRACTRAND__POLYMORPHIC_RNG_BASICS_H(sfc64)
				void seed(Uint64 s);
				void seed_fast(Uint64 s);
				void seed(Uint64 s1, Uint64 s2, Uint64 s3);
			};
		}
		PRACTRAND__LIGHT_WEIGHT_RNG(sfc64)
	}
}
