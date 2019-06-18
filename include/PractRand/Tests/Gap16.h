namespace PractRand {
	namespace Tests {
		class Gap16 : public TestBaseclass {
		public:
			virtual void init( PractRand::RNGs::vRNG *known_good );
			virtual std::string get_name() const;// {return std::string("Gap16");}
			//virtual double get_result();
			//virtual double result_to_pvalue ( Uint64 blocks, double r );
			virtual void get_results ( std::vector<TestResult> &results );

			virtual void test_blocks(TestBlock *data, int numblocks);
		protected:
			enum { 
				SIZE1 = 1<<18, //handles the common case, tracks sets of (1 << SET1_SHIFT) gaps
				SIZE2 = 1<<17, //handles the uncommon cases, tracks each set of (1 << SET2_SHIFT) gaps
				SIZE3 = 1<<17, //handles rare cases, tracks each set of (1 << SET3_SHIFT) gaps (VERY rare cases go to extreme_lags instead)
				SET1_SHIFT = 1,
				SET2_SHIFT = 2,
				SET3_SHIFT = 3
			};
			FixedSizeCount<Uint8, SIZE1 + SIZE2 + SIZE3> counts;
			std::vector<Uint32> extreme_lags;
			void increment_lag(Uint32 lag);
			bool autofail;
			Uint32 last[65536];
			int warmup;
		};
		class Rep16 : public TestBaseclass {
		public:
			virtual void init(PractRand::RNGs::vRNG *known_good);
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);

			virtual void test_blocks(TestBlock *data, int numblocks);
		protected:
			FixedSizeCount<Uint8, 65536 * 2> counts;
		};
	}//Tests
}//PractRand
