namespace PractRand {
	namespace Tests {
		class Birthday32 : public TestBaseclass {
			enum {
				BUFFER_SIZE_L2 = 12, // must be at least 8
				BUFFER_SIZE = 1 << BUFFER_SIZE_L2,
				MAX_DUPLICATES = 32
			};
			Uint32 buffer[1 << BUFFER_SIZE_L2];
			Uint64 counts[MAX_DUPLICATES];
			int num_buffered;
			void flush_buffer();
		public:
			Birthday32();
			virtual void init(PractRand::RNGs::vRNG *known_good);
			//virtual void deinit();
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);

			virtual void test_blocks(TestBlock *data, int numblocks);
		};
		class Birthday64 : public TestBaseclass {
			enum {
				BUFFER_SIZE_L2 = 23, // must be at least 7
				BUFFER_SIZE = 1 << BUFFER_SIZE_L2,
				MAX_DUPLICATES = 64,
				SORT_HELPER_BITS = 10
			};
			Uint64 buffer[1 << BUFFER_SIZE_L2];
			Uint64 counts[MAX_DUPLICATES];
			static void _histogram_in_place_sort64(Uint64 *base, long length, long bits_already, Uint32 freq_counts[1 << SORT_HELPER_BITS]);
			static void _histogram_in_place_sort64(Uint64 *base, long length);
			static void _histogram_sort64(Uint64 *base, long length, long bits_already, Uint32 freq_counts[1 << SORT_HELPER_BITS]);
			static void _histogram_sort64(Uint64 *base, long length);
			int num_buffered;
			void flush_buffer();
		public:
			Birthday64();
			virtual void init(PractRand::RNGs::vRNG *known_good);
			//virtual void deinit();
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);

			virtual void test_blocks(TestBlock *data, int numblocks);
		};
		namespace BirthdayHelpers {
			enum { SORT_HELPER_BITS = 8 };
			struct i128 {
				Uint64 low;
				Uint64 high;
				bool operator==(const i128 &other) const {
					return high == other.high && low == other.low;
				}
				bool operator<(const i128 &other) const {
					if (high < other.high) return true;
					if (high > other.high) return false;
					return low < other.low;
				}
				i128 operator-(const i128 &other) const {
					i128 rv;
					rv.high = high - other.high;
					rv.low = low - other.low;
					rv.high -= other.low > low ? 1 : 0;
					return rv;
				}
			};

			// this is the fastest in-place sort I've tried so far
			// in-place is important if I want to sort something huge without assuming I can allocate a comparable amount of memory to help with the sorting
			void histogram_in_place_sort128(i128 *base, Uint64 length, long bits_already, Uint64 freq_counts[1 << SORT_HELPER_BITS]);
			void histogram_in_place_sort128(i128 *base, Uint64 length, long bits_already = 0);

			// ..but sometimes I'm sorting smaller buffers, with pre-allocated regions to sort into..., so maybe another interface for that would help
			void histogram_sort_and_copy(i128 *base, i128 *dest, Uint64 length, long bits_already, Uint64 freq_counts[1 << SORT_HELPER_BITS]);
			void histogram_sort_and_copy(i128 *base, i128 *dest, Uint64 length, long bits_already = 0);
			//possibly faster algorithm for the same interface?
			void radix_sort_and_copy(i128 *base, i128 *dest, Uint64 length, long bits_already = 0);

			void _sorted_deltas_of_sorted_values(i128 *base, long length_L2, Uint64 freq_counts[1 << SORT_HELPER_BITS]);
			void _sorted_deltas_of_sorted_values(i128 *base, long length_L2);
		};
		class BirthdayLamda1 : public TestBaseclass {
		protected:
			//optimized for lambda=1, few runs, as described in "On the performance of birthday spacings tests with certain families of random number generators" (L'ecuyer & Simard, 2001)
			//using namespace BirthdayHelpers;// this is illegal in C++, I guess instead I'll try:
			typedef BirthdayHelpers::i128 i128;
			enum {
				SORT_HELPER_BITS = BirthdayHelpers::SORT_HELPER_BITS,
				DO_LARGEST_SPACING = 1,
			};
			bool autofail;
			Uint64 sort_helper_counts[1 << SORT_HELPER_BITS];
			//i128 buffer[1 << BUFFER_SIZE_L2];//can't have arrays this large inside a class due to object file or executable file format constraints
			//std::vector<i128> buffer;// ... and the STL vector implementation I'm using throws some kind of exception if it exceeds about 4 GB or so
			i128 *buffer;
			Uint64 num_buffered;
			virtual Uint64 flush_buffer();
			double duplicates;
			double expected_duplicates;
			double longest_spacing;
			int buffer_size_L2;
			int bits_to_use;
		public:
			BirthdayLamda1(int buffer_size_L2_ = 26);
			virtual ~BirthdayLamda1();
			virtual void init(PractRand::RNGs::vRNG *known_good);
			//virtual void deinit();
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);

			virtual void test_blocks(TestBlock *data, int numblocks);
		};
		class BirthdaySystematic : public BirthdayLamda1 {
			// similar to BirthdayLambda1 above
			// but if a result is requested before the first sample is ready, it will return a result for a partial buffer
			// and attempts to have everything optimized for the possibility of that partial-buffer case
			virtual Uint64 flush_buffer();
			static Uint64 get_target_num_at_bufsize(int bufsize_L2_);
			int already_sorted;//if this is half of (1ull << bufsize_L2) then incomplete_duplicates should hold 

			double score;//for scoring method 2
			static double evaluate_score(double lambda, Uint64 duplicates);

			void do_incomplete_buffer();
			double incomplete_duplicates;
			double incomplete_expected_duplicates;
		public:
			BirthdaySystematic(int max_bufsize_L2_ = 28);
			virtual void init(PractRand::RNGs::vRNG *known_good);
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);
			virtual void test_blocks(TestBlock *data, int numblocks);
		};
		class BirthdayAlt : public TestBaseclass {
			//as for BirthdayLambda1, but: 
			// keep all bits regardless of buffer size, just count an adjusting range of near deltas as if they were exact matches (or score them based upon how exact they are?)
			// try filtering the initial samples range, as if it was a small part of a larger sort buffer
			typedef BirthdayHelpers::i128 i128;
			enum {
				SORT_HELPER_BITS = BirthdayHelpers::SORT_HELPER_BITS,
				//FILTER_BITS = 0,
			};
			//i128 buffer[1 << BUFFER_SIZE_L2];
			//std::vector<i128> buffer;
			i128 *buffer;
			int num_buffered;
			int buffer_size_L2;
			int filter_bits;
			Uint64 sort_helper_counts[1 << SORT_HELPER_BITS];
			bool autofail;
			void flush_buffer();

			double score_sum_log;
			double score_sum_log2;
			double score_sum_log_sqr;
			Uint64 count;
			static void _lookup_constants(int buffer_size_L2, long double *offset, long double *deviation, long double *samples);
		public:
			BirthdayAlt(int buffer_size_L2_, int filter_bits_ = 0);
			~BirthdayAlt();
			virtual void init(PractRand::RNGs::vRNG *known_good);
			//virtual void deinit();
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);

			virtual void test_blocks(TestBlock *data, int numblocks);
		};//*/
	}//Tests
}//PractRand
