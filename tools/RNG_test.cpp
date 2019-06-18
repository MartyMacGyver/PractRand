
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

//for access to some functions the dummy PRNG might want to use:
#include "PractRand/rng_internals.h"


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
Deliberately flawed, though still better than many platforms default RNGs
*/

//#include "PractRand/endian.h"

struct BitPermutator {
	//Uint32 lookup32_0[256], lookup32_1[256], lookup32_2[256], lookup32_3[256];
	//Uint32 permutate_value32(Uint32 value)
	Uint16 lookup16[2][256];
	Uint16 permutate_value16(Uint16 value) { return lookup16[0][value & 255] | lookup16[1][value >> 8]; }
	void init(PractRand::RNGs::vRNG *seeder_rng) {
		int bits[16];
		for (int i = 0; i < 16; i++) bits[i] = i;
		for (int i = seeder_rng->raw8() + 256; i > 0; i--) {
			int a, b; a = seeder_rng->raw8() & 15; b = seeder_rng->raw8() & 15;
			int tmp = bits[a]; bits[a] = bits[b]; bits[b] = tmp;
		}
		for (int t = 0; t < 2; t++) for (int i = 0; i < 256; i++) { lookup16[t][i] = 0; }
		for (int old_bp = 0; old_bp < 16; old_bp++) {
			int new_bp_value = 1 << bits[old_bp];
			for (int i = 0; i < 256; i++) if ((i >> (old_bp & 7)) & 1) lookup16[old_bp >> 3][i] |= new_bp_value;
		}
	}
};
BitPermutator permutator;

class DummyRNG : public PractRand::RNGs::vRNG16 {
public:
	//declare state
	Uint32 x, y, z, index;
	//and any helper methods you want:
	static Uint16 rotate16(Uint16 v, int bits) { return (v << bits) | (v >> (16 - bits)); }
	static Uint32 rotate32(Uint32 v, int bits) { return (v << bits) | (v >> (32 - bits)); }
	//implement algorithm
	Uint16 raw16() {
		//x += z ^ index++;
		//y += x ^ z;
		//z += y ^ (y >> 24);
		//return z;

		y ^= x;
		x = rotate16(x, 14);
		x ^= (y << 2) ^ y;
		y = rotate16(y, 7);
		//return x + y;
		//return rotate16(x + y, 6) + x;
		//return permutator.permutate_value16(x + y) + y;
		return permutator.permutate_value16(permutator.permutate_value16(x+y) + y);
		//return rotate16(x ^ (y + ((y) << 3)), 9) + (x ^ (x >> 4));
		//return rotate16(x, 6) ^ rotate16(x, 9) ^ x;
		//Uint16 tmp = x + y; tmp = rotate16(tmp, 6) ^ rotate16(tmp, 9) ^ tmp; return rotate16(tmp, 6) + x;
	}
	//allow PractRand to be aware of your internal state
	//uses include: default seeding mechanism (individual PRNGs can override), state serialization/deserialization, maybe eventually some avalanche testing tools
	void walk_state(PractRand::StateWalkingObject *walker) {
		walker->handle(x);
		walker->handle(y);
		walker->handle(z);
		walker->handle(index);
	}
	//seeding from integers
	//not actually necessary, in the absence of such a method a default seeding-from-integer path will use walk_state to randomize the member variables
	//note that a separate path exists for seeding-from-another-PRNG
	void seed(Uint64 s) {
		permutator.init(&known_good);
		x = y = z = s;
		y = s >> 32;
		//for (int i = 0; i < 6; i++) raw32();
		index = 0;
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
	std::printf("  ");//2 characters
	//NAME
	if (true) {// 34 characters
		std::printf("%s", result.name.c_str());
		int len = result.name.length();
		for (int i = len; i < 34; i++) std::printf(" ");
	}

	//RAW TEST RESULT
	if (true) {// 10 characters?
		double raw = result.get_raw();
		if (raw > 99999.0) std::printf("R>+99999  ");
		else if (raw < -99999.0) std::printf("R<-99999  ");
		else if (std::fabs(raw) < 999.95) std::printf("R=%+6.1f  ", raw);
		else std::printf("R=%+6.0f  ", raw);
		//if (std::fabs(raw) < 99999.5) std::printf(" ");
		//if (std::fabs(raw) < 999999.5) std::printf(" ");
		//if (std::fabs(raw) < 9999999.5) std::printf(" ");
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
	double as = (std::fabs(result.get_suspicion()) + 1 - 1) * dec;// +1 for suspicion conversion, -1 to account for there being 2 failure regions (near-zero and near-1)
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
		else if (rs >3.0) std::printf("suspicious       ");
		else if (rs >2.0) std::printf("mildly suspicious");
		else if (rs >1.0) std::printf("unusual          ");
		else if (rs >0.0) std::printf("normalish       ");
		else              std::printf("normal           ");
	}
	std::printf("\n");
	return rs;
}

#include "SeedingTester.h"

const char *seed_str = NULL;

double show_checkpoint(TestManager *tman, int mode, Uint64 seed, double time, bool smart_thresholds, double threshold, bool end_on_failure) {
	std::printf("rng=%s", tman->get_rng()->get_name().c_str());

	std::printf(", seed=");
	if (tman->get_rng()->get_flags() & PractRand::RNGs::FLAG::SEEDING_UNSUPPORTED) {
		if (seed_str) std::printf("%s", seed_str);
		else std::printf("unknown");
	}
	else {
		if (seed >> 32) std::printf("0x%lx%08lx", long(seed >> 32), long(seed & 0xFFffFFff));
		else std::printf("0x%lx", long(seed));
	}
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
	double total_weight = 0, min_weight = 9999999;
	for (int i = 0; i < results.size(); i++) {
		double weight = results[i].get_weight();
		total_weight += weight;
		if (weight < min_weight) min_weight = weight;
	}
	if (min_weight <= 0) {
		std::printf("error: result weight too small\n");
		std::exit(1);
	}
	std::vector<int> marked;
	for (int i = 0; i < results.size(); i++) {
		results[i].set_weight(results[i].get_weight() / total_weight);
		if (!smart_thresholds) {
			if (std::fabs(0.5 - results[i].get_pvalue()) < 0.5 - threshold) continue;
		}
		else {
			double T = threshold * results[i].get_weight() * 0.5;
			if (std::fabs(0.5 - results[i].get_pvalue()) < (0.5 - T)) continue;
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
	std::fflush(stdout);
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
		scale = -1;//one second
		expect_B = false;
		break;
	case 'm':
		scale = -60;//one minute
		expect_B = false;
		break;
	case 'h':
		scale = -3600;//one hour
		expect_B = false;
		break;
	case 'd':
		scale = -86400;//one day
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

#include "PractRand/tests/Birthday.h"
#include "PractRand/tests/FPMulti.h"
#include "PractRand/tests/DistFreq4.h"
#include "PractRand/tests/Gap16.h"
PractRand::Tests::ListOfTests testset_BirthdaySystematic() {
	//return PractRand::Tests::ListOfTests(new PractRand::Tests::FPMulti(3,0));
	//return PractRand::Tests::ListOfTests(new PractRand::Tests::BirthdayAlt(10), new PractRand::Tests::Birthday32());
	//return PractRand::Tests::ListOfTests(new PractRand::Tests::BirthdayAlt(22));
	//return PractRand::Tests::ListOfTests(new PractRand::Tests::BirthdaySystematic(25));
	//return PractRand::Tests::ListOfTests(new PractRand::Tests::Birthday32());
	//return PractRand::Tests::ListOfTests(new PractRand::Tests::Birthday64());
	//return PractRand::Tests::ListOfTests(new PractRand::Tests::BirthdayLamda1(20));
	return PractRand::Tests::ListOfTests(new PractRand::Tests::Rep16());
}
struct UnfoldedTestSet {
	int number;
	PractRand::Tests::ListOfTests(*callback)();
	const char *name;
};
UnfoldedTestSet test_sets[] = {
	{ 0, PractRand::Tests::Batteries::get_core_tests, "core" },//default value must come first
	{ 1, PractRand::Tests::Batteries::get_expanded_core_tests, "expanded" },
	{ 10, testset_BirthdaySystematic, "special (Birthday)" },
	{ -1, NULL, NULL }
};
int lookup_te_value(int te) {
	for (int i = 0; true; i++) {
		if (test_sets[i].number == te) return i;
		if (test_sets[i].number == -1) return -1;
	}
}

int main(int argc, char **argv) {
	PractRand::initialize_PractRand();
	std::printf("RNG_test using PractRand version %s\n", PractRand::version_str);
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
		std::printf("-tlmin, -tlmax, -tlshow, -multithreaded, -singlethreaded, and -seed.\n");
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
		std::printf("                 ... and now additional value(s) are supported.  Setting this \n");
		std::printf("                 to 10 will use an systematically expanding Birthday Spacings \n");
		std::printf("                 Test in place of a normal test set.  This test is separate \n");
		std::printf("                 because it uses too much memory to run concurrently with other\n");
		std::printf("                 tests\n");
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
		std::printf("                 This is repeated indefinitely, with care taken to minimize\n");
		std::printf("                 the amount of duplicate seeds used.  \n");
		//           12345678901234567890123456789012345678901234567890123456789012345678901234567890
		std::printf("  -ttep          Test target: Entropy pooling.  This should only be done on\n");
		std::printf("                 RNGs that support entropy pooling.  It is similar to \n");
		std::printf("                 -ttseed64, but the entropy accumulation methods are used\n");
		std::printf("                 instead of simple seeding, and the amount of entropy used is\n");
		std::printf("                 much larger.\n");
		std::printf("  -walk_sequence  Some test-target modes will search seeds sequentially, \n");
		std::printf("                  each subsequent seed 1 higher than the previous.\n");
		std::printf("  -walk_greycode  Some test-target modes will search seeds in a simple \n");
		std::printf("                  greycoded sequence, each subsequent seed at Hamming \n");
		std::printf("                  distance 1 from the prior in a strict order.\n");
		std::printf("  -walk_random    Some test-target modes will search seeds in a random walk,\n");
		std::printf("                  each subsequent seed chosen at random from unused values \n");
		std::printf("                  at Hamming distance 1 from the prior value.\n");
		std::printf("  -walk_random_l  Some test-target modes will search seeds in a random walk,\n");
		std::printf("                  each subsequent seed chosen at random from unused values \n");
		std::printf("                  at Hamming distance 1 from the prior value, but lower bits\n");
		std::printf("                  will be changed much more often than higher bits.\n");
		std::printf("  -walk_random_h  Some test-target modes will search seeds in a random walk,\n");
		std::printf("                  each subsequent seed chosen at random from unused values \n");
		std::printf("                  at Hamming distance 1 from the prior value, but higher bits\n");
		std::printf("                  will be changed much more often than lower bits.\n");
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
		std::printf("                  The value should be expressed in hexadecimal.  An '0x' prefix\n");
		std::printf("                  on the seed is acceptable but not necessary.\n");
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
		std::printf("  example: -tlmin 40M (sets the minimum test length to ~40 million bytes or ~40\n");
		std::printf("    million seeds, depending upon test target mode)\n");
		std::printf("  A minor detail: I use the de facto standard (in which K means 1024 when\n");
		std::printf("  dealing with quantities of binary information) not the official standard (in\n");
		std::printf("  which K means 1000 no matter what is being dealt with unless an 'i' follows\n");
		std::printf("  the 'K').\n");
		//           12345678901234567890123456789012345678901234567890123456789012345678901234567890
		std::exit(0);
	}
	//PractRand::RNGs::vRNG *rng = get_rng(argv[1]);
	RNG_Factories::register_recommended_RNGs();
	RNG_Factories::register_nonrecommended_RNGs();
	RNG_Factories::register_input_RNGs();
	RNG_Factories::register_candidate_RNGs();
	RNG_Factories::RNG_factory_index["dummy"] = RNG_Factories::_generic_notrecommended_RNG_factory<DummyRNG>;
	Seeder_MetaRNG::register_name();
	EntropyPool_MetaRNG::register_name();
	std::string errmsg;
	RNGs::vRNG *rng = RNG_Factories::create_rng(argv[1], &errmsg);
	if (!rng) {
		if (errmsg.empty()) std::fprintf(stderr, "unrecognized RNG name.  aborting.\n");
		else std::fprintf(stderr, "%s\n", errmsg.c_str());
		std::exit(1);
	}

	bool do_self_test = true;
	bool use_multithreading = false;
	bool end_on_failure = true;
	bool smart_thresholds = true;
	double threshold = 0.1;
	int folding = 1;//0 = no folding, 1 = standard folding, 2 = extra folding
	int test_set_index = lookup_te_value(0);
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
			int expanded = std::atoi(argv[++i]);
			test_set_index = lookup_te_value(expanded);//0 maps to 0, but other values may not map to themselves
			if (test_set_index == -1) {
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
			seed_str = argv[++i];
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

	Uint64 seed = known_good.raw32();//64 bit space, as that's what the interface accepts, but 32 bit random value so that by default it's not too onerous to record/compare/whatever the value by hand
	if (seed_str && !(rng->get_flags() & PractRand::RNGs::FLAG::SEEDING_UNSUPPORTED)) {
		if (!interpret_seed(seed_str, seed)) {
			std::printf("\"%s\" is not a valid 64 bit hexadecimal seed\n", seed_str);
			std::exit(0);
		}
	}
	known_good.seed(seed + 1);//the +1 is there just in case the RNG uses the same algorithm as the known good RNG

	PractRand::RNGs::vRNG *testing_rng;
	if (mode == 0) {
		rng->seed(seed);
		testing_rng = rng;
	}
	else if (mode == 1) {
		//it would be nice to print a warning here for RNGs that use generic integer seeding
		//but that's a little difficult atm as there's no way to query whether an RNG does so
		testing_rng = new Seeder_MetaRNG(rng);
		testing_rng->seed(seed);
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
		testing_rng = new EntropyPool_MetaRNG(rng,48,64);
		testing_rng->seed(seed);
	}

	std::printf("RNG = %s, seed = ", testing_rng->get_name().c_str());
	if (testing_rng->get_flags() & PractRand::RNGs::FLAG::SEEDING_UNSUPPORTED) {
		if (seed_str) std::printf("%s", seed_str);
		else std::printf("unknown");
	}
	else {
		if (seed >> 32) std::printf("0x%lx%08lx", long(seed >> 32), long((seed << 32) >> 32));
		else std::printf("0x%lx", long(seed));
	}
	const char *folding_names[3] = {"none", "standard", "extra"};
	std::printf("\ntest set = %s, folding = %s", test_sets[test_set_index].name, folding_names[folding]);
	if (folding == 1) {
		int native_bits = testing_rng->get_native_output_size();
		if (native_bits > 0) std::printf(" (%d bit)", native_bits);
		else std::printf("(unknown format)");
	}

	std::printf("\n\n");

	Tests::ListOfTests tests( (Tests::TestBaseclass*)NULL);
	if (test_set_index == -1) { std::printf("internal error\n"); std::exit(1); }
	if (false) ;
	else if (folding == 0) tests = test_sets[test_set_index].callback();
	else if (folding == 1) tests = Tests::Batteries::apply_standard_foldings(testing_rng, test_sets[test_set_index].callback);
	else if (folding == 2) tests = Tests::Batteries::apply_extended_foldings(test_sets[test_set_index].callback);
	else { std::printf("internal error\n"); std::exit(1); }

//	Tests::ListOfTests tests = Tests::Batteries::get_expanded_standard_tests(rng);
#if defined MULTITHREADING_SUPPORTED
	TestManager *tman;
	if (use_multithreading) tman = new MultithreadedTestManager(&tests, &known_good);
	else tman = new TestManager(&tests, &known_good);
#else
	TestManager *tman = new TestManager(&tests, &known_good);
#endif
	tman->reset(testing_rng);

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



