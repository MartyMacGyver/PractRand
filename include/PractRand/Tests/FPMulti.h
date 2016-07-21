namespace PractRand {
	namespace Tests {
		class FPMulti : public TestBaseclass {
		protected:
			enum {
				EXP_BITS = 6,
				BASE_SIG_BITS = 12,
				MAX_EXP = 64 - BASE_SIG_BITS,
				FREQ_SIG_BITS = 12,
				GAP_SIG_BITS = 8,
				SEQ_SIG_BITS = 1,
				SEQ_N = 12,

				//INTER_SEQ_MIN_E = 8,
				//				DIRECTION = 0 //0 = low to high, 1 = high to low
			};
			//static unsigned long count_leading_zeroes32( Uint32 value );
			const int stride_bits_L2, skip_platters;
			struct Platter {
				FixedSizeCount<Uint16, 1 << (SEQ_SIG_BITS * SEQ_N)> seq_count;
				Uint32 seq_current;
				Uint32 history[1 << GAP_SIG_BITS];//relative to this platter only
				double history_sum;
				Uint64 history2[1 << GAP_SIG_BITS];//global - implementation is not efficient
				double history2_sum;
				double expected_gap2;
				Uint64 total_count;
				FixedSizeCount<Uint16, 1 << FREQ_SIG_BITS> freq_count;
				void reset();
			};
			Platter platter[MAX_EXP + 1];
			Uint64 base_position;//change to global_position, update continually
			void process(Platter &p, Uint32 sig, Sint32 pos);
		public:
			FPMulti(int stride_bits_L2_ = 3, int skip_platters_ = 6);
			virtual void init(PractRand::RNGs::vRNG *known_good);
			virtual void deinit();
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);
			//virtual double get_result();
			//	virtual double result_to_pvalue ( Uint64 blocks, double r );

			virtual void test_blocks(TestBlock *data, int numblocks);
		};
	}//Tests
}//PractRand
