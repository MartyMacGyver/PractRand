namespace PractRand {
	namespace Tests {
		class Pat5 : public TestBaseclass {
		public:
			Pat5();
			virtual void init(PractRand::RNGs::vRNG *known_good);
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);

			virtual void test_blocks(TestBlock *data, int numblocks);

		protected:
			typedef Uint32 Word;
			enum {
				WORD_BITS = sizeof(Word) * 8,
				ZERO_FILTER_BITS = 20,//should not exceed WORD_BITS - PATTERN_INDEX_BITS - (1 << PRIMARY_WORD_DISTANCE_BITS) + 1
				PATTERN_INDEX_BITS = 3,
				PRIMARY_WORD_DISTANCE_BITS = 2,
				PRIMARY_WORD_DISTANCE_EXTRA_BITS = 0,
				NUM_SECONDARY_WORDS = 1,
				SECONDARY_WORD_DISTANCE_BITS = 6,
				SECONDARY_WORD_DISTANCE_EXTRA_BITS = 0,
				NUM_TERTIARY_WORDS = 2,
				TERTIARY_WORD_DISTANCE_BITS = 5,
				TERTIARY_WORD_DISTANCE_EXTRA_BITS = 1,
				NUM_QUATERNARY_WORDS = 2,
				QUATERNARY_WORD_DISTANCE_BITS = 3,
				QUATERNARY_WORD_DISTANCE_EXTRA_BITS = 3,
				TABLE_SIZE_L2 = PATTERN_INDEX_BITS + PRIMARY_WORD_DISTANCE_BITS + SECONDARY_WORD_DISTANCE_BITS + TERTIARY_WORD_DISTANCE_BITS + QUATERNARY_WORD_DISTANCE_BITS,
				PATTERN_WIDTH = 1 + 2 * (NUM_SECONDARY_WORDS + NUM_TERTIARY_WORDS + NUM_QUATERNARY_WORDS),
			};
			class Pattern {
			public:
				Word base_pattern[PATTERN_WIDTH];
				Word _padding;
				Sint64 total_count;
			};
			//PractRand::RNGs::Raw::arbee internal_rng;
			//state:
			Pattern patterns[1 << PATTERN_INDEX_BITS];
			FixedSizeCount<Uint8, 1 << TABLE_SIZE_L2> counts;
			//Uint64 lifespan;
			//internal helpers:
			int transform_bitcount_primary(int bit_count) const;
			int transform_bitcount_secondary(int bit_count) const;
			int transform_bitcount_tertiary(int bit_count) const;
		};
	}//Tests
}//PractRand
