namespace PractRand {
	namespace RNGs {
		namespace Raw {
			class efiix64x48 {
			public:
				enum {
					OUTPUT_TYPE = OUTPUT_TYPES::NORMAL_1,
					OUTPUT_BITS = 64,
					FLAGS = FLAG::USES_SPECIFIED | FLAG::USES_INDIRECTION | FLAG::USES_CYCLIC_BUFFER | FLAG::ENDIAN_SAFE
				};
			protected:
				typedef Uint64 Word;
				enum {
					ITERATION_SIZE_L2 = 5,
					ITERATION_SIZE = 1 << ITERATION_SIZE_L2,
					INDIRECTION_SIZE_L2 = 4,
					INDIRECTION_SIZE = 1 << INDIRECTION_SIZE_L2
				};
				Word indirection_table[INDIRECTION_SIZE], iteration_table[ITERATION_SIZE];
				Word i, a, b, c;
			public:
				~efiix64x48();
				Uint64 raw64();
				void seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4);
				void seed(Uint64 s) { seed(s, s, s, s); }
				void seed(vRNG *source_rng);
				void walk_state(StateWalkingObject *walker);
			};
		}

		namespace Polymorphic {
			class efiix64x48 : public vRNG64 {
				PRACTRAND__POLYMORPHIC_RNG_BASICS_H(efiix64x48)
				void seed(Uint64 s1, Uint64 s2, Uint64 s3, Uint64 s4);
				void seed(Uint64 s);
				void seed(vRNG *source_rng);
			};
		}
		PRACTRAND__LIGHT_WEIGHT_RNG(efiix64x48)
	}
}
