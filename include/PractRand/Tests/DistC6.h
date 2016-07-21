namespace PractRand {
	namespace Tests {
		class DistC6 : public TestBaseclass {
		public:
			DistC6 ( int length_ = 9, int unitsL_ = 0, 
				int bits_clipped_0_ = 1, 
				int bits_clipped_1_ = 0, 
				int bits_clipped_2_ = 0 ); 
			virtual void init( PractRand::RNGs::vRNG *known_good );
			virtual std::string get_name() const;
			//virtual double get_result();
			//virtual double result_to_pvalue ( Uint64 blocks, double r );
			virtual void get_results ( std::vector<TestResult> &results );

			virtual void test_blocks(TestBlock *data, int numblocks);

		protected:
			enum { ENABLE_REORDER = 1, ENABLE_8_BIT_BYPASS = 1 };
			//configuration:
			int length;
			int unitsL;
			int bits_clipped_0;
			int bits_clipped_1;
			int bits_clipped_2;
			//precalcs:
			int bits_per_sample;
			int size;
			Uint32 mask_pre;
			Uint32 lookup_table[ENABLE_8_BIT_BYPASS ? 256 : 65];//reorder_bits(reorder_codes(transform_bitcount( X ))) or reorder_bits(reorder_codes(transform_bitcount(count_bits8( X ))))
			Uint8 reorder_codes_table[64];
			Uint8 unreorder_codes_table[64];
			//state:
			int warmup;
			Uint32 last_index;
			VariableSizeCount<Uint8> counts;
			//internal helpers:
			virtual int transform_bitcount ( int bit_count ) const ;
			static Uint32 _reorder_bits ( Uint32 rcode, int bits_per_sample, int length ) ;
			Uint32 reorder_bits( Uint32 rcode ) const {return _reorder_bits(rcode, bits_per_sample, length);}
			Uint32 unreorder_bits ( Uint32 rbcode ) const {return _reorder_bits(rbcode, length, bits_per_sample);}
			Uint32 reorder_codes   ( Uint32 code ) const {return reorder_codes_table[code];}
			Uint32 unreorder_codes ( Uint32 rcode ) const {return unreorder_codes_table[rcode];}
			void generate_reorder_codes ();
			Uint32 _advance_index ( Uint32 index, int rbcode ) const {
				if (ENABLE_REORDER)
					return ((index & mask_pre) << 1) | rbcode ;
				else
					return ((index & mask_pre) << bits_per_sample) | rbcode ;
			}
			void advance_index ( int bit_count ) {
				last_index = _advance_index(last_index, lookup_table[bit_count]);}
		};
	}//Tests
}//PractRand
