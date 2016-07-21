namespace PractRand {
	namespace RNGs {
		namespace Raw {
			//implemented in RNGs/sfc.cpp
			class sfc16 {
			public:
				enum {
					OUTPUT_TYPE = OUTPUT_TYPES::NORMAL_1,
					OUTPUT_BITS = 16,
					FLAGS = FLAG::ENDIAN_SAFE | FLAG::USES_SPECIFIED
				};
			protected:
				Uint16 a, b, c, counter;
			public:
				Uint16 raw16();
				void seed(Uint64 s);
				void seed_fast(Uint64 s);
				void seed(Uint16 s1, Uint16 s2, Uint16 s3);
				void walk_state(StateWalkingObject *walker);
			};
		}
		
		namespace Polymorphic {
			class sfc16 : public vRNG16 {
				PRACTRAND__POLYMORPHIC_RNG_BASICS_H(sfc16)
				void seed(Uint64 s);
				void seed_fast(Uint64 s);
				void seed(Uint16 s1, Uint16 s2, Uint16 s3);
			};
		}
		PRACTRAND__LIGHT_WEIGHT_RNG(sfc16)
	}
}
