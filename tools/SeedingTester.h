class Seeder_MetaRNG : public PractRand::RNGs::vRNG64 {
public:
	PractRand::RNGs::Polymorphic::hc256 known_good;
	PractRand::RNGs::vRNG *base_rng;
	Uint64 current_seed;

	std::set<Uint64> unordered_history;
	std::deque<Uint64> history;
	int history_limit;

	Seeder_MetaRNG(PractRand::RNGs::vRNG *base_rng_) : known_good(PractRand::SEED_NONE), history_limit(1024) {
		base_rng = base_rng_;
		//current_seed = known_good.raw64();
		//record_seed(current_seed);
	}
	~Seeder_MetaRNG() { delete base_rng; }
	void autoseed() {
		known_good.autoseed();
		current_seed = known_good.raw64();
		record_seed(current_seed);
	}
	void seed(PractRand::Uint64 s) {
		known_good.seed(s);
		current_seed = known_good.raw64();
		record_seed(current_seed);
	}
	bool record_seed(Uint64 new_seed) {
		if (!unordered_history.insert(new_seed).second) return false;
		current_seed = new_seed;
		history.push_back(current_seed);
		if (history.size() > history_limit) {
			unordered_history.erase(history.front());
			history.pop_front();
		}
		return true;
	}
	void evolve_seed() {
		Uint64 bits_tried = 0;
		while (true) {
			Uint64 bit = 1ull << known_good.randi(64);
			Uint64 new_seed = current_seed ^ bit;
			bits_tried |= bit;
			if (record_seed(new_seed)) return;
			if (0 == ~bits_tried) {
				while (true) {
					if (record_seed(known_good.raw64())) return;
				}
			}
		}
	}
	Uint64 raw64() {
		base_rng->seed(current_seed);
		Uint64 rv = base_rng->raw64();
		evolve_seed();
		return rv;
	}
	std::string get_name() const {
		std::ostringstream tmp;
		tmp << "SeedingTester(" << base_rng->get_name() << ")";
		return tmp.str();
	}
	void walk_state(StateWalkingObject *) {}
	static PractRand::RNGs::vRNG *_factory(std::vector<std::string> &params) {
		if (params.size() != 1) {params.push_back("wrong number of parameters - should be SeedingTester(rng)"); return NULL; }
		PractRand::RNGs::vRNG *rng = RNG_Factories::create_rng(params[0]);
		if (!rng) return NULL;
		return new Seeder_MetaRNG(rng);
	}
	static void register_name() {
		RNG_Factories::RNG_factory_index["SeedingTester"] = _factory;
	}
};
class EntropyPool_MetaRNG : public PractRand::RNGs::vRNG64 {
public:
	typedef PractRand::Uint64 Transform;
	PractRand::RNGs::Polymorphic::hc256 known_good;
	PractRand::RNGs::vRNG *base_entropy_pool;
	int min_length, max_length;
	int history_length;
	std::vector<Uint8> current_seed;
	Transform last_transform;
	std::multiset<Uint64> unordered_history;//hashes only
	std::deque<std::pair<std::multiset<Uint64>::iterator, Transform> > history;//hashes first, then transform applied - newest at front

	//what it should be: (but the current version is good enough)
	//std::map<Uint64,Uint64> unordered_history;// hashes -> positions;
	//std::deque<std::pair<std::map<Uint64,Uint64>::iterator,Transform> > history;//hashes first, then transform applied - newest at front
	//Uint64 position;//starts at 0, incremented after every entropy string

	EntropyPool_MetaRNG(PractRand::RNGs::vRNG *base_entropy_pool_, int min_length_, int max_length_) : known_good(PractRand::SEED_NONE), base_entropy_pool(base_entropy_pool_), min_length(min_length_), max_length(max_length_), history_length(16384) {
		//int len = (min_length + max_length) / 2;
		current_seed.reserve(max_length);
		//current_seed.resize(len);
		//for (int i = 0; i < len; i++) current_seed[i] = known_good.raw8();
		//last_transform = ???;
	}
	~EntropyPool_MetaRNG() { delete base_entropy_pool; }
	void autoseed() {
		known_good.autoseed();
		int len = (min_length + max_length) / 2;
		current_seed.resize(len);
		for (int i = 0; i < len; i++) current_seed[i] = known_good.raw8();
	}
	void seed(PractRand::Uint64 s) {
		known_good.seed(s);
		int len = (min_length + max_length) / 2;
		current_seed.resize(len);
		for (int i = 0; i < len; i++) current_seed[i] = known_good.raw8();
	}
	Transform pick_random_transform(const std::vector<Uint8> &message) {
		while (true) {
			if (known_good.randf() < 0.96) {//toggle bit
				return known_good.randi(message.size() * 8) + (Uint64(0) << 56);
			}
			if (known_good.randf() < 0.50) {//insertion
				if (message.size() >= max_length) continue;
				//low 8 bits = value to insert ; next 28 bits = position to insert at ; top 8 bits = action type
				Uint64 position = known_good.randi(message.size() + 1);
				return known_good.raw8() + (position << 8) + (Uint64(1) << 56);
			}
			else {//deletion
				if (message.size() <= min_length) continue;
				Uint64 position = known_good.randi(message.size());
				return message[position] + (position << 8) + (Uint64(2) << 56);
			}
		}
	}
	void apply_transform(std::vector<Uint8> &message, Transform transform) {
		//toggle bit, add byte at end, add byte at begining, remove byte at end, remove byte at begining
		//adds or removals must include the data added or removed in addition to the action
		switch (transform >> 56) {
		case 0://toggle bit
			message[transform >> 3] ^= 1 << (transform & 7);
			break;
		case 1://insert byte
		{
				   int position = (transform >> 8) & ((1ull << 28) - 1);
				   int value = transform & 255;
				   int old_size = message.size();
				   if (position > old_size) { std::printf("internal error - invalid EntropyPool_MetaRNG transform (insert)\n"); std::exit(1); }
				   message.resize(old_size + 1);
				   if (position < old_size) std::memmove(&message[position + 1], &message[position], old_size - position);
				   message[position] = value;
		}
			break;
		case 2://delete byte
		{
				   int position = (transform >> 8) & ((1ull << 28) - 1);
				   int value = transform & 255;
				   int old_size = message.size();
				   if (message[position] != value || position >= old_size) { std::printf("internal error - invalid EntropyPool_MetaRNG transform (deletion)\n"); std::exit(1); }
				   if (position != old_size - 1) std::memmove(&message[position], &message[position + 1], old_size - 1 - position);
				   message.resize(old_size - 1);
		}
			break;
		default:
			std::printf("internal error - unrecognized EntropyPool_MetaRNG transform\n");
			std::exit(1);
		}
	}
	void apply_inverse_transform(std::vector<Uint8> &message, Transform transform) {
		switch (transform >> 56) {
		case 0://reverse a toggle bit by doing the same thign
			apply_transform(message, transform);
			break;
		case 1://reverse an insertion with a deletion
			apply_transform(message, transform + (Uint64(1) << 56));
			break;
		case 2://reverse a deletion with an insertion
			apply_transform(message, transform - (Uint64(1) << 56));
			break;
		}
	}
	PractRand::Uint64 hash_message(const std::vector<Uint8> &message) {
		base_entropy_pool->reset_entropy();
		base_entropy_pool->add_entropy_N(&message[0], message.size());
		//base_entropy_pool->add_entropy64(0);
		base_entropy_pool->flush_buffers();
		return base_entropy_pool->raw64();
	}
	void check_history_length() {
		while (history.size() > history_length) {
			unordered_history.erase(history.back().first);
			history.pop_back();
		}
	}
	int hamming_distance(const Uint8 *message1, const Uint8 *message2, int n) {
		Uint32 sum = 0;
		for (int i = 0; i < n; i++) sum += PractRand::Tests::count_bits8(message1[i] ^ message2[i]);
		return sum;
	}
	bool check_conflict(const std::vector<Uint8> &message) {
		std::vector<Uint8> rewound = current_seed;
		for (std::deque<std::pair<std::multiset<Uint64>::iterator, Transform> >::iterator it = history.begin(); it != history.end(); it++) {
			//if (message.size() == rewound.size() && !std::memcmp(&message[0], &rewound[0], message.size())) {
			if (message.size() == rewound.size() && !hamming_distance(&message[0], &rewound[0], message.size())) {
				return true;
			}
			apply_inverse_transform(rewound, it->second);
		}
		return false;
	}
	void evolve_seed() {
		while (true) {
			Transform t = pick_random_transform(current_seed);
			std::vector<Uint8> new_seed = current_seed;
			apply_transform(new_seed, t);
			Uint64 hash = hash_message(new_seed);
			std::pair<std::multiset<Uint64>::iterator, std::multiset<Uint64>::iterator> sitr = unordered_history.equal_range(hash);
			if (sitr.first == sitr.second || !check_conflict(new_seed)) {//no conflicts
				current_seed.swap(new_seed);
				std::multiset<Uint64>::iterator it;
				if (sitr.first == unordered_history.end()) it = unordered_history.insert(hash);
				else it = unordered_history.insert(--sitr.first, hash);
				history.push_front(std::pair<std::multiset<Uint64>::iterator, Transform>(it, t));
				check_history_length();
				return;
			}
		}
	}
	PractRand::Uint64 raw64() {
		PractRand::Uint64 rv = hash_message(current_seed);
		evolve_seed();
		return rv;
	}
	std::string get_name() const {
		std::ostringstream tmp;
		tmp << "EntropyPoolingTester(" << base_entropy_pool->get_name() << "," << min_length << "to" << max_length << ")";
		return tmp.str();
	}
	void walk_state(PractRand::StateWalkingObject *) {}
	static PractRand::RNGs::vRNG *_factory(std::vector<std::string> &params) {
		if (params.size() != 3) { params.push_back("wrong number of parameters - should be EntropyPoolingTester(rng,minlength,maxlength)"); return NULL; }
		int minlength = std::atoi(params[1].c_str());
		int maxlength = std::atoi(params[1].c_str());
		if (minlength < 1 || maxlength < 1 || minlength > maxlength || maxlength > 500) { params.push_back("EntropyPoolingTester parameters out of range - 0 < minlength <= maxlength < 500"); return NULL; }
		PractRand::RNGs::vRNG *rng = RNG_Factories::create_rng(params[0]);
		if (!rng) return NULL;
		return new EntropyPool_MetaRNG(rng, minlength, maxlength);
	}
	static void register_name() {
		RNG_Factories::RNG_factory_index["EntropyPoolingTester"] = _factory;
	}
};