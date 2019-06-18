namespace PractRand {
	namespace Tests {
		class DistFreq4 : public TestBaseclass {
		public:
			DistFreq4(int blocks_per_) : blocks_per(blocks_per_) {}
			virtual void init(PractRand::RNGs::vRNG *known_good);
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);

			virtual void test_blocks(TestBlock *data, int numblocks);

		protected:
			enum {
				ALIGNMENT1 = 4,//the alignments must be powers of 2
				ALIGNMENT2 = 4,
				// TODO: try it with a 3rd window
				SIZE1 = 4,
				SIZE2 = 4,
				POSITIONS1_L2 = 0,//	4,8:38		0,10:38		0,8(8):>39		0(8),8(8):>39		0,11:39
				POSITIONS2_L2 = 10,
				TOTAL_INDEX_BITS = SIZE1 + SIZE2 + POSITIONS1_L2 + POSITIONS2_L2
			};
			int blocks_till_next;
			int blocks_per;
			FixedSizeCount<Uint8, 1 << TOTAL_INDEX_BITS> counts;
		};
		class TripleFreq : public TestBaseclass {
		public:
			TripleFreq(int passes_at_once_, int blocks_per_pass_) : passes_at_once(passes_at_once_), blocks_per_pass(blocks_per_pass_) {}
			virtual void init(PractRand::RNGs::vRNG *known_good);
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);

			virtual void test_blocks(TestBlock *data, int numblocks);

		protected:
			enum {
				BASE_ALIGNMENT = 64,//don't change
				WINDOW_ALIGNMENT = 4,
				SIZE1 = 4,//sizes must be multiples of WINDOW_ALIGNMENT
				SIZE2 = 4,
				SIZE3 = 4,
				POSITIONS2_L2 = 6,
				POSITIONS3_L2 = 6,
				TOTAL_INDEX_BITS = SIZE1 + SIZE2 + SIZE3 + POSITIONS2_L2 + POSITIONS3_L2,
				REGION_INDEX_BITS = SIZE1 + SIZE2 + SIZE3 + POSITIONS3_L2,
				PASSES_PER_REGION = 1 << 14, //
				NUMBER_OF_REGIONS = 1 << POSITIONS2_L2
			};
			int regions_tested;
			int passes_till_next_region;
			int blocks_till_next_pass;
			int blocks_per_pass;
			int passes_at_once;
			FixedSizeCount<Uint16, 1 << TOTAL_INDEX_BITS> counts;
			// reordered order:                (pos2 aka region), (pos3), (window1), (window2), (window3)
			// index order, from high to low:  (pos2 aka region), (window1), (window2), (pos3), (window3)
		};
		class TripleMirrorFreq : public TestBaseclass {
		public:
			TripleMirrorFreq(int passes_at_once_, int blocks_per_pass_) : passes_at_once(passes_at_once_), blocks_per_pass(blocks_per_pass_) {}
			virtual void init(PractRand::RNGs::vRNG *known_good);
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);

			virtual void test_blocks(TestBlock *data, int numblocks);
			virtual int get_blocks_to_repeat() const;

		protected:
			enum {
				BASE_ALIGN_L2 = 2,
				POSITION_ALIGN_L2 = 2,
				BLOCK_STEP = 16,
				SIZE1 = 3,//units of bits
				SIZE2 = 3,
				SIZE3 = 3,
				POSITIONS_L2 = 6,//can't exceed (10-SAMPLE_ALIGN_L2) atm
				TOTAL_INDEX_BITS = SIZE1 + SIZE2 + SIZE3 + POSITIONS_L2,
			};
			int blocks_till_next_pass;
			int blocks_per_pass;
			int passes_at_once;
			FixedSizeCount<Uint16, 1 << TOTAL_INDEX_BITS> counts;
			// index order, from high to low:  (pos), (window1), (window2), (window3)
		};
		class TripleMirrorFreqN : public TestBaseclass {
		public:
			TripleMirrorFreqN(int minimum_level_) : minimum_level(minimum_level_), blocks_per_pass(1 << minimum_level_) {}
			virtual void init(PractRand::RNGs::vRNG *known_good);
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);

			virtual void test_blocks(TestBlock *data, int numblocks);
			//virtual int get_blocks_to_repeat() const;

		protected:
			enum {
				MAX_LEVELS = 16,
				ALIGN_L2 = 2,
				ALIGN = 1 << ALIGN_L2,
				SIZE1 = 3,//units of bits, can't exceed ALIGN
				SIZE2 = 3,
				SIZE3 = 3,
				POSITIONS_L2 = 4,//can't exceed (6-ALIGN_L2) atm
				TOTAL_INDEX_BITS = SIZE1 + SIZE2 + SIZE3 + POSITIONS_L2,
			};
			Uint64 saved_blocks[MAX_LEVELS * 2];
			char level_state[MAX_LEVELS];
			//states: 
			//0: no blocks saved
			//1: 1 block saved
			//2: 2 blocks saved, in order
			//3: 2 blocks saved, reverse order
			char level_polarity[MAX_LEVELS];// 0: 
			int blocks_till_next_pass;
			int blocks_per_pass;
			int minimum_level;
			FixedSizeCount<Uint16, MAX_LEVELS << TOTAL_INDEX_BITS> counts;
			// index order, from high to low:  (pos), (window1), (window2), (window3)
		};
		class TripleMirrorCoup : public TestBaseclass {
		public:
			TripleMirrorCoup(int passes_at_once_, int blocks_per_pass_) : passes_at_once(passes_at_once_), blocks_per_pass(blocks_per_pass_) {}
			virtual void init(PractRand::RNGs::vRNG *known_good);
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);

			virtual void test_blocks(TestBlock *data, int numblocks);
			virtual int get_blocks_to_repeat() const;

		protected:
			enum {
				BASE_ALIGN_L2 = 2,
				POSITION_ALIGN_L2 = 2,
				BLOCK_STEP = 16,
				SIZE1 = 3,//units of bits
				SIZE2 = 3,
				SIZE3 = 3,
				POSITIONS_L2 = 6,//can't exceed (10-SAMPLE_ALIGN_L2) atm
				TOTAL_INDEX_BITS = SIZE1 + SIZE2 + SIZE3 + POSITIONS_L2,
				COUP_BUCKETS = 256,
			};
			Uint64 pass_number;
			int blocks_till_next_pass;
			int blocks_per_pass;
			int passes_at_once;
			FixedSizeCount<Uint16, 1 << TOTAL_INDEX_BITS> counts;
			Uint64 coup_masks[1 << (TOTAL_INDEX_BITS >> 6)];
			Uint64 coup_counts[COUP_BUCKETS];
			Uint64 coup_last[1 << POSITIONS_L2];
			Uint64 coup_collected;
			// index order, from high to low:  (pos), (window1), (window2), (window3)
		};
		class QuadFreq : public TestBaseclass {//not yet implemented
		public:
			QuadFreq(int passes_at_once_, int blocks_per_pass_) : passes_at_once(passes_at_once_), blocks_per_pass(blocks_per_pass_) {}
			virtual void init(PractRand::RNGs::vRNG *known_good);
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);

			virtual void test_blocks(TestBlock *data, int numblocks);

		protected:
			enum {
				BASE_ALIGNMENT = 64,//don't change
				WINDOW_ALIGNMENT = 3,
				SIZE1 = 3,//sizes must be multiples of WINDOW_ALIGNMENT
				SIZE2 = 3,
				SIZE3 = 3,
				SIZE4 = 3,
				POSITIONS2_L2 = 5,
				POSITIONS3_L2 = 5,
				POSITIONS4_L2 = 5,
				TOTAL_INDEX_BITS = SIZE1 + SIZE2 + SIZE3 + SIZE4 + POSITIONS2_L2 + POSITIONS3_L2 + POSITIONS4_L2,
				REGION_INDEX_BITS = SIZE1 + SIZE2 + SIZE3 + POSITIONS4_L2,
				PASSES_PER_REGION = 1 << 14, //
				NUMBER_OF_REGIONS = 1 << POSITIONS2_L2
			};
			int regions_tested;
			int passes_till_next_region;
			int blocks_till_next_pass;
			int blocks_per_pass;
			int passes_at_once;
			FixedSizeCount<Uint16, 1 << TOTAL_INDEX_BITS> counts;
			// reordered order:                (pos2 aka region), (pos3), (window1), (window2), (window3)
			// index order, from high to low:  (pos2 aka region), (window1), (window2), (pos3), (window3)
		};
		class LPerm16 : public TestBaseclass {//not yet implemented
		public:
			LPerm16(int word_bits_, int passes_at_once_ = 0, int blocks_per_pass_ = 1) : word_bits(word_bits_), passes_at_once(passes_at_once_), blocks_per_pass(blocks_per_pass_) {}
			virtual void init(PractRand::RNGs::vRNG *known_good);
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);

			virtual void test_blocks(TestBlock *data, int numblocks);

		protected:
			enum {
				LPERM_BUCKETS = 1 << 15 // do not change
			};
			int word_bits;// 8, 16, 32, or 64 bits; 16 is recommended
			int blocks_till_next_pass;
			int blocks_per_pass;
			int passes_at_once;
			FixedSizeCount<Uint16, LPERM_BUCKETS> lperm_counts;//consider: also doing longer range tests on the comparisons done here
		};
	}//Tests
}//PractRand
