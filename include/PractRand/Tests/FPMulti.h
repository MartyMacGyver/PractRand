namespace PractRand {
	namespace Tests {
		class FPMulti : public TestBaseclass {
		protected:
			typedef Uint32 COUP_WORD;//must be 4 or 8 bytes
			enum {
				EXP_BITS = 6,
				BASE_SIG_BITS = 12,
				MAX_EXP = 64 - BASE_SIG_BITS,
				FREQ_SIG_BITS = 12,
				COUP_SIG_BITS = 12,
				GAP_SIG_BITS = 8,
				SEQ_SIG_BITS = 1,
				SEQ_N = 12,

				//INTER_SEQ_MIN_E = 8,
				//				DIRECTION = 0 //0 = low to high, 1 = high to low
				COUP_WORD_SIZE = 8 * sizeof(COUP_WORD),
				COUP_WORD_SIZE_L2 = sizeof(COUP_WORD) == 4 ? 5 : 6,
				COUP_MASK_SIZE = 1 << (COUP_SIG_BITS - COUP_WORD_SIZE_L2),
			};
			//static unsigned long count_leading_zeroes32( Uint32 value );
			const int stride_bits_L2, skip_platters;
			bool autofail;
			struct Platter {
				FixedSizeCount<Uint16, 1 << (SEQ_SIG_BITS * SEQ_N)> seq_count;
				Uint32 seq_current;
				Uint32 history1[1 << GAP_SIG_BITS];//relative to this platter only
				Uint64 history2[1 << GAP_SIG_BITS];//global - implementation is not efficient
				Uint64 total_count;
				double gap_sum1;//our gap test is actually closer to Maurer's Universal Statitical Test, though they're basically the same thing
				Uint64 gap_negative_count1;
				double gap_sum2;//using history2 instead of history1
				Uint64 gap_negative_count2;
				double expected_gap2;
				FixedSizeCount<Uint16, 1 << FREQ_SIG_BITS> freq_count;
				COUP_WORD coup_mask[COUP_MASK_SIZE];
				Uint64 last_coup;
				FixedSizeCount<Uint16, 1024> coup_count;
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

			virtual void test_blocks(TestBlock *data, int numblocks);
		};
	}//Tests
}//PractRand
