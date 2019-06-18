namespace PractRand {
	namespace Tests {
		//class BitMatrix;
		class BRank : public TestBaseclass {
		public:
			BRank ( 
				Uint32 rate_hl2_ // 2 * log2(time units per KB)
			);
			virtual void init( PractRand::RNGs::vRNG *known_good );
			virtual std::string get_name() const;
			virtual void get_results ( std::vector<TestResult> &results );
			virtual void deinit();

			virtual void test_blocks(TestBlock *data, int numblocks);
		protected:
			Uint32 rate_hl2;
			Uint64 rate;
			
			virtual void pick_next_size();
			virtual void finish_matrix();

			Uint64 saved_time;

			//partially complete matrix:
			BitMatrix *in_progress;
			Uint32 blocks_in_progress;
			int size_index;//which PerSize is active atm?

			//stats:
			class PerSize {
			public:
				//PerSize() {}
				Uint32 size;
				Uint64 time_per;
				Uint64 total;
				enum { NUM_COUNTS = 10, MAX_OUTLIERS = 100 };
				Uint64 counts[NUM_COUNTS];

				Uint64 outliers_overflow;
				std::vector<Uint32> outliers;
				void reset();
			};
			std::vector<PerSize> ps;

		};
	}//Tests
}//PractRand
