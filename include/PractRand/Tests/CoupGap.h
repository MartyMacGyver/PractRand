namespace PractRand {
	namespace Tests {
		class CoupGap : public TestBaseclass {
			//enum {MAX_OLDEST_AGE = 256 * 16};
			enum {MAX_CURRENT_AGE = 256 * 2};

			//low8 = current_sym, high8 = oldest_sym
			FixedSizeCount<Uint8, 256*256> count_syms_by_oldest_sym;

			//low8 = oldest_sym, high8 = current_gap
//			FixedSizeCount<Uint8, 256*MAX_CURRENT_AGE> count_gaps_by_oldest_sym;

//			Uint32 last_sym_pos[256];
			unsigned long oldest_sym;
			unsigned long youngest_sym;
			Uint8 next_younger_sym[256];
			bool sym_has_appeared[256];
			int symbols_ready;

			Uint32 autofail;
			Uint64 blocks;
		public:
			CoupGap() {}

			virtual void init( RNGs::vRNG *known_good );
			virtual std::string get_name() const;
			virtual void get_results ( std::vector<TestResult> &results );
			virtual void test_blocks(TestBlock *data, int numblocks);
		};
	}//Tests
}//PractRand
