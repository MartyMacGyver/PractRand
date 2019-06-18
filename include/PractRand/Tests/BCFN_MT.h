namespace PractRand {
	namespace Tests {
		class BCFTFN : public TestBaseclass {
		protected:
			enum {LEVELS = 8};
			long overlap, tbits, mask;
			VariableSizeCount<Uint16> counts[LEVELS];
			long current[LEVELS];
			long warmup[LEVELS];
		public:
			BCFTFN(int funcmask = 1);

		};
		class BCFN_MT : public TestBaseclass {
		protected:
			enum { LEVELS = 32, THRESHOLDS_L2 = 2, THRESHOLDS=1<<THRESHOLDS_L2, INDEX_SIZE_L2 = 8, INDEX_SIZE = 1 << INDEX_SIZE_L2, TOTAL = THRESHOLDS * LEVELS };

			VariableSizeCount<Uint16> counts[TOTAL];
			long cur[TOTAL];
			long warmup[TOTAL];

			Uint8 threshold_lookup[LEVELS * INDEX_SIZE];
			long bitcount_shift[LEVELS];

			long mask;

			bool odd[LEVELS];
			long leftovers[LEVELS];
			//long total[TOTAL];
			long tbits;
			long unitsL2;
			//Uint64 blocks;
		public:
			BCFN_MT(int unitsL2_ = 0, int tbits_ = 7);
			virtual void init( PractRand::RNGs::vRNG *known_good );
			virtual void deinit( );
			virtual std::string get_name() const;
			//virtual double get_result();
			//virtual double result_to_pvalue ( Uint64 blocks, double r );
			virtual void get_results ( std::vector<TestResult> &results );

			virtual void test_blocks(TestBlock *data, int numblocks);
			//void handle_high_levels ( int level, int bits );
			static int sign(int value) {return ((Uint32(value)) >> 31) & 1;}
			void handle_high_levels(int level, int bits ) {
				if (bits) {
					int adjusted_bits = abs(bits) >> bitcount_shift[level];
					if (adjusted_bits >= INDEX_SIZE) adjusted_bits = INDEX_SIZE - 1;
					int max_threshold = threshold_lookup[adjusted_bits + level * THRESHOLDS];
					int s = sign(bits);
					for (int threshold = 0; threshold < max_threshold; threshold++) {
						int lti = level * THRESHOLDS + threshold;
						cur[lti] = ((cur[lti] << 1) & mask) | s;
						if (!warmup[lti]) counts[lti].increment(cur[lti]);
						else --warmup[lti];
					}
				}

				if (odd[level] && level < LEVELS) {
					odd[level] = false;
					handle_high_levels(level+1, bits + leftovers[level]);
				}
				else {
					leftovers[level] = bits;
					odd[level] = true;
				}
			}
		};
	}//Tests
}//PractRand
