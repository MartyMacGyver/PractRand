class TestManager {
protected:
	std::vector<PractRand::Tests::TestBlock> buffer;
	PractRand::RNGs::vRNG *rng;
	PractRand::RNGs::vRNG *known_good;
	PractRand::Tests::ListOfTests *tests;
	int max_buffer_amount;
	int prefix_blocks;
	int main_blocks;
	int blocks_to_repeat;
	Uint64 blocks_so_far;
	bool freshly_created;
	int prep_blocks(Uint64 &blocks);
public:
	const PractRand::RNGs::vRNG *get_rng() const {return rng;}//RNG being tested
	Uint64 get_blocks_so_far() {return blocks_so_far;}//number of blocks tested

	TestManager(PractRand::Tests::ListOfTests *tests_, PractRand::RNGs::vRNG *known_good_=NULL, int max_buffer_amount_ = 1 << (25-10));
	//rng_ = RNG to test
	//tests_ = list of tests use on the RNG
	//known_good_ = sometimes the tests or test manager need good random numbers for some reason
	//max_buffer_amount_ = size in kilobytes of the maximum amount of random data to keep buffered up at on time

	virtual ~TestManager();//destructor (destroys the tests in the ListOfTests)

	virtual void reset(PractRand::RNGs::vRNG *rng_ = NULL);//resets contents for starting a new test run ; if rng is NULL then it will reuse the current RNG

	virtual void test(Uint64 blocks);//does testing... the number of blocks is ADDITIONAL blocks to test, not total blocks to test

	virtual void get_results( std::vector<PractRand::TestResult> &result_vec );//gets the results
};

TestManager::TestManager(PractRand::Tests::ListOfTests *tests_, PractRand::RNGs::vRNG *known_good_, int max_buffer_amount_) {
	rng = NULL;
	tests = tests_;
	known_good = known_good_;
	if (!known_good) known_good = new PractRand::RNGs::Polymorphic::hc256(PractRand::SEED_AUTO);
	blocks_so_far = 0;
	max_buffer_amount = max_buffer_amount_;
	prefix_blocks = 0;
	main_blocks = 0;
	for (unsigned int i = 0; i < tests->tests.size(); i++) tests->tests[i]->init(known_good);
	freshly_created = true;
}
TestManager::~TestManager() {
	for (unsigned int i = 0; i < tests->tests.size(); i++) tests->tests[i]->deinit();
	for (unsigned int i = 0; i < tests->tests.size(); i++) delete tests->tests[i];
}
void TestManager::reset(PractRand::RNGs::vRNG *rng_) {
	if (!freshly_created) for (unsigned int i = 0; i < tests->tests.size(); i++) tests->tests[i]->deinit();
	freshly_created = false;
	for (unsigned int i = 0; i < tests->tests.size(); i++) tests->tests[i]->init(known_good);
	blocks_to_repeat = 0;
	for (unsigned int i = 0; i < tests->tests.size(); i++) {
		int rb = tests->tests[i]->get_blocks_to_repeat();
		if (blocks_to_repeat < rb) blocks_to_repeat = rb;
	}
	buffer.resize(max_buffer_amount + blocks_to_repeat);
	if (rng_) rng = rng_;
	main_blocks = 0;
	prefix_blocks = 0;
	blocks_so_far = 0;
}
int TestManager::prep_blocks(Uint64 &blocks) {
	Uint64 _delta_blocks = blocks;
	if (_delta_blocks > max_buffer_amount) _delta_blocks = max_buffer_amount;
	int delta_blocks = int(_delta_blocks);
	blocks -= delta_blocks;
	size_t repeat_region_start, repeat_region_size;
	if (prefix_blocks + main_blocks >= blocks_to_repeat) {
		repeat_region_start = prefix_blocks + main_blocks - blocks_to_repeat;
		repeat_region_size = blocks_to_repeat;
	}
	else {
		repeat_region_start = 0;
		repeat_region_size = prefix_blocks + main_blocks;
	}
	if (repeat_region_start != 0)
		std::memmove(&buffer[0], &buffer[repeat_region_start], repeat_region_size * PractRand::Tests::TestBlock::SIZE);
	prefix_blocks = repeat_region_size;
	main_blocks = delta_blocks;
	buffer[prefix_blocks].fill(rng, main_blocks);
	blocks_so_far += delta_blocks;
	return delta_blocks;
}
void TestManager::test(Uint64 num_blocks) {
	while (num_blocks) {
		prep_blocks(num_blocks);
		for (unsigned int i = 0; i < tests->tests.size(); i++)
			tests->tests[i]->test_blocks(&buffer[prefix_blocks], main_blocks);
	}
}
void TestManager::get_results( std::vector<PractRand::TestResult> &result_vec ) {
	for (unsigned int i = 0; i < tests->tests.size(); i++) {
		tests->tests[i]->get_results(result_vec);
	}
}
