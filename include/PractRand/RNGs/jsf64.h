namespace PractRand {
	namespace RNGs {
		namespace Raw {
			//implemented in RNGs/jsf.cpp
			class jsf64 {
			public:
				enum {
					OUTPUT_TYPE = OUTPUT_TYPES::NORMAL_1,
					OUTPUT_BITS = 64,
					FLAGS = FLAG::USES_SPECIFIED | FLAG::ENDIAN_SAFE
				};
			protected:
				Uint64 a, b, c, d;
			public:
				Uint64 raw64();
				void seed(Uint64 s);
				void seed_fast(Uint64 s);
				void walk_state(StateWalkingObject *walker);
				//static void self_test();
			};
		}
		
		namespace Polymorphic {
			class jsf64 : public vRNG64 {
				PRACTRAND__POLYMORPHIC_RNG_BASICS_H(jsf64)
				void seed(Uint64 s);
				void seed_fast(Uint64 s);
			};
		}
		PRACTRAND__LIGHT_WEIGHT_RNG(jsf64)
	}
}
