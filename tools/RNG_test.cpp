
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <cstring>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <list>
#include <sstream>

//master header, includes everything in PractRand for both 
//  practical usage and research... 
//  EXCEPT it does not include specific algorithms
#include "PractRand_full.h"

#define MULTITHREADING_SUPPORTED
#define CPP_2011_SUPPORTED

//specific algorithms: all recommended RNGs
#include "PractRand/RNGs/all.h"

//specific algorithms: non-recommended RNGs
#include "PractRand/RNGs/other/transform.h"
#include "PractRand/RNGs/other/mult.h"
#include "PractRand/RNGs/other/simple.h"
#include "PractRand/RNGs/other/fibonacci.h"
#include "PractRand/RNGs/other/indirection.h"
#include "PractRand/RNGs/other/special.h"

#ifdef WIN32 // needed to allow binary stdin on windows
#include <fcntl.h>
#include <io.h>
#endif

using namespace PractRand;

PractRand::RNGs::Polymorphic::hc256 known_good(PractRand::SEED_AUTO);

//helpers for the test programs, to deal with RNG names, test usage, etc
#include "RNG_from_name.h"

#include "TestManager.h"
#ifdef MULTITHREADING_SUPPORTED
#include "multithreading.h"
#include "MultithreadedTestManager.h"
#endif
#include "Candidate_RNGs.h"

#if defined CPP_2011_SUPPORTED
#include <chrono>
typedef std::chrono::system_clock::rep TimeUnit;
TimeUnit get_time() { return std::chrono::system_clock::now().time_since_epoch().count(); }
double get_time_period() { return std::chrono::system_clock::period().num / (double)std::chrono::system_clock::period().den; }
#else
typedef std::clock_t TimeUnit;
std::clock_t get_time() {return std::clock();}
double get_time_period() { return 1.0 / CLOCKS_PER_SEC; }
#endif

/*
A minimal RNG implementation, just enough to make it usable.  
Deliberately flawed, though still better than most platforms default RNGs
*/

//#include "PractRand/endian.h"

class DummyRNG : public PractRand::RNGs::vRNG32 {
public:
	//declare state
	Uint32 x, y, z, index;
	//implement algorithm
	Uint32 raw32() {
		x += index++;
		y += x;
		z += y ^ index;
		return z;
	}
	//allow PractRand to be aware of your internal state
	void walk_state(PractRand::StateWalkingObject *walker) {
		walker->handle(x);
		walker->handle(y);
		walker->handle(z);
		walker->handle(index);
	}
	//any name you want
	std::string get_name() const {return "DummyRNG";}
};
/*
	The above class is enough to create a PRNG compatible with PractRand.  
	You can pass that to any non-template function in PractRand expecting a polymorphic RNG and it should work fine.  
	(some template functions in PractRand require additional metadata or other weirdness)
	HOWEVER, that is not enough to allow this (or any other) command line tool to recognize the name of your RNG from the command line.  
	For that, search for the line mentioning RNG_factory_index["dummy"] in the "main" function below here, 
	that line allows it to recognize "dummy" on the command line as corresponding to this class.  
*/

double print_result(const PractRand::TestResult &result, bool print_header = false) {
	if (print_header) std::printf("  Test Name                         Raw       Processed     Evaluation\n");
	//                                     10        20        30        40        50        60        70        80
	std::printf("  ");
	//NAME
	if (true) {// 34 characters
		std::printf("%s", result.name.c_str());
		int len = result.name.length();
		for (int i = len; i < 34; i++) std::printf(" ");
	}

	//RAW TEST RESULT
	if (true) {// 10 characters?
		double raw = result.get_raw();
		if (raw < 999.95) std::printf("R=%+6.1f ", raw);
		else std::printf("R=%+6.0f ", raw);
		if (fabs(raw) < 99999.5) std::printf(" ");
		//if (fabs(raw) < 999999.5) std::printf(" ");
		//if (fabs(raw) < 9999999.5) std::printf(" ");
	}

	//RESULT AS A NUMERICAL "SUSPICION LEVEL" (log of distance from pvalue to closest extrema)
	if (false) {// 12 characters?
		bool printed = false;
		double susp = result.get_suspicion();
		if (result.type == result.TYPE_PASSFAIL)
			std::printf("  %s    ", result.get_pvalue() ? "\"pass\"" : "\"fail\"");
		else if (result.type == result.TYPE_RAW)
			std::printf("            ");
		else if (result.type == result.TYPE_BAD_P || result.type == result.TYPE_BAD_S || result.type == result.TYPE_RAW_NORMAL) {
			std::printf("S=~%+6.1f   ", susp);
			printed = true;
		}
		else {
			std::printf("S =%+6.1f   ", susp);
			printed = true;
		}
		if (printed) {
			if (fabs(susp) < 9999.95) std::printf(" ");
			if (fabs(susp) < 999.95) std::printf(" ");
		}
	}

	//RESULT AS A p-value
	if (true) {// 14 characters?
		if (result.type == result.TYPE_PASSFAIL)
			std::printf("  %s      ", result.get_pvalue() ? "\"pass\"" : "\"fail\"");
		else if (result.type == result.TYPE_RAW)
			std::printf("              ");
		else if (result.type == result.TYPE_BAD_P || result.type == result.TYPE_GOOD_P || result.type == result.TYPE_RAW_NORMAL) {
			double p = result.get_pvalue();
			double a = std::fabs(p-0.5);
			std::printf((result.type != result.TYPE_GOOD_P) ? "p~= " : "p = ");
			if (a > 0.49) {
				double s = result.get_suspicion();
				double ns = std::fabs(s) + 1;
				double dec = ns / (std::log(10.0) / std::log(2.0));
				double dig = std::ceil(dec);
				double sig = std::floor(std::pow(0.1, dec - dig));
				if (dig > 999) std::printf(" %d        ", (s > 0) ? 1 : 0);
				else {
					if (s > 0) std::printf("1-%1.0fe-%.0f  ", sig, dig);
					else       std::printf("  %1.0fe-%.0f  ", sig, dig);
					if (dig < 100) std::printf(" ");
					if (dig < 10) std::printf(" ");
				}
			}
			else if (a > 0.4) std::printf("%4.2f      ", p);
			else              std::printf("%3.1f       ", p);
		}
		else if (result.type == result.TYPE_BAD_S || result.type == result.TYPE_GOOD_S) {
			double s = result.get_suspicion();
			double p = result.get_pvalue();
			std::printf((result.type == result.TYPE_BAD_S || result.type == result.TYPE_RAW_NORMAL) ? "p~=" : "p =");
			if (p >= 0.01 && p <= 0.99) std::printf(" %.3f     ", p);
			else {
				double ns = std::fabs(s) + 1;
				double dec = ns / (std::log(10.0) / std::log(2.0));
				double dig = std::ceil(dec);
				double sig = std::pow(0.1, dec - dig);
				sig = std::floor(sig * 10) * 0.1;
				if (dig > 9999) std::printf(" %d         ", (s > 0) ? 1 : 0);
				else if (dig > 999) {
					sig = std::floor(sig);
					if (s > 0) std::printf("1-%1.0fe-%.0f  ", sig, dig);
					else       std::printf("  %1.0fe-%.0f  ", sig, dig);
				}
				else {
					if (s > 0) std::printf("1-%3.1fe-%.0f ", sig, dig);
					else       std::printf("  %3.1fe-%.0f ", sig, dig);
					if (dig < 100) std::printf(" ");
					if (dig < 10) std::printf(" ");
				}
			}
		}
	}

	double dec = std::log(2.) / std::log(10.0);
	double as = (std::fabs(result.get_suspicion()) + 1) * dec;
	double wmod = std::log(result.get_weight()) / std::log(0.5) * dec;
	double rs = as - wmod;
	//double ap = std::fabs(0.5 - result.get_pvalue());
	//MESSAGE DESCRIBING RESULT IN ENGLISH
	if (true) {// 17 characters?
		/*
			Threshold Values:
			The idea is to assign a suspicioun level based not just upon the 
			p-value but also the number of p-values and their relative importance.  
			If there are a million p-values then we probably don't care about 
			anything less extreme than a one in ten million event.  
			But if there's one important p-value and a million unimportant ones then 
			the important one doesn't have to be that extreme to rouse our suspicion.  

			Output Format:
			unambiguous failures are indented 2 spaces to make them easier to spot
			probable failures with (barely) enough room for ambiguity are indented 1 space
			the most extreme failures get a sequence of exclamation marks to distinguish them
		*/
		if (false) ;
		else if (rs >999) std::printf("  FAIL !!!!!!!!  ");
		else if (rs >325) std::printf("  FAIL !!!!!!!   ");
		else if (rs >165) std::printf("  FAIL !!!!!!    ");
		else if (rs > 85) std::printf("  FAIL !!!!!     ");
		else if (rs > 45) std::printf("  FAIL !!!!      ");
		else if (rs > 25) std::printf("  FAIL !!!       ");
		else if (rs > 17) std::printf("  FAIL !!        ");
		else if (rs > 12) std::printf("  FAIL !         ");
		else if (rs >8.5) std::printf("  FAIL           ");
		else if (rs >6.0) std::printf(" VERY SUSPICIOUS ");
		else if (rs >4.0) std::printf("very suspicious  ");
		else if (rs >2.5) std::printf("suspicious       ");
		else if (rs >1.0) std::printf("unusual          ");
		else              std::printf("normal           ");
	}
	std::printf("\n");
	return rs;
}

class Seeder_MetaRNG : public PractRand::RNGs::vRNG64 {
public:
	PractRand::RNGs::vRNG *base_rng;
	Uint64 current_seed;

	std::set<Uint64> unordered_history;
	std::deque<Uint64> history;
	int history_limit;

	Seeder_MetaRNG(PractRand::RNGs::vRNG *base_rng_, int history_limit_=1024) : history_limit(history_limit_) {
		base_rng = base_rng_;
		current_seed = known_good.raw64();
		record_seed(current_seed);
	}
	~Seeder_MetaRNG() {delete base_rng;}
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
};
class EntropyPool_MetaRNG : public PractRand::RNGs::vRNG64 {
public:
	typedef Uint64 Transform;
	PractRand::RNGs::vRNG *base_entropy_pool;
	int min_length, max_length;
	int history_length;
	std::vector<Uint8> current_seed;
	Transform last_transform;
	std::multiset<Uint64> unordered_history;//hashes only
	std::deque<std::pair<std::multiset<Uint64>::iterator,Transform> > history;//hashes first, then transform applied - newest at front

	//what it should be: (but the current version is good enough)
	//std::map<Uint64,Uint64> unordered_history;// hashes -> positions;
	//std::deque<std::pair<std::map<Uint64,Uint64>::iterator,Transform> > history;//hashes first, then transform applied - newest at front
	//Uint64 position;//starts at 0, incremented after every entropy string

	EntropyPool_MetaRNG(PractRand::RNGs::vRNG *base_entropy_pool_, int min_length_, int max_length_, int history_length_ = 1024) : base_entropy_pool(base_entropy_pool_), min_length(min_length_), max_length(max_length_), history_length(history_length_) {
		int len = (min_length + max_length) / 2;
		current_seed.reserve(max_length);
		current_seed.resize(len);
		for (int i = 0; i < len; i++) current_seed[i] = known_good.raw8();
		//last_transform = ???;
	}
	~EntropyPool_MetaRNG() {delete base_entropy_pool;}
	Transform pick_random_transform(const std::vector<Uint8> &message) {
		while (true) {
			if (known_good.randf() < 0.96) {//toggle bit
				return known_good.randi(message.size() * 8) + (Uint64(0) << 56);
			}
			if (known_good.randf() < 0.50) {//insertion
				if (message.size() >= max_length) continue;
				//low 8 bits = value to insert ; next 28 bits = position to insert at ; top 8 bits = action type
				Uint64 position = known_good.randi(message.size()+1);
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
			if (position > old_size) {std::printf("internal error - invalid EntropyPool_MetaRNG transform (insert)\n");std::exit(1);}
			message.resize(old_size + 1);
			if (position < old_size) std::memmove(&message[position+1], &message[position], old_size-position);
			message[position] = value;
			}
			break;
		case 2://delete byte
			{
			int position = (transform >> 8) & ((1ull << 28) - 1);
			int value = transform & 255;
			int old_size = message.size();
			if (message[position] != value || position >= old_size) {std::printf("internal error - invalid EntropyPool_MetaRNG transform (deletion)\n");std::exit(1);}
			if (position != old_size-1) std::memmove(&message[position], &message[position+1], old_size-1-position);
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
			apply_transform(message, transform + (Uint64(1)<<56));
			break;
		case 2://reverse a deletion with an insertion
			apply_transform(message, transform - (Uint64(1)<<56));
			break;
		}
	}
	Uint64 hash_message(const std::vector<Uint8> &message) {
		base_entropy_pool->reset_entropy();
		base_entropy_pool->add_entropy_N(&message[0], current_seed.size());
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
		for (std::deque<std::pair<std::multiset<Uint64>::iterator,Transform> >::iterator it = history.begin(); it != history.end(); it++) {
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
				history.push_front(std::pair<std::multiset<Uint64>::iterator,Transform>(it, t));
				check_history_length();
				return;
			}
		}
	}
	Uint64 raw64() {
		Uint64 rv = hash_message(current_seed);
		evolve_seed();
		return rv;
	}
	std::string get_name() const {
		std::ostringstream tmp;
		tmp << "EntropyPoolingTester(" << base_entropy_pool->get_name() << "," << min_length << "to" << max_length << ")";
		return tmp.str();
	}
	void walk_state(StateWalkingObject *) {}
};


double show_checkpoint(TestManager *tman, int mode, Uint64 seed, double time, bool smart_thresholds, double threshold, bool end_on_failure) {
	std::printf("rng=%s", tman->get_rng()->get_name().c_str());

	std::printf(", seed=");
	if (seed >> 32) std::printf("0x%lx%08lx", long(seed >> 32), long(seed & 0xFFffFFff));
	else std::printf("0x%lx", long(seed));
	std::printf("\n");

	std::printf("length= ");
	Uint64 length = tman->get_blocks_so_far() * Tests::TestBlock::SIZE;
	double log2b = std::log(double(length)) / std::log(2.0);
	const char *unitstr[6] = {"kilobyte", "megabyte", "gigabyte", "terabyte", "petabyte", "exabyte"};
	int units = int(std::floor(log2b / 10)) - 1;
	if (units < 0 || units > 5) {std::printf("internal error: length out of bounds?\n");std::exit(1);}
	if (length & (length-1))
		std::printf("%.3f %ss", length * std::pow(0.5,units*10.0+10), unitstr[units] );
	else std::printf("%.0f %s%s", length * std::pow(0.5,units*10.0+10), unitstr[units], length != (Uint64(1024)<<(units*10)) ? "s" : "" );
	if (length & (length-1)) std::printf(" (2^%.3f", log2b - (mode?3:0)); else std::printf(" (2^%.0f", log2b - (mode?3:0));
	const char *mode_unit_names[3] = {"bytes", "seeds", "entropy strings"};
	std::printf(" %s), time= ", mode_unit_names[mode]);
	if (time < 99.95) std::printf("%.1f seconds\n", time);
	else std::printf("%.0f seconds\n", time);

	std::vector<PractRand::TestResult> results;
	tman->get_results(results);
	double total_weight = 0;
	for (int i = 0; i < results.size(); i++) total_weight += results[i].get_weight();
	std::vector<int> marked;
	for (int i = 0; i < results.size(); i++) {
		results[i].set_weight(results[i].get_weight() / total_weight);
		if (!smart_thresholds) {
			if (std::fabs(0.5 - results[i].get_pvalue()) < 0.5 - threshold) continue;
		}
		else {
			double T = threshold * results[i].get_weight();
			if (std::fabs(0.5 - results[i].get_pvalue()) < 0.5 * (1 - T)) continue;
		}
		marked.push_back(i);
	}
	double biggest_decimal_suspicion = 0;
	for (int i = 0; i < marked.size(); i++) {
		double decimal_suspicion = print_result(results[marked[i]], i == 0);
		if (decimal_suspicion > biggest_decimal_suspicion) biggest_decimal_suspicion = decimal_suspicion;
	}
	if (marked.size() == results.size())
		;
	else if (marked.size() == 0)
		std::printf("  no anomalies in %d test result(s)\n", int(results.size()));
	else
		std::printf("  ...and %d test result(s) without anomalies\n", int(results.size() - marked.size()));
	std::printf("\n");
	if (end_on_failure && biggest_decimal_suspicion > 8.5) std::exit(0);
}
double interpret_length(const std::string &lengthstr, bool normal_mode) {
	//(0-9)*[.(0-9)*][((K|M|G|T|P)[B])|(s|m|h|d)]
	int mode_factor = normal_mode ? 1 : 8;
	int pos = 0;
	double value = 0;
	for (; pos < lengthstr.size(); pos++) {
		char c = lengthstr[pos];
		if (c < '0') break;
		if (c > '9') break;
		value = value * 10 + (c - '0');
	}
	if (!pos) return 0;
	if (pos == lengthstr.size()) return std::pow(2.0,value) * mode_factor;
	if (lengthstr[pos] == '.') {
		pos++;
		double sig = 0.1;
		for (; pos < lengthstr.size(); pos++,sig*=0.1) {
			char c = lengthstr[pos];
			if (c < '0') break;
			if (c > '9') break;
			value += (c - '0') * sig;
		}
		if (pos == lengthstr.size()) return std::pow(2.0,value) * mode_factor;
	}
	double scale = 0;
	bool expect_B = true;
	char c = lengthstr[pos];
	switch (c) {
	case 'K':
		scale = 1024.0;
		break;
	case 'M':
		scale = 1024.0 * 1024.0;
		break;
	case 'G':
		scale = 1024.0 * 1024.0 * 1024.0;
		break;
	case 'T':
		scale = 1024.0 * 1024.0 * 1024.0 * 1024.0;
		break;
	case 'P':
		scale = 1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0;
		break;
	case 's':
		scale = -1;
		expect_B = false;
		break;
	case 'm':
		scale = -60;
		expect_B = false;
		break;
	case 'h':
		scale = -3600;
		expect_B = false;
		break;
	case 'd':
		scale = -86400;
		expect_B = false;
		break;
	}
	pos++;
	if (pos == lengthstr.size()) {
		if (scale < 0) {
			if (value < 0.05) value = 0.05;
			return value * scale;
		}
		return value * scale * mode_factor;
	}
	if (!expect_B) return 0;
	if (lengthstr[pos++] != 'B') return 0;
	if (pos != lengthstr.size()) return 0;
	return value * scale;
}
bool interpret_seed(const std::string &seedstr, Uint64 &seed) {
	//would prefer strtol, but that is insufficiently portable when it has to handle 64 bit values
	Uint64 value = 0;
	int position = 0;
	if (seedstr.length() >= 3 && seedstr[0] == '0' && seedstr[1] == 'x') position = 2;
	while (position < seedstr.length()) {
		int c = seedstr[position++];
		if (value >> 60) return false;//too long
		value *= 16;
		if (c >= '0' && c <= '9') value += (c-'0');
		else if (c >= 'a' && c <= 'f') value += (c-'a')+10;
		else if (c >= 'A' && c <= 'F') value += (c-'A')+10;
		else return false;//invalid character
	}
	seed = value;
	return true;
}

int main(int argc, char **argv) {
	PractRand::initialize_PractRand();
#ifdef WIN32 // needed to allow binary stdin on windows
	_setmode( _fileno(stdin), _O_BINARY);
#endif
	if (argc <= 1) {
		std::printf("usage: %s RNG_name [options]  --  runs tests on RNG_name\n", argv[0]);
		std::printf("or: %s -help  --  displays more instructions\n", argv[0]);
		std::printf("or: %s -version  --  displays version information\n", argv[0]);
		std::printf("RNG_name can be the name of any PractRand recommended RNG (example: sfc16) or\n");
		std::printf("non-recommended RNG (example: mm32) or transformed RNG (exmple: SShrink(sfc16).\n");
		//           12345678901234567890123456789012345678901234567890123456789012345678901234567890
		std::printf("Alternatively, use stdin as an RNG name to read raw binay data piped in from an\n");
		std::printf("external RNG.\n");
		std::printf("options available include -a, -e, -p, -tf, -te, -ttnormal, -ttseed64, -ttep,\n");
		std::printf("-tlmin, -tlmax, -tlshow, -threads, -nothreads, and -seed.\n");
		std::printf("For more information run: %s -help\n\n", argv[0]);
		std::exit(0);
	}
	if (!strcmp(argv[1], "-version") || !strcmp(argv[1], "--version") || !strcmp(argv[1], "-v")) {
		std::printf("RNG_test version %s\n", PractRand::version_str);
		// arbitrarily declaring the version number of RNG_test to match the version number of PractRand
		std::printf("A command line tool for testing RNGs with the PractRand library.\n");
		std::exit(0);
	}
	if (!strcmp(argv[1], "-help") || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) {
		std::printf("syntax: %s RNG_name [options]\n", argv[0]);
		std::printf("or: %s -help (to see this message)\n", argv[0]);
		std::printf("or: %s -version (to see version number)\n", argv[0]);
		std::printf("A command line tool for testing RNGs with the PractRand library.\n");
		std::printf("RNG names:\n");
		std::printf("  To use an external RNG, use stdin as an RNG name and pipe in the random\n");
		std::printf("  numbers.  stdin8, stdin16, stdin32, and stdin64 also work, each interpretting\n");
		std::printf("  the input in slightly different ways.  Use stdin if you're uncertain how many\n");
		//           12345678901234567890123456789012345678901234567890123456789012345678901234567890
		std::printf("  bits the RNG produces at a time, or if it's not one of those options.\n");
		std::printf("  The lowest quality recommended RNGs are sfc16 and mt19937.\n");
		std::printf("  The entropy pooling RNGs available are arbee and sha2_basd_pool.\n");
		std::printf("  Small recommended RNGs include sfc16, sfc32, sfc64, jsf32, jsf64, .\n");
		std::printf("threshold options:\n");
		std::printf(" At most one threshold option should be specified.\n");
		//std::printf(" The default threshold setting is '-e 0.1', an alternative is '-p 0.001'\n");
		std::printf(" The default threshold setting is '-e 0.1', alternatives are '-p 0.001' or '-a'\n");
		std::printf("  -a             no threshold - display all test results.\n");
		std::printf("  -e EXPECTED    sets intelligent p-value thesholds to display an expected\n");
		std::printf("                 number of test results equal to EXPECTED.  If EXPECTED is zero\n");
		std::printf("                 or less then intelligent p-value thresholds will be disabled\n");
		std::printf("                 EXPECTED is a float with default value 0.1\n");
		std::printf("  -p THRESHOLD   sets simple p-value thresholds to display any test results\n");
		std::printf("                 within THRESHOLD of an extrema.  If THRESHOLD is zero or less \n");
		std::printf("                 then simple p-value thresholds will be disabled\n");
		std::printf("                 THRESHOLD is a float with recommended value 0.001\n");
		std::printf("test set options:\n");
		std::printf(" The default test set options are '-tf 1' and '-te 0'\n");
		std::printf("  -tf FOLDING    FOLDING may be 0, 1, or 2.  0 means that the base tests are \n");
		std::printf("                 run on only the raw test data.  1 means that the base tests \n");
		std::printf("                 are run on the raw test data and also on a simple transform \n");
		std::printf("                 that emphasizes the lowest bits.  2 means that the base tests\n");
		std::printf("                 are run on a wider variety of transforms of the test data.\n");
		std::printf("  -te EXPANDED   EXPANDED may be 0 or 1.  0 means that the base tests used are\n");
		std::printf("                 the normal ones for PractRand, optimized for sensitivity per \n");
		std::printf("                 time.  1 means that the expanded test set is used, optimized \n");
		std::printf("                 for sensitivity per bit.\n");
		std::printf("test target options:\n");
		std::printf(" At most one test target option should be specified.\n");
		std::printf(" The default test target option is '-ttnormal'\n");
		std::printf("  -ttnormal      Test target: normal - the testing is done on the RNGs output.\n");
		std::printf("  -ttseed64      Test target: RNG seeding from 64 bit integers.  First, the RNG\n");
		std::printf("                 is seeded with a randomly chosen 64 bit integer.  Then 8 bytes\n");
		std::printf("                 of output are taken from the RNG and given to the tests.  Then\n");
		std::printf("                 another seed is chosen at a low hamming distance from the\n");
		std::printf("                 prior seed and another 8 bytes of RNG output are given to the\n");
		std::printf("                 tests.\n");
		std::printf("                 This is repeated indefinitely, with care taken to avoid\n");
		std::printf("                 duplicate seeds.  Eventually (after about a quarter billion\n");
		std::printf("                 seeds are used) issues start to crop up due to the limited\n");
		std::printf("                 size of the seedspace and the inability to keep a complete\n");
		std::printf("                 record of previously chosen seeds.\n");
		//           12345678901234567890123456789012345678901234567890123456789012345678901234567890
		std::printf("  -ttep          Test target: Entropy pooling.  This should only be done on\n");
		std::printf("                 RNGs that support entropy pooling.  It is similar to \n");
		std::printf("                 -ttseed64, but the entropy accumulation methods are used\n");
		std::printf("                 instead of simple seeding, and the amount of entropy used is\n");
		std::printf("                 much larger.\n");
		std::printf("test length options:\n");
		std::printf("  -tlmin LENGTH  sets the minimum test length to LENGTH.  The tests will run on\n");
		std::printf("                 that much data before it starts printing regular results.  A\n");
		std::printf("                 large minimum will prevent it from displaying results on any\n");
		std::printf("                 test lengths other than the maximum length (set by tlmax) and\n");
		std::printf("                 lengths that were explicitly requested (by tlshow).\n");
		std::printf("                 See notes on lengths for details on how to express the length\n");
		std::printf("                 you want.\n");
		std::printf("                 The default minimum is 1.5 seconds (-tlmin 1.5s).\n");
		std::printf("  -tlmax LENGTH  sets the maximum test length to LENGTH.  The tests will stop\n");
		std::printf("                 after that much data.  See notes on lengths for details on how\n");
		std::printf("                 to express the length you want.\n");
		std::printf("                 The default maximum is 32 terabytes (-tlmin 32TB).\n");
		std::printf("  -tlshow LENGTH sets an additional point at which to display interim results.\n");
		std::printf("                 You can set multiple such points if desired.\n");
		std::printf("                 These are in addition to the normal interim results points,\n");
		std::printf("                 which are at every amount of data that is a power of 2 after\n");
		std::printf("                 the minimum and before the maximum.\n");
		std::printf("                 See the notes on lengths for details on how to express the\n");
		std::printf("                 lengths you want.\n");
		std::printf("  -tlfail        Halts testing after interim results are displayed if those\n");
		std::printf("                 results include any failures. (default)\n");
		std::printf("  -tlmaxonly     The opposite of -tlfail\n");
		std::printf("other options:\n");
		std::printf("  -multithreaded  enables multithreaded testing.  Typically up to 5 cores can\n");
		std::printf("                  be used at once.\n");
		std::printf("  -singlethreaded disables multithreaded testing.  (default)\n");
		std::printf("  -seed SEED      specifies a 64 bit integer to seed the tested RNG with.  If\n");
		std::printf("                  no seed is specified then a seed will be chosen randomly.  \n");
		std::printf("                  The value should be expressed in hexadecimal.\n");
		std::printf("notes on lengths:\n");
		//           12345678901234567890123456789012345678901234567890123456789012345678901234567890
		std::printf("  Each of the test length options requires a field named LENGTH.  These fields\n");
		std::printf("  can accept either an amount of time or an amount of data.  In either case, \n");
		std::printf("  several types of units are supported.  \n");
		std::printf("  A time should be expressed as a number postfixed with either s, m, h, or d, \n");
		std::printf("  to express a number of seconds, minutes, hours, or days.  \n");
		std::printf("  example: -tlmin 1.4s (sets the minimum test length to 1.4 seconds)\n");
		std::printf("  An amount of data can be expressed as a number with no postfix, in which case\n");
		std::printf("  the number will be treated as the log-based-2 of the amount of bytes to test\n");
		std::printf("  (in normal target mode) or the log-baed-2 of the number of seeds or strings \n");
		std::printf("  to test in alternate test target modes.\n");
		std::printf("  example: -tlmin 23 (sets the minimum test length to 8 million bytes or 8\n");
		std::printf("    million seeds, depending upon test target mode)\n");
		std::printf("  Alternatively, an amount of data can be expressed as a number followed by \n");
		std::printf("  KB, MB, GB, TB, or PB for kilobytes, megabytes, gigabytes, or petabytes.\n");
		std::printf("  example: -tlmin 14KB (sets the minimum test length to 14 kilobytes\n");
		std::printf("  If the B is omitted on KB, MB, GB, TB, or PB then it treat the metric\n");
		std::printf("  prefixes as refering to numbers of bytes in normal test target mode, or\n");
		std::printf("  numbers of seeds in seeding test target mode, or numbers of strings in \n");
		std::printf("  entropy pooling test target mode.\n");
		std::printf("  example: -tlmin 40M (sets the minimum test length to 40 million bytes or 40\n");
		std::printf("    million seeds, depending upon test target mode)\n");
		std::printf("  A minor detail: I use the de facto standard (in which K means 1024 when\n");
		std::printf("  dealing with quantities of information) not the official standard (in which K\n");
		std::printf("  means 1000 no matter what is being dealt with unless an 'i' follows the 'K').\n");
		//           12345678901234567890123456789012345678901234567890123456789012345678901234567890
		std::exit(0);
	}
	//PractRand::RNGs::vRNG *rng = get_rng(argv[1]);
	RNG_Factories::register_recommended_RNGs();
	RNG_Factories::register_nonrecommended_RNGs();
	RNG_Factories::register_input_RNGs();
	RNG_Factories::register_candidate_RNGs();
	RNG_Factories::RNG_factory_index["dummy"] = RNG_Factories::_generic_notrecommended_RNG_factory<DummyRNG>;
	PractRand::RNGs::vRNG *rng = RNG_Factories::create_rng(argv[1]);
	if (!rng) {
		std::printf("unrecognized RNG name.  aborting.\n");
		std::exit(0);
	}

	bool do_self_test = true;
	bool use_multithreading = false;
	bool end_on_failure = true;
	Uint64 seed = known_good.raw32();//64 bit space, as that's what the interface accepts, but 32 bit random value so that by default it's not too onerous to record/compare/whatever the value by hand
	bool smart_thresholds = true;
	double threshold = 0.2;
	int folding = 1;//0 = no folding, 1 = standard folding, 2 = extra folding
	int expanded = 0;//0 = core test set, 1 = expanded test set
	int mode = 0;//0 = normal, 1 = test seeding, 2 = test entropy pooling
	for (int i = 2; i < argc; i++) {
		int params_left = argc - i - 1;
		//-a
		//-e EXPECTED
		//-p THRESHOLD
		if (false) ;
		else if (!std::strcmp(argv[i], "-a")) {
			smart_thresholds = false;
			threshold = 1;
		}
		else if (!std::strcmp(argv[i], "-e")) {
			if (params_left < 1) {std::printf("command line option %s must be followed by a value\n", argv[i]); std::exit(0);}
			smart_thresholds = true;
			threshold = std::atof(argv[++i]);
			if (threshold < 0.000001 || threshold > 1000) {
				std::printf("invalid smart threshold: -e %s (must be between 0.000001 and 1000)\n", argv[i]);
				std::exit(0);
			}
		}
		else if (!std::strcmp(argv[i], "-p")) {
			if (params_left < 1) {std::printf("command line option %s must be followed by a value\n", argv[i]); std::exit(0);}
			smart_thresholds = false;
			threshold = std::atof(argv[++i]);
			if (threshold < 0.0000000001 || threshold > 1.0) {
				std::printf("invalid p-value threshold: -p %s (must be between 0.0000000001 and 1)\n", argv[i]);
				std::exit(0);
			}
		}
		//-tf FOLDING
		//-te EXPANDED
		else if (!std::strcmp(argv[i], "-tf")) {
			if (params_left < 1) {std::printf("command line option %s must be followed by a value\n", argv[i]); std::exit(0);}
			folding = std::atoi(argv[++i]);
			if (folding < 0 || folding > 2) {
				std::printf("invalid folding test set value: -tf %s\n", argv[i]);
				std::exit(0);
			}
		}
		else if (!std::strcmp(argv[i], "-te")) {
			if (params_left < 1) {std::printf("command line option %s must be followed by a value\n", argv[i]); std::exit(0);}
			expanded = std::atoi(argv[++i]);
			if (expanded < 0 || expanded > 1) {
				std::printf("invalid expanded test set value: -te %s\n", argv[i]);
				std::exit(0);
			}
		}
		//-ttnormal
		//-ttseed64
		//-ttep
		else if (!std::strcmp(argv[i], "-ttnormal")) mode = 0;
		else if (!std::strcmp(argv[i], "-ttseed64")) mode = 1;
		else if (!std::strcmp(argv[i], "-ttep"))     mode = 2;
		//-tlmin LENGTH
		//-tlmax LENGTH
		//-tlshow LENGTH
		else if (!std::strcmp(argv[i], "-tlmin")) i++;
		else if (!std::strcmp(argv[i], "-tlmax")) i++;
		else if (!std::strcmp(argv[i], "-tlshow")) i++;
		//-tlfail
		//-tlmaxonly
		else if (!std::strcmp(argv[i], "-tlfail")) end_on_failure = true;
		else if (!std::strcmp(argv[i], "-tlmaxonly")) end_on_failure = false;

		//-threads
		//-nothreads
		//-seed SEED
		else if (!std::strcmp(argv[i], "-multithreaded")) use_multithreading = true;
		else if (!std::strcmp(argv[i], "-singlethreaded")) use_multithreading = false;
		else if (!std::strcmp(argv[i], "-skip_selftest")) do_self_test = false;
		else if (!std::strcmp(argv[i], "-seed")) {
			if (params_left < 1) {std::printf("command line option %s must be followed by a value\n", argv[i]); std::exit(0);}
			if (!interpret_seed(argv[++i],seed)) {
				std::printf("\"%s\" is not a valid 64 bit hexadecimal seed\n", argv[i]); std::exit(0);
			}
		}
		else {
			std::printf("unrecognized parameter: %s\naborting\n", argv[i]);
			std::exit(0);
		}
	}
#if !defined MULTITHREADING_SUPPORTED
	if (use_multithreading) {
		std::printf("multithreading is not supported on this build.  If multithreading should be supported, try defining the MULTITHREADING_SUPPORTED preprocessor symbol during the build process.\n");
		std::exit(0);
	}
#endif
	enum {TL_MIN = 1, TL_MAX = 2, TL_SHOW = 3};//, TL_FLAG_UNSPECIFIED_UNITS = 16};
	std::map<double,int> show_times;
	std::map<Uint64,int> show_datas;
	double show_min = -2.0;
	double show_max = 1ull << 45;
	//walking parameters a second time to force the mode to be known prior to finding the test lengths
	for (int i = 2; i < argc; i++) {
		int params_left = argc - i - 1;
		if (false) ;
		else if (!std::strcmp(argv[i], "-tlmin")) {
			if (params_left < 1) {std::printf("command line option %s must be followed by a value\n", argv[i]); std::exit(0);}
			double length = interpret_length(argv[++i], !mode);
			if (!length) {std::printf("invalid test length: %s\n", argv[i]);std::exit(0);}
			show_min = length;
		}
		else if (!std::strcmp(argv[i], "-tlmax")) {
			if (params_left < 1) {std::printf("command line option %s must be followed by a value\n", argv[i]); std::exit(0);}
			double length = interpret_length(argv[++i], !mode);
			if (!length) {std::printf("invalid test length: %s\n", argv[i]);std::exit(0);}
			show_max = length;
		}
		else if (!std::strcmp(argv[i], "-tlshow")) {
			if (params_left < 1) {std::printf("command line option %s must be followed by a value\n", argv[i]); std::exit(0);}
			double length = interpret_length(argv[++i], !mode);
			if (!length) {std::printf("invalid test length: %s\n", argv[i]);std::exit(0);}
			if (length < 0) show_times[-length] = TL_SHOW;
			else show_datas[Uint64(length) / Tests::TestBlock::SIZE] = TL_SHOW;
		}
	}
	if (show_min < 0) show_times[-show_min] = TL_MIN;
	else show_datas[Uint64(show_min) / Tests::TestBlock::SIZE] = TL_MIN;
	if (show_max < 0) show_times[-show_max] = TL_MAX;
	else show_datas[Uint64(show_max) / Tests::TestBlock::SIZE] = TL_MAX;

	if (do_self_test) PractRand::self_test_PractRand();


	std::time_t start_time = std::time(NULL);
	TimeUnit start_clock = get_time();

	known_good.seed(seed+1);//the +1 is there just in case the RNG uses the same algorithm as the known good RNG

	PractRand::RNGs::vRNG *testing_rng;
	if (mode == 0) {
		rng->seed(seed);
		testing_rng = rng;
	}
	else if (mode == 1) {
		//it would be nice to print a warning here for RNGs that use generic integer seeding
		//but that's a little difficult atm as there's no way to query whether an RNG does so
		testing_rng = new Seeder_MetaRNG(rng,1<<10);
	}
	else if (mode == 2) {
		if (!(rng->get_flags() & PractRand::RNGs::FLAG::SUPPORTS_ENTROPY_ACCUMULATION)) {
			std::printf("Entropy pooling is not supported by this RNG, so mode --ttep is invalid.\n");
			std::printf("aborting\n");
			std::exit(0);
		}
		rng->reset_entropy();
		Uint64 a = rng->raw64();
		rng->reset_entropy();
		Uint64 b = rng->raw64();
		if (a != b) {
			std::printf("entropy pooling RNG \"%s\" failed basic check 1.\naborting\n", rng->get_name().c_str());
			std::exit(0);
		}
		Uint64 s64 = known_good.raw64();
		rng->reset_entropy();
		rng->add_entropy64(s64);
		Uint64 c1 = rng->raw64();
		Uint64 c2 = rng->raw64();
		rng->reset_entropy();
		rng->add_entropy64(s64);
		Uint64 d = rng->raw64();
		rng->reset_entropy();
		rng->add_entropy64(s64+1);
		Uint64 e1 = rng->raw64();
		Uint64 e2 = rng->raw64();
		if (c1 != d) {
			std::printf("entropy pooling RNG \"%s\" failed basic check 2.\naborting\n", rng->get_name().c_str());
			std::exit(0);
		}
		if (c1 == e1 && c2 == e2) {
			std::printf("entropy pooling RNG \"%s\" probably failed basic check 3.\naborting\n", rng->get_name().c_str());
			std::exit(0);
		}
		rng->seed(seed);
		//I'd like to test varying length entropy strings, but known good EPs are failing eventually when varying length is allowed for some reason
		testing_rng = new EntropyPool_MetaRNG(rng,35,35,1<<10);
	}

	std::printf("RNG = %s, PractRand version %s, seed = 0x", testing_rng->get_name().c_str(), PractRand::version_str);
	if (seed >> 32) std::printf("%lx%08lx", long(seed>>32), long((seed<<32)>>32));
	else std::printf("%lx", long(seed));
	const char *test_set_names[2] = {"normal", "expanded"};
	const char *folding_names[3] = {"none", "standard", "extra"};
	std::printf("\ntest set = %s, folding = %s", test_set_names[expanded], folding_names[folding]);
	if (folding == 1) {
		int native_bits = testing_rng->get_native_output_size();
		if (native_bits > 0) std::printf(" (%d bit)", native_bits);
		else std::printf("(unknown format)");
	}

	std::printf("\n\n");

	Tests::ListOfTests tests( (Tests::TestBaseclass*)NULL);
	if (false) ;
		// speed in testing on 3.3 GHrz Core i5, tested on a very fast RNG -- name = description
		// 5.6 GB / minute -- core = basic tests only, no folding
	else if (folding == 0 && expanded == 0) tests = Tests::Batteries::get_core_tests();

		// 4.4 GB / minute -- standard = basic tests, smart folding
	else if (folding == 1 && expanded == 0) tests = Tests::Batteries::get_standard_tests(testing_rng);

		// 2.5 GB / minute -- extended folding = basic tests, extra folding -- 
	else if (folding == 2 && expanded == 0) tests = Tests::Batteries::get_folded_tests();

		// 1.47 GB / minute -- expanded core = extra tests, no folding
	else if (folding == 0 && expanded == 1) tests = Tests::Batteries::get_expanded_core_tests();

		// 1.03 GB / minute -- expanded standard = extra tests, smart folding
	else if (folding == 1 && expanded == 1) tests = Tests::Batteries::get_expanded_standard_tests(testing_rng);

		// 0.50 GB / minute -- maximum = extra tests, extra folding
	else if (folding == 2 && expanded == 1) tests = Tests::Batteries::get_expanded_folded_tests();
	else {std::printf("internal error\n"); std::exit(1);}

//	Tests::ListOfTests tests = Tests::Batteries::get_expanded_standard_tests(rng);
#if defined MULTITHREADING_SUPPORTED
	TestManager *tman;
	if (use_multithreading) tman = new MultithreadedTestManager(testing_rng, &tests, &known_good);
	else tman = new TestManager(testing_rng, &tests, &known_good);
#else
	TestManager *tman = new TestManager(testing_rng, &tests, &known_good);
#endif

	Uint64 blocks_tested = 0;
	bool already_shown = false;
	Uint64 next_power_of_2 = 1;
	bool showing_powers_of_2 = false;
	double time_passed = 0;
	while (true) {
		Uint64 blocks_to_test = next_power_of_2 - blocks_tested;
		enum {MAX_BLOCKS = 256 * 1024};
		if (blocks_to_test > MAX_BLOCKS) blocks_to_test = MAX_BLOCKS;
		while (!show_datas.empty()) {
			Uint64 data_checkpoint = show_datas.begin()->first - blocks_tested;
			if (data_checkpoint) {
				if (data_checkpoint < blocks_to_test) blocks_to_test = data_checkpoint;
				break;
			}
			int action = show_datas.begin()->second;
			if (action == TL_SHOW) {
				if (!already_shown) show_checkpoint(tman, mode, seed, time_passed, smart_thresholds, threshold, end_on_failure);
				already_shown = true;
			}
			else if (action == TL_MIN) {
				showing_powers_of_2 = true;
				if (!already_shown) show_checkpoint(tman, mode, seed, time_passed, smart_thresholds, threshold, end_on_failure);
				already_shown = true;
			}
			else if (action == TL_MAX) {
				if (!already_shown) show_checkpoint(tman, mode, seed, time_passed, smart_thresholds, threshold, end_on_failure);
				return 0;
			}
			else {std::printf("internal error: unrecognized test length code, aborting\n");std::exit(1);}
			show_datas.erase(show_datas.begin());
		}
		while (!show_times.empty()) {
			double time_checkpoint = show_times.begin()->first - time_passed;
			if (time_checkpoint > 0) break;
			int action = show_times.begin()->second;
			show_times.erase(show_times.begin());
			if (action == TL_SHOW) {
				if (!already_shown) show_checkpoint(tman, mode, seed, time_passed, smart_thresholds, threshold, end_on_failure);
				already_shown = true;
			}
			else if (action == TL_MIN) showing_powers_of_2 = true;
			else if (action == TL_MAX) {
				if (!already_shown) show_checkpoint(tman, mode, seed, time_passed, smart_thresholds, threshold, end_on_failure);
				return 0;
			}
			else {std::printf("internal error: unrecognized test length code, aborting\n");std::exit(1);}
		}

		if (blocks_tested == next_power_of_2) {
			if (showing_powers_of_2) {
				if (!already_shown) show_checkpoint(tman, mode, seed, time_passed, smart_thresholds, threshold, end_on_failure);
				already_shown = true;
			}
			next_power_of_2 <<= 1;
			continue;
		}
		tman->test(blocks_to_test);
		blocks_tested += blocks_to_test;
		already_shown = false;

		double clocks_passed = TimeUnit(get_time() - start_clock) * get_time_period();//may wrap too quickly
		int seconds_passed = std::time(NULL) - start_time;
		if (seconds_passed >= 1000 || seconds_passed > clocks_passed + 2.0) time_passed = seconds_passed;
		else time_passed = clocks_passed;
	}

	return 0;
}



