namespace PractRand {
	namespace Tests {
		class Coup16 : public TestBaseclass {
		protected:
			enum { S = 65536 / 32 };
			Uint32 flags[S];
			FixedSizeCount<Uint8, 65536> counts;
			//to do: also measure sequential correlation, or do something like BCFN on whether or not each sample is >= the expected value
		public:
			Coup16() {}
			virtual void init(PractRand::RNGs::vRNG *known_good);
			virtual std::string get_name() const;
			virtual void get_results(std::vector<TestResult> &results);
			virtual void test_blocks(TestBlock *data, int numblocks);
		};
	}//Tests
}//PractRand
