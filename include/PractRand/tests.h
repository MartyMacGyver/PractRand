#ifndef __PRACTRAND_TESTS_H__
#define __PRACTRAND_TESTS_H__


namespace PractRand {
	namespace RNGs { class vRNG; }
	class TestResult {
	public:
		std::string name;
		double weight;
		double raw;//-inf to +inf but concentrated around -10 to +10
		double processed;//varies depending upon type:
		int type;
		enum {TYPE_RAW, TYPE_RAW_NORMAL, TYPE_BAD_P, TYPE_BAD_S, TYPE_GOOD_P, TYPE_GOOD_S, TYPE_PASSFAIL};
		//  TYPE_RAW           there is no processed value
		//  TYPE_RAW_NORMAL    there is no processed value, but treat the raw value as roughly a badly distorted normal distribution
		//  TYPE_BAD_P         a p-value, but probably a VERY low quality p-value
		//  TYPE_BAD_S         same scale as TYPE_GOOD_S, but extremely crude & poor resolution
		//  TYPE_GOOD_P        a relatively trustworthy p-value
		//  TYPE_GOOD_S        a transformed p-value that has better dynamic range... 1 : 0.75, 0 : 0.5, -1 = 0.25, -2 = 0.125, -3 = 0.0625, -0.3 = 0.406126..., etc
		//  TYPE_PASSFAIL      0 means pass, 1 means fail

		TestResult(const std::string &name_, double raw_, double processed_, int type_, double weight_) : name(name_), raw(raw_), processed(processed_), type(type_), weight(weight_) {}
		double get_raw() const {return raw;}
		double get_pvalue() const;
		double get_suspicion() const;
		double get_weight() const {return weight;}
		void set_weight(double weight_) {weight = weight_;}
		static double pvalue_to_suspicion(double pvalue);
		static double suspicion_to_pvalue(double suspicion);
	};
	namespace Tests {
		union TestBlock {
			enum {
				SIZE_L2 = 10,
				SIZE = 1<<SIZE_L2
			};
			Uint8  as8 [SIZE  ];
			Uint16 as16[SIZE/2];
			Uint32 as32[SIZE/4];
			Uint64 as64[SIZE/8];
			void fill(RNGs::vRNG *rng, unsigned long numblocks = 1);
		};
		class ListOfTests;
		class TestBaseclass {
		protected:
			Uint64 blocks_tested;
		public:
			virtual void init(PractRand::RNGs::vRNG *known_good);
			virtual void deinit() {}
			virtual ~TestBaseclass()   {}
			virtual std::string get_name() const = 0;

			//1.  Maximum length per test_blocks() call is about 512MB (numblocks==1<<19)
			//2.  Maximum total length from multiple calls varies slightly but is usually 
			//       at least 256 TB (numblocks == 1<<48), and often more.  
			//3.  If the total length is too short then results may not be produced, or 
			//       might be less reliable.  
			//4.  You are supposed to make sure that the last REPEATED_BLOCKS blocks from 
			//       your previous test_blocks() calls are prepended on to the blocks 
			//       array.  Most tests don't care about this, but a few do.  The extra 
			//       blocks prepended are to appear at negative indices on the blocks[] 
			//       array, and are not counted in numblocks.  
			virtual void test_blocks(TestBlock *blocks, int numblocks) = 0;


			virtual void get_results ( std::vector<TestResult> &results ) = 0;

			virtual int get_blocks_to_repeat() const;//this is the number of blocks at negative indeces that test_blocks should be able to access
			//containing duplicates of data from the end of the previous set of blocks passed to test_blocks
			//obviously up to the limit of the number of blocks previously passed in
			//this may not be changed after the call to init()
		protected:
		};
	}//Tests
}//PractRand

#endif //__PRACTRAND_TESTS_H__