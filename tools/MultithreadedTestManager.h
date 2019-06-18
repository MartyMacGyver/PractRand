class MultithreadedTestManager : public TestManager {
	class TestThread {
		PractRand::Tests::TestBlock *base_block;
		PractRand::Tests::TestBaseclass *test;
		Uint64 numblocks;
		Threading::Lock lock;
		bool finished;
	public:
		volatile bool retire() {
			lock.enter();
			if (!finished) {
				//must have tried to retire it too quickly to initialize properly?
				lock.leave();
				Threading::sleep(0);
				return false;
			}
			lock.leave();
			delete this;
			return true;
		}
		TestThread(PractRand::Tests::TestBaseclass *test_, PractRand::Tests::TestBlock * base_block_, Uint64 numblocks_)
		:
			base_block(base_block_),
			test(test_),
			numblocks(numblocks_),
			finished(false)
		{
			//_thread_func(this);
			Threading::create_thread(_thread_func, this);
		}
		static THREADFUNC_RETURN_TYPE THREADFUNC_CALLING_CONVENTION _thread_func(void *param_) {
			TestThread *param = (TestThread*)param_;
			param->lock.enter();
			enum {MAX_BLOCKS_PER_CALL = 1ull << 18};
			while (param->numblocks > MAX_BLOCKS_PER_CALL) {
				param->test->test_blocks(param->base_block,MAX_BLOCKS_PER_CALL);
				param->numblocks -= MAX_BLOCKS_PER_CALL;
				param->base_block += MAX_BLOCKS_PER_CALL;
			}
			if (param->numblocks) param->test->test_blocks(param->base_block,(int)param->numblocks);
			param->finished = true;
			param->lock.leave();
			return 0;
		}
	};
	std::vector<TestThread *> threads;
	void wait_on_threads() {
		while(!threads.empty()) {
			TestThread *last = threads.back();
			if (last->retire()) threads.pop_back();
		}
	}


public:
	//std::vector<PractRand::Tests::TestBlock> buffer1, buffer2;
	std::vector<PractRand::Tests::TestBlock> alt_buffer;
	//RNGs::vRNG *rng;
	//RNGs::vRNG *known_good;
	//Tests::ListOfTests *tests;
	//int max_buffer_amount;
	//int prefix_blocks;
	//int main_blocks;
	//Uint64 blocks_so_far;
	void multithreaded_prep_blocks(Uint64 num_blocks) {
		int new_prefix_blocks = blocks_to_repeat;
		if (new_prefix_blocks > prefix_blocks + main_blocks)
			new_prefix_blocks = prefix_blocks + main_blocks;
		if (new_prefix_blocks) {
			std::memcpy(
				&buffer[0], 
				&alt_buffer[prefix_blocks + main_blocks - new_prefix_blocks],
				PractRand::Tests::TestBlock::SIZE * new_prefix_blocks
			);
		}
		prefix_blocks = new_prefix_blocks;
		main_blocks = (num_blocks > max_buffer_amount) ? max_buffer_amount : Uint32(num_blocks);
		buffer[prefix_blocks].fill(rng, main_blocks);
		blocks_so_far += main_blocks;
	}
public:
	MultithreadedTestManager(PractRand::Tests::ListOfTests *tests_, PractRand::RNGs::vRNG *known_good_, int max_buffer_amount_ = 1 << (27-10)) : TestManager(tests_, known_good_, max_buffer_amount_) {
		//buffer1.resize(max_buffer_amount + Tests::TestBaseclass::REPEATED_BLOCKS);
		for (unsigned int i = 0; i < tests->tests.size(); i++) tests->tests[i]->init(known_good);
	}
	void test(Uint64 num_blocks) {
		while (num_blocks) {
			multithreaded_prep_blocks(num_blocks);
			num_blocks -= main_blocks;
			wait_on_threads();
			alt_buffer.swap(buffer);
			for (unsigned int i = 0; i < tests->tests.size(); i++) {
				threads.push_back( new TestThread( tests->tests[i], &alt_buffer[prefix_blocks], main_blocks ) );
			}
		}
		wait_on_threads();
	}
	virtual void reset(PractRand::RNGs::vRNG *rng_ = NULL) {//resets contents for starting a new test run ; if rng is NULL then it will reuse the current RNG
		if (!freshly_created) for (unsigned int i = 0; i < tests->tests.size(); i++) tests->tests[i]->deinit();
		freshly_created = false;
		for (unsigned int i = 0; i < tests->tests.size(); i++) tests->tests[i]->init(known_good);
		blocks_to_repeat = 0;
		for (unsigned int i = 0; i < tests->tests.size(); i++) {
			int rb = tests->tests[i]->get_blocks_to_repeat();
			if (blocks_to_repeat < rb) blocks_to_repeat = rb;
		}
		buffer.resize(max_buffer_amount + blocks_to_repeat);
		alt_buffer.resize(max_buffer_amount + blocks_to_repeat);
		if (rng_) rng = rng_;
		if (!rng) issue_error();
		main_blocks = 0;
		prefix_blocks = 0;
		blocks_so_far = 0;
	}
};
