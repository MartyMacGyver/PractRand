namespace PractRand {
	namespace RNGs {
		namespace Raw {
			class efiix32x384 {
			public:
				enum {
					OUTPUT_TYPE = OUTPUT_TYPES::NORMAL_1,
					OUTPUT_BITS = 32,
					FLAGS = FLAG::USES_SPECIFIED | FLAG::USES_INDIRECTION | FLAG::USES_CYCLIC_BUFFER | FLAG::ENDIAN_SAFE
				};
			protected:
				typedef Uint32 Word;
				enum {
					ITERATION_SIZE = 128,
					INDIRECTION_SIZE = 256
				};
				Word iteration_table[ITERATION_SIZE], indirection_table[INDIRECTION_SIZE];
				Word i, a, b, c;
			public:
				~efiix32x384();
				Uint32 raw32();
				void seed(const Word *seeds, int num_seeds, int seeding_quality=4);
				void seed(Uint64 s) {enum {N=sizeof(s)/sizeof(Word)}; Word sa[N]; for (int i = 0; i < N; i++) sa[i] = Word(s>>(i*8*sizeof(Word))); seed(&sa[0], N);}
				void seed(vRNG *source_rng);
				void walk_state(StateWalkingObject *walker);
			};
		}
		
		namespace Polymorphic {
			class efiix32x384 : public vRNG32 {
				PRACTRAND__POLYMORPHIC_RNG_BASICS_H(efiix32x384)
				void seed(const Uint32 *seeds, int num_seeds, int seeding_quality=4) {implementation.seed(seeds, num_seeds, seeding_quality);}
				void seed(Uint64 s);
				void seed(vRNG *source_rng);
			};
		}
		PRACTRAND__LIGHT_WEIGHT_RNG(efiix32x384)
	}
}
