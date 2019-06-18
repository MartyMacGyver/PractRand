#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <cstring>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <sstream>
#include <algorithm>
//#include <map>


//master header, includes everything in PractRand for both 
//  practical usage and research... 
//  EXCEPT it does not include specific algorithms
#include "PractRand_full.h"

//using issue_error here
#include "PractRand/rng_internals.h"

//specific testing algorithms, to detect bias in supposedly random numbers
#include "PractRand/Tests/BCFN.h"
#include "PractRand/Tests/Gap16.h"
#include "PractRand/Tests/DistC6.h"
#include "PractRand/Tests/transforms.h"
#include "PractRand/Tests/FPF.h"
#include "PractRand/Tests/FPMulti.h"
#include "PractRand/Tests/BRank.h"
#include "PractRand/Tests/CoupGap.h"
#include "PractRand/Tests/mod3.h"

//specific RNG algorithms, to produce (pseudo-)random numbers
#include "PractRand/RNGs/all.h"
#include "PractRand/RNGs/other/transform.h"
#include "PractRand/RNGs/other/mult.h"
#include "PractRand/RNGs/other/simple.h"
#include "PractRand/RNGs/other/fibonacci.h"
#include "PractRand/RNGs/other/indirection.h"
#include "PractRand/RNGs/other/special.h"

using namespace PractRand;
using namespace PractRand::Tests;

//some helpers for the sample programs:
#include "multithreading.h"
#include "TestManager.h"
#include "MultithreadedTestManager.h"





double ref_p117[117] = {
	0.00001, 0.00002, 0.00005, 0.0001, 0.0002, 0.0005, 0.001, 0.002, 0.005,
	0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10,
	0.11, 0.12, 0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.19, 0.20,
	0.21, 0.22, 0.23, 0.24, 0.25, 0.26, 0.27, 0.28, 0.29, 0.30,
	0.31, 0.32, 0.33, 0.34, 0.35, 0.36, 0.37, 0.38, 0.39, 0.40,
	0.41, 0.42, 0.43, 0.44, 0.45, 0.46, 0.47, 0.48, 0.49, 0.50,
	0.51, 0.52, 0.53, 0.54, 0.55, 0.56, 0.57, 0.58, 0.59, 0.60,
	0.61, 0.62, 0.63, 0.64, 0.65, 0.66, 0.67, 0.68, 0.69, 0.70,
	0.71, 0.72, 0.73, 0.74, 0.75, 0.76, 0.77, 0.78, 0.79, 0.80,
	0.81, 0.82, 0.83, 0.84, 0.85, 0.86, 0.87, 0.88, 0.89, 0.90,
	0.91, 0.92, 0.93, 0.94, 0.95, 0.96, 0.97, 0.98, 0.99,
	0.995, 0.998, 0.999, 0.9995, 0.9998, 0.9999, 0.99995, 0.99998, 0.99999
};
double ref_p129[129] = {
	0.0000001, 0.0000002, 0.0000005,
	0.000001, 0.000002, 0.000005, 0.00001, 0.00002, 0.00005,
	0.0001, 0.0002, 0.0005, 0.001, 0.002, 0.005,
	0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10,
	0.11, 0.12, 0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.19, 0.20,
	0.21, 0.22, 0.23, 0.24, 0.25, 0.26, 0.27, 0.28, 0.29, 0.30,
	0.31, 0.32, 0.33, 0.34, 0.35, 0.36, 0.37, 0.38, 0.39, 0.40,
	0.41, 0.42, 0.43, 0.44, 0.45, 0.46, 0.47, 0.48, 0.49, 0.50,
	0.51, 0.52, 0.53, 0.54, 0.55, 0.56, 0.57, 0.58, 0.59, 0.60,
	0.61, 0.62, 0.63, 0.64, 0.65, 0.66, 0.67, 0.68, 0.69, 0.70,
	0.71, 0.72, 0.73, 0.74, 0.75, 0.76, 0.77, 0.78, 0.79, 0.80,
	0.81, 0.82, 0.83, 0.84, 0.85, 0.86, 0.87, 0.88, 0.89, 0.90,
	0.91, 0.92, 0.93, 0.94, 0.95, 0.96, 0.97, 0.98, 0.99,
	0.995, 0.998, 0.999, 0.9995, 0.9998, 0.9999,
	0.99995, 0.99998, 0.99999, 0.999995, 0.999998, 0.999999,
	0.9999995, 0.9999998, 0.9999999
};
double ref_p129_with_formatting[] = {
	0.0000001, 0.0000002, 0.0000005, -1,
	0.000001, 0.000002, 0.000005, -1,
	0.00001, 0.00002, 0.00005, -1,
	0.0001, 0.0002, 0.0005, -1,
	0.001, 0.002, 0.005, -1,
	0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10, -1,
	0.11, 0.12, 0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.19, 0.20, -1,
	0.21, 0.22, 0.23, 0.24, 0.25, 0.26, 0.27, 0.28, 0.29, 0.30, -1,
	0.31, 0.32, 0.33, 0.34, 0.35, 0.36, 0.37, 0.38, 0.39, 0.40, -1,
	0.41, 0.42, 0.43, 0.44, 0.45, 0.46, 0.47, 0.48, 0.49, 0.50, -1,
	0.51, 0.52, 0.53, 0.54, 0.55, 0.56, 0.57, 0.58, 0.59, 0.60, -1,
	0.61, 0.62, 0.63, 0.64, 0.65, 0.66, 0.67, 0.68, 0.69, 0.70, -1,
	0.71, 0.72, 0.73, 0.74, 0.75, 0.76, 0.77, 0.78, 0.79, 0.80, -1,
	0.81, 0.82, 0.83, 0.84, 0.85, 0.86, 0.87, 0.88, 0.89, 0.90, -1,
	0.91, 0.92, 0.93, 0.94, 0.95, 0.96, 0.97, 0.98, 0.99, -1,
	0.995, 0.998, 0.999, -1,
	0.9995, 0.9998, 0.9999, -1,
	0.99995, 0.99998, 0.99999, -1,
	0.999995, 0.999998, 0.999999, -1,
	0.9999995, 0.9999998, 0.9999999, -2
};
void print_ss(const SampleSet &ss, const std::string &name, Uint64 blocks) {
//	std::printf("{\"BCFN-%d/%d\",%7.0f,%5d, %d, {", tbits, 1<<stride_L2, double(Uint64(std::pow(2,length_L2) / 1024)), (int)ss.size(), (int)ss.num_duplicates());
//	for (int i = 0; i < 117; i++) std::printf("%s%+7.3f", i ? "," : "", ss.get_result_by_percentile(ref_p[i]));
//	std::printf("}, %+.4f, %+.4f, %.4f},\n", ss.get_result_by_percentile(0.5), ss.get_mean(), ss.get_stddev());
	std::printf("{\"%s\",%9.0f,%5d,%4d, {", name.c_str(), (double)blocks, (long)ss.size(), (long)ss.num_duplicates());
	for (int i = 0; i < 129; i++) {
		if (ref_p129[i] >= 0.01 && ref_p129[i] <= 0.99) std::printf("%s%+7.3f", i ? "," : "", ss.get_result_by_percentile(ref_p129[i]));
		else std::printf("%s%+10.5f", i ? "," : "", ss.get_result_by_percentile(ref_p129[i]));
	}
	std::printf("}, %+.4f, %+.4f, %.4f, %d},\n", ss.get_result_by_percentile(0.5), ss.get_mean(), ss.get_stddev(), 0);
}


double fake_bcfn(PractRand::RNGs::vRNG *known_good, int tbits, Uint64 n) {
	PractRand::RNGs::LightWeight::sfc32 rng(known_good);
	int size = 1 << tbits;
	int mask = size - 1;
	Uint32 cur = rng.raw32();
	std::vector<Uint64> table; table.resize(size, 0);
	std::vector<double> probs; probs.resize(size, 1.0/size);
	n = (n + 7) >> 4;
	while (n) {
		Uint32 n32 = Uint32(n);
		if (n32 != n) n32 = 1<<30;
		n -= n32;
		for (; n32 > 0; n32--) {
			table[(cur >>  0) & mask]++;
			table[(cur >>  1) & mask]++;
			table[(cur >>  2) & mask]++;
			table[(cur >>  3) & mask]++;
			table[(cur >>  4) & mask]++;
			table[(cur >>  5) & mask]++;
			table[(cur >>  6) & mask]++;
			table[(cur >>  7) & mask]++;
			table[(cur >>  8) & mask]++;
			table[(cur >>  9) & mask]++;
			table[(cur >> 10) & mask]++;
			table[(cur >> 11) & mask]++;
			table[(cur >> 12) & mask]++;
			table[(cur >> 13) & mask]++;
			table[(cur >> 14) & mask]++;
			table[(cur >> 15) & mask]++;
			cur >>= 16;
			cur |= rng.raw32() << 16;
		}
	}
	int reduced_size = size;
	//if (!level) reduced_size = simplify_prob_table(size, samples / 32, &probs[0], &tempcount[0], true, true);
	double rv = PractRand::Tests::g_test(reduced_size, &probs[0], &table[0]);
	double rn = PractRand::Tests::math_chisquared_to_normal(rv, reduced_size-1);
	return rn;
}
double fake_bcfn2(PractRand::RNGs::vRNG *known_good, int tbits, Uint64 n, double p) {
	if (p == 0.5) return fake_bcfn(known_good, tbits, n);
	if (p <= 0 || p >= 1) issue_error();
	PractRand::RNGs::LightWeight::sfc32 rng(known_good);
	int size = 1 << tbits;
	int mask = size - 1;
	Uint32 p_i = Uint32(std::floor(p * std::pow(2.0, 32)));
	Uint32 cur = rng.raw32();
	std::vector<Uint64> table; table.resize(size, 0);
	std::vector<double> probs; probs.resize(size);
	for (Uint32 i = 0; i < size; i++) {
		int ones = count_bits32(i);
		probs[i] = std::pow(p, ones) * std::pow(1-p, tbits-ones);
	}
	n = (n + 3) >> 3;
	while (n) {
		Uint32 n32 = Uint32(n);
		if (n32 != n) n32 = 1<<30;
		n -= n32;
		for (; n32 > 0; n32--) {
			table[(cur >>  0) & mask]++;
			table[(cur >>  1) & mask]++;
			table[(cur >>  2) & mask]++;
			table[(cur >>  3) & mask]++;
			table[(cur >>  4) & mask]++;
			table[(cur >>  5) & mask]++;
			table[(cur >>  6) & mask]++;
			table[(cur >>  7) & mask]++;
#define BIT (rng.raw32() < p_i ? 1 : 0)
			Uint32 next8 = (BIT);
			next8 |= (BIT) << 1;
			next8 |= (BIT) << 2;
			next8 |= (BIT) << 3;
			next8 |= (BIT) << 4;
			next8 |= (BIT) << 5;
			next8 |= (BIT) << 6;
			next8 |= (BIT) << 7;
#undef BIT
			cur >>= 8;
			cur |= next8 << 24;
		}
	}
	int reduced_size = size;
	//if (!level) reduced_size = simplify_prob_table(size, samples / 32, &probs[0], &tempcount[0], true, true);
	double rv = PractRand::Tests::g_test(reduced_size, &probs[0], &table[0]);
	double rn = PractRand::Tests::math_chisquared_to_normal(rv, reduced_size-1);
	return rn;
}
SampleSet fake_bcfn_dist(PractRand::RNGs::vRNG *known_good, int tbits, Uint64 n, Uint32 samples, double p) {
	SampleSet ss;
	if (p == 0.5) for (Uint32 i = 0; i < samples; i++) ss._add(fake_bcfn(known_good, tbits, n));
	else for (Uint32 i = 0; i < samples; i++) ss._add(fake_bcfn2(known_good, tbits, n, p));
	ss._normalize();
	return ss;
}
void print_fake_bcfn_dist(int tbits, int stride_L2, double length_L2, int samples, bool unbalanced) {
	PractRand::RNGs::Polymorphic::hc256 known_good(PractRand::SEED_AUTO);
	SampleSet ss;
	static double chance_skipped[15] = {
		0.0,              //1 bit
		0.5,              //2 bit
		0.375,            //4 bit
		0.2734375,        //8 bit
		0.196380615234375,//16 bit
		0.139949934091419,//32 bit
		0.0993467537479669,//64 bit
		0.0703860921700151,//128 bit
		0.0498191099361402,//256 bit
		0.0352446354858388,//512 bit
		0.0249278058726663,//1 Kbit
		0.0176287723815027,//2 Kbit
		0.0124661853439194,//4 Kbit
		0.0088151932052590,//8 Kbit
		0.0062333780055594//16 Kbit
	};
	int level = stride_L2 + 3;
	double even_chance = (level <= 15) ? chance_skipped[level] : (chance_skipped[15] * std::pow(0.5, 0.5 * (level-15)));
	double p = unbalanced ? (even_chance + 1)*0.5 : 0.5;
	double unskipped_chance = unbalanced ? 1 : 1 - even_chance;
	ss = fake_bcfn_dist(&known_good, tbits, std::pow(2, length_L2 + 3 - level) * unskipped_chance - tbits + 1, samples, p);
	std::printf("{\"%s-%d/%d\",%9.0f,%5d,%4d, {", unbalanced?"BCFNU":"BCFN", tbits, 1<<stride_L2, double(Uint64(std::pow(2,length_L2) / 1024)), (int)ss.size(), (int)ss.num_duplicates());
	for (int i = 0; i < 129; i++) std::printf("%s%+7.3f", i ? "," : "", ss.get_result_by_percentile(ref_p129[i]));
	std::printf("}, %+.4f, %+.4f, %.4f, %d},\n", ss.get_result_by_percentile(0.5), ss.get_mean(), ss.get_stddev(), 0);
}
void blah_bcfn() {
	for (int n = 1<<10; n <= 1<<24; n<<=2) {
		for (int stride = 3; stride <= 4; stride++) for (double len = 20+stride/2; len <= 20+stride/2; len++) {
			//print_fake_bcfn_dist(6, 2,len,n, false);
			//print_fake_bcfn_dist(6, 2,len+std::log(1.5)/std::log(2.0),n, false);
			//print_fake_bcfn_dist(9, stride,len,n, true);
			print_fake_bcfn_dist(9, stride,len+std::log(1.5)/std::log(2.0),n, true);
		}
	}
}
Uint64 generate_binomial_dist(PractRand::RNGs::vRNG *known_good, Uint64 sample_length) {
	//returns number of 0s in (sample_length) random bits
	if (sample_length > 1ull << 12) {
		double p = known_good->randf();
		double n = Tests::math_pvalue_to_normaldist(p);
		double mean = sample_length * 0.5;
		double dev = sqrt(sample_length * 0.5 * 0.5);
		//double delta = 1.0 / dev;
		double rv = mean + n * dev;
		return Uint64(rv);
	}
	Uint32 len = sample_length;
	Uint32 rv = 0;
	for (; len >= 32; len -= 32) rv += Tests::count_bits32(known_good->raw32());
	for (; len >= 8; len -= 8) rv += Tests::count_bits8(known_good->raw8());
	for (; len >= 1; len -= 1) rv += known_good->raw32() & 1;	
	return rv;
}
void fake_fpf_raw(PractRand::RNGs::vRNG *known_good, int tbits, Uint64 sample_length, int trials, SampleSet &ss_raw) {
	std::vector<Uint64> counts;
	long size = 1 << tbits;
	counts.resize(size);
	long mask = size - 1;
	for (long i = 0; i < trials; i++) {
		for (long x = 0; x <= mask; x++) counts[x] = 0;
		Uint64 samples_left = sample_length;
		while (samples_left >= 1024) {
			for (int x = 0; x < 256; x++) {
				counts[known_good->raw32() & mask]++; counts[known_good->raw32() & mask]++; counts[known_good->raw32() & mask]++; counts[known_good->raw32() & mask]++;
			}
			samples_left -= 1024;
		}
		long samples_left2 = samples_left;
		for (long x = 0; x < samples_left2; x++) counts[known_good->raw32() & mask]++;//*/
		/*counts[0] = sample_length;
		for (int b = 0; b < tbits; b++) {
			for (int x = 0; x < (1<<b); x++) {
				Uint64 base = counts[x];
				Uint64 half = generate_binomial_dist(known_good, base);
				counts[x] = half;
				counts[x + (1<<b)] = base-half;
			}
		}//*/

		double raw = raw = Tests::g_test_flat(mask+1,&counts[0]);
		ss_raw._add(Tests::math_chisquared_to_normal(raw, mask));
		//ss_raw._add(Tests::math_chisquared_to_pvalue(raw, mask));
		//ss_raw._add(TestResult::pvalue_to_suspicion(Tests::math_chisquared_to_pvalue(raw, mask)));
		//ss_raw._add(raw);
	}
	ss_raw._normalize();
}
void print_fake_fpf_intra(int tbits, Uint64 sample_length, int trials) {
	PractRand::RNGs::Polymorphic::hc256 known_good(PractRand::SEED_AUTO);
	SampleSet ss_raw;
	fake_fpf_raw(&known_good, tbits, sample_length, trials, ss_raw);
	//double offset = sample_length * std::log(sample_length * std::pow(0.5, tbits));
	SampleSet ss;
	//for (int i = 0; i < ss_raw.size(); i++) ss._add(Tests::math_chisquared_to_normal((ss_raw.get_result_by_index(i)+offset)*2, (1<<tbits)-1));
	//ss._normalize();
	ss = ss_raw;


	std::printf("{\"FPF-%d+6/8+\",%9.0f,%5d,%4d, {", tbits, double(sample_length), (int)ss.size(), (int)ss.num_duplicates());
	for (int i = 0; i < 129; i++) {
		if (ref_p129[i] >= 0.01 && ref_p129[i] <= 0.99) std::printf("%s%+7.3f", i ? "," : "", ss.get_result_by_percentile(ref_p129[i]));
		else std::printf("%s%+10.5f", i ? "," : "", ss.get_result_by_percentile(ref_p129[i]));
	}
	std::printf("}, %+.4f, %+.4f, %.4f, %d},\n", ss.get_result_by_percentile(0.5), ss.get_mean(), ss.get_stddev(), 0);
}
void print_fake_fpf_others(int tbits, Uint64 sample_length, int trials) {
}
void blah_fpf_all2() {
	PractRand::RNGs::Polymorphic::hc256 known_good(PractRand::SEED_AUTO);
	for (int num_platters = 1; num_platters <= 50; num_platters++) {
		SampleSet ss;
		for (int trial = 0; trial < 1 << 26; trial++) {
			double sum = 0;
			for (int p = 0; p < num_platters; p++) {
				double susp = TestResult::pvalue_to_suspicion(known_good.randlf());
				sum += susp * susp - 4.162737902123020;
			}
			sum /= std::sqrt(double(num_platters));
			sum /= 9.308158403091918;
			ss._add(sum);
		}
		ss._normalize();
		std::printf("{\"FPF:all2\",%d,%5d,%4d, {", num_platters, (int)ss.size(), (int)ss.num_duplicates());
		for (int i = 0; i < 129; i++) {
			if (ref_p129[i] >= 0.01 && ref_p129[i] <= 0.99) std::printf("%s%+7.3f", i ? "," : "", ss.get_result_by_percentile(ref_p129[i]));
			else std::printf("%s%+10.5f", i ? "," : "", ss.get_result_by_percentile(ref_p129[i]));
		}
		std::printf("}, %+.4f, %+.4f, %.4f, %d},\n", ss.get_result_by_percentile(0.5), ss.get_mean(), ss.get_stddev(), 0);
	}
}
void blah_fpf() {
	for (int trials = 1 << 0; trials <= 1 << 24; trials <<= 2) {
		printf("\n\n\n");
		for (int tbits = 2; tbits <= 3; tbits+=1) {
			for (Uint64 length = 1ull<<(tbits+9); length <= 1ull<<(tbits+9); length <<= 1) {
				print_fake_fpf_intra(tbits, length, trials);
				if (length > 1) print_fake_fpf_intra(tbits, length + (length >> 1), trials);
			}
		}
	}
}


void find_test_distributions() {
	std::time_t start_time = std::time(NULL);
	std::clock_t start_clock = std::clock();

	PractRand::RNGs::Polymorphic::hc256 known_good(PractRand::SEED_AUTO);

	PractRand::RNGs::Polymorphic::efiix32x48 rng(&known_good);

	//Tests::ListOfTests tests(new Tests::BCFN2(2,13));
	//Tests::ListOfTests tests(new Tests::Gap16());
	//Tests::ListOfTests tests(new Tests::BRank(40));
	//Tests::ListOfTests tests(new Tests::BCFN_FF(2, 13));
	Tests::ListOfTests tests(new Tests::mod3_simple());
	//Tests::ListOfTests tests = Tests::Batteries::get_core_tests();
	//Tests::ListOfTests tests = Tests::Batteries::get_expanded_core_tests();
	//Tests::ListOfTests tests(new Tests::DistC6(9,0, 1,0,0));
	//Tests::ListOfTests tests(new Tests::DistC6(6,1, 1,0,0));
	//Tests::ListOfTests tests(new Tests::DistC6(5,2, 1,0,0));
	//Tests::ListOfTests tests(new Tests::DistC6(4,3, 0,0,1));
	//Tests::ListOfTests tests(new Tests::DistC6(5,3, 1,0,1));
	//Tests::ListOfTests tests(new Tests::CoupGap());
	//Tests::ListOfTests tests(new Tests::FPF(0,14,6));
	//Tests::ListOfTests tests(new Tests::FPF(3,14,6));
	//Tests::ListOfTests tests(new Tests::FPF(4,14,6));
	//Tests::ListOfTests tests(new Tests::FPF(5,14,6));
	//Tests::ListOfTests tests(new Tests::FPF(6,14,6));
	//Tests::ListOfTests tests(new Tests::FPMulti(4, 0));
	TestManager tman(&tests, &known_good);
	tman.reset(&rng);

	//Uint64 test_size = 1 << 16;
	//test_size *= Tests::TestBlock::SIZE;

	std::map<std::string,std::map<Uint64,SampleSet> > data;
	Uint64 next_checkpoint = 1;
	for (Uint64 n = 0; n <= 1ull<<30; n++) {
		if (n == next_checkpoint) {
			enum {CHUNKY = 1 << 12};
			if (next_checkpoint < CHUNKY) next_checkpoint <<= 1; else next_checkpoint += CHUNKY;
			std::printf("\n\n\n\n");
			std::printf("==================================================\n");
			if (n & 1023) std::printf("checkpoint @ %d\n", int(n));
			else std::printf("checkpoint @ %dK\n", int(n) >> 10);
			/*if (test_size < 10ull << 20) std::printf("for length = %d KB\n", test_size >> 10);
			else if (test_size < 10ull << 30) std::printf("for length = %d MB\n", test_size >> 20);
			else if (test_size < 10ull << 40) std::printf("for length = %d GB\n", test_size >> 30);
			else std::printf("for length = %d TB\n", test_size >> 40);*/
			std::printf("==================================================\n");
			int test_name_index = 0;
			for (std::map<std::string,std::map<Uint64,SampleSet> >::iterator it = data.begin(); it != data.end(); it++, test_name_index++) {
				std::string name = it->first;
				for (std::map<Uint64,SampleSet>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
					Uint64 length = it2->first;
					//int length_L2 = it2->first;
					SampleSet &ss = it2->second;
					ss._normalize();
					//std::printf("//mean= %f; median= %f; stddev= %f;\n", ss.get_mean(), ss.get_result_by_percentile(0.50), ss.get_stddev());
					std::printf("  {\"%s\",%9.0f, %d,%4d, {", name.c_str(), double(length), (int)ss.rs.size(), (int)ss.num_duplicates());
					/*double p[] = { 
						0.00001, 0.00002, 0.00005, 0.0001, 0.0002, 0.0005, 0.001, 0.002, 0.005, -1, 
						0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10, -1, 
						0.11, 0.12, 0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.19, 0.20, -1, 
						0.21, 0.22, 0.23, 0.24, 0.25, 0.26, 0.27, 0.28, 0.29, 0.30, -1, 
						0.31, 0.32, 0.33, 0.34, 0.35, 0.36, 0.37, 0.38, 0.39, 0.40, -1, 
						0.41, 0.42, 0.43, 0.44, 0.45, 0.46, 0.47, 0.48, 0.49, 0.50, -1, 
						0.51, 0.52, 0.53, 0.54, 0.55, 0.56, 0.57, 0.58, 0.59, 0.60, -1, 
						0.61, 0.62, 0.63, 0.64, 0.65, 0.66, 0.67, 0.68, 0.69, 0.70, -1, 
						0.71, 0.72, 0.73, 0.74, 0.75, 0.76, 0.77, 0.78, 0.79, 0.80, -1, 
						0.81, 0.82, 0.83, 0.84, 0.85, 0.86, 0.87, 0.88, 0.89, 0.90, -1, 
						0.91, 0.92, 0.93, 0.94, 0.95, 0.96,	0.97, 0.98, 0.99, -1, 
						0.995, 0.998, 0.999, 0.9995, 0.9998, 0.9999, 0.99995, 0.99998, 0.99999, -2
					};*/
					double *p = ref_p129_with_formatting;
					for (int i = 0; p[i] != -2; i++) {
						if (p[i] == -1) continue;
						if (i) std::printf(",");
						if (p[i] >= 0.01 && p[i] <= 0.99) std::printf("%+7.3f", ss.get_result_by_percentile(p[i]));
						else std::printf("%+10.5f", ss.get_result_by_percentile(p[i]));
					}
					std::printf("}, %+7.3f, %+7.3f, %7.3f, %d},\n", ss.get_result_by_percentile(0.50), ss.get_mean(), ss.get_stddev(), 0);
					//std::printf("}},\n");

					/*for (int L2 = 0; (2ull << L2) <= ss.rs.size(); L2++) {
						std::printf("//bott.L2:%2d:%+8.3f      ", L2, ss.get_result_by_index( (1ull << L2)-1 ));
						std::printf("topL2:%2d : %+8.3f\n",      L2, ss.get_result_by_index(ss.rs.size() - (1ull << L2)));
					}
					for (int L10 = 1; std::pow(10.0, double(L10)) <= ss.rs.size(); L10++) {
						double p = std::pow(0.1, double(L10));
						std::printf("//%f: %+8.3f      ", p, ss.get_result_by_percentile( p ));
						std::printf("%f: %+8.3f\n",   1-p, ss.get_result_by_percentile( 1-p ));
					}*/
				}
			}
		}
		Uint64 blocks_so_far = 0;
		for (int length_L2 = 25; length_L2 <= 32; length_L2 += 1) {
			if (length_L2 >= 10+3 && length_L2 < 99) {
				Uint64 new_blocks = (5ull << (length_L2-3)) / Tests::TestBlock::SIZE;
				tman.test(new_blocks - blocks_so_far);
				blocks_so_far = new_blocks;
				for (int i = 0; i < tests.tests.size(); i++) {
					std::vector<PractRand::TestResult> results;
					tests.tests[i]->get_results(results);
					for (int j = 0; j < results.size(); j++) data[results[j].name][new_blocks]._add(results[j].get_raw());
				}
			}
			if (length_L2 >= 10+2 && length_L2 <= 99) {
				Uint64 new_blocks = (3ull << (length_L2-2)) / Tests::TestBlock::SIZE;
				tman.test(new_blocks - blocks_so_far);
				blocks_so_far = new_blocks;
				for (int i = 0; i < tests.tests.size(); i++) {
					std::vector<PractRand::TestResult> results;
					tests.tests[i]->get_results(results);
					for (int j = 0; j < results.size(); j++) data[results[j].name][new_blocks]._add(results[j].get_raw());
				}
			}
			if (length_L2 >= 10+3 && length_L2 < 99) {
				Uint64 new_blocks = (7ull << (length_L2-3)) / Tests::TestBlock::SIZE;
				tman.test(new_blocks - blocks_so_far);
				blocks_so_far = new_blocks;
				for (int i = 0; i < tests.tests.size(); i++) {
					std::vector<PractRand::TestResult> results;
					tests.tests[i]->get_results(results);
					for (int j = 0; j < results.size(); j++) data[results[j].name][new_blocks]._add(results[j].get_raw());
				}
			}//*/
			if (true) {
				Uint64 new_blocks = (1ull << length_L2) / Tests::TestBlock::SIZE;
				tman.test(new_blocks - blocks_so_far);
				blocks_so_far = new_blocks;
				for (int i = 0; i < tests.tests.size(); i++) {
					std::vector<PractRand::TestResult> results;
					tests.tests[i]->get_results(results);
					for (int j = 0; j < results.size(); j++) data[results[j].name][new_blocks]._add(results[j].get_raw());
				}
			}
		}
		/*for (int i = 0; i < tests.tests.size(); i++) {
			std::vector<PractRand::TestResult> results;
			tests.tests[i]->get_results(results);
			for (int j = 0; j < results.size(); j++) data[results[j].name]._add(results[j].value);
		}*/
		tman.reset();
	}
}

static void calibrate_set_uniformity(SampleSet *calib, int n, PractRand::RNGs::vRNG *known_good) {
	for (int i = 0; i < 1ull<<20; i++) {
		SampleSet tmp;
		for (int j = 0; j < n; j++) tmp._add(known_good->randlf());
		tmp._normalize();
		calib->_add(Tests::test_uniformity(tmp));
	}
	calib->_normalize();
}
static void simple_chisquare_test( PractRand::RNGs::vRNG *known_good ) {
	enum {SIZE = 1<<4};
	Uint64 counts[SIZE];
	double probs[SIZE];
	SampleSet ssA, ssB;
	enum {N = 8};
	for (int x = 0; x < SIZE; x++) probs[x] = 1.0 / SIZE;
	for (int i = 0; i < N; i++) {
		for (int x = 0; x < SIZE; x++) counts[x] = 0;
		for (int x = 0; x < SIZE*100; x++) counts[known_good->randi(SIZE)]++;
		double r = PractRand::Tests::g_test(SIZE, &probs[0], &counts[0]);
		double p = Tests::math_chisquared_to_pvalue(r, SIZE-1);
		double r2 = Tests::math_chisquared_to_normal(r, SIZE-1);
		double p2 = Tests::math_normaldist_to_pvalue(r2);
		ssA._add(p);
		ssB._add(p2);
	}
	ssA._normalize();
	ssB._normalize();
	SampleSet calib;
	calibrate_set_uniformity(&calib, N, known_good);
	std::printf("simple_chisquare_test: %.4f  %.4f\n", calib.get_percentile(Tests::test_uniformity(ssA)), calib.get_percentile(Tests::test_uniformity(ssB)));
}
void verify_test_distributions() {
	std::time_t start_time = std::time(NULL);
	std::clock_t start_clock = std::clock();

	PractRand::RNGs::Polymorphic::hc256 known_good(PractRand::SEED_AUTO);
	PractRand::RNGs::Polymorphic::efiix32x48 rng(PractRand::SEED_AUTO);

	//simple_chisquare_test(&known_good);

	//Tests::ListOfTests tests = Tests::Batteries::get_core_tests();
	//Tests::ListOfTests tests = Tests::Batteries::get_expanded_core_tests();
	Tests::ListOfTests tests(new Tests::FPF(4, 14, 6));
	//Tests::ListOfTests tests(new Tests::BRank(18));
	//Tests::ListOfTests tests(new Tests::DistC6(9,0, 1,0,0));
	//Tests::ListOfTests tests(new Tests::DistC6(6,1, 1,0,0));
	//Tests::ListOfTests tests(new Tests::DistC6(5,2, 1,0,0));
	//Tests::ListOfTests tests(new Tests::DistC6(4,3, 0,0,1));
	//Tests::ListOfTests tests(new Tests::DistC6(5,3, 1,0,1));
	//Tests::ListOfTests tests(new Tests::Gap16());
	TestManager tman(&tests, &known_good);
	tman.reset(&rng);

	std::map<std::string,std::map<int,SampleSet> > data;
	Uint64 next_checkpoint = 1;
	for (Uint64 n = 0; n <= 1<<20; n++) {
		if (n == next_checkpoint) {
			SampleSet calib;
			calibrate_set_uniformity(&calib, n, &known_good);
			enum {CHUNKY = 1 << 8};
			if (next_checkpoint < CHUNKY) next_checkpoint <<= 1; else next_checkpoint += CHUNKY;
			std::printf("\n\n\n\n");
			std::printf("==================================================\n");
			std::printf("checkpoint @ %d\n", int(n) );
			/*if (test_size < 10ull << 20) std::printf("for length = %d KB\n", test_size >> 10);
			else if (test_size < 10ull << 30) std::printf("for length = %d MB\n", test_size >> 20);
			else if (test_size < 10ull << 40) std::printf("for length = %d GB\n", test_size >> 30);
			else std::printf("for length = %d TB\n", test_size >> 40);*/
			std::printf("==================================================\n");
			int test_name_index = 0;
			for (std::map<std::string,std::map<int,SampleSet> >::iterator it = data.begin(); it != data.end(); it++, test_name_index++) {
				std::string name = it->first;
				for (std::map<int,SampleSet>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
					int length_L2 = it2->first;
					SampleSet &ss = it2->second;
					ss._normalize();
					//if (ss.num_duplicates()) continue;
					std::printf("\n\n name=\"%s\"; length_L2=%d;\n", name.c_str(), length_L2);
					std::printf("total= %d; duplicates= %d;\n", ss.rs.size(), ss.num_duplicates());
					if (!ss.num_duplicates()) {
						double sum = Tests::test_uniformity(ss);
						double p = calib.get_percentile(sum);
						std::printf("blah = %f of (%f:%f:%f)\n", sum, calib.get_result_by_percentile(.05), calib.get_result_by_percentile(.5), calib.get_result_by_percentile(.95));
						std::printf("p = %f\n", p);
						if (fabs(p-.5) > 0.499) std::printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
					}
					std::printf("\n");
				}
			}
		}
		Uint64 blocks_so_far = 0;
		for (int length_L2 = 21; length_L2 <= 30; length_L2 += 1) {
			//data["fred"][length_L2]._add(known_good.randf(0.95));continue;
			Uint64 new_blocks = (1ull << length_L2) / Tests::TestBlock::SIZE;
			tman.test(new_blocks - blocks_so_far);
			blocks_so_far = new_blocks;
			for (int i = 0; i < tests.tests.size(); i++) {
				std::vector<PractRand::TestResult> results;
				tests.tests[i]->get_results(results);
				for (int j = 0; j < results.size(); j++) data[results[j].name][length_L2]._add(results[j].get_pvalue());
			}
		}
		tman.reset();
	}
}

struct Data {
	Uint64 count;
	double vec[129];
	Data() : count(0) {
		for (int i = 0; i < 129; i++) vec[i] = 0;
	}
};
std::map<std::string,std::map<int, Data > > data;

void print_data() {
	for (std::map<std::string,std::map<int,Data > >::iterator it = data.begin(); it != data.end(); it++) {
		std::string name = it->first;
		for (std::map<int,Data >::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
			int length_L2 = it2->first;
			const Data &d = it2->second;
			std::printf("{ \"%s\", 1ull << (%d - 10), %.0f, {", name.c_str(), length_L2, (double)d.count );
			for (int i = 0; i < 129; i++) {
				if (ref_p129[i] >= 0.01 && ref_p129[i] <= 0.99) std::printf("%+7.3f", d.vec[i]);
				else std::printf("%+10.5f", d.vec[i]);
				if (i != 128) std::printf(",");
			}
			std::printf("}},\n");
		}
	}
}

static int sum_of_bytes(Uint32 in) {
	const Uint32 mask1 = 0x00FF00FF;
	in = (in & mask1) + ((in>>8) & mask1);
	in += in >> 16;
	return (in & 1023);
}
double gaussian_PDF(double x) {return std::exp( (x)*(x)*-0.5 ) * 0.3989422804014326779399460652221;}
double denormalized_gaussian_PDF(double x) {return std::exp( (x)*(x)*-0.5 );}//does not seem to help any
double calc_ziggurat_final_area(double x) {return x * gaussian_PDF(x) + Tests::math_normaldist_to_pvalue(-x);}
double calc_ziggurat_area(double left_x, double right_x) {
	double upper_y = gaussian_PDF(left_x);
	double lower_y = gaussian_PDF(right_x);
	return (upper_y - lower_y) * right_x;
}
double find_ziggurat_point(double old_x, double target_area) {
	double area = 0;
	double x = old_x;
	double slope = 1;
	for (int i = 0; i < 500; i++) {
		double delta_x = (target_area - area) * slope * 0.875;
		double new_x = x + delta_x;
		double new_area = calc_ziggurat_area(old_x, new_x);
		double delta_area = new_area - area;
		if (new_area > target_area) {
			slope *= 0.5;
			continue;
		}
		if (!delta_area) return x;
		slope = delta_x / delta_area;
		if (slope > 1024) {slope = 1024; if (!x) slope = 1;}
		area = new_area;
		x = new_x;
	}
	return x;
}

	enum {TABLE_SIZE=1<<7};
	static float table_y[TABLE_SIZE-1];
	static float table_x[TABLE_SIZE-1];
	static double tail_scale, tail_p, final_stripe_area;
template<class RNG>
double generate_gaussian( RNG &rng ) {
	//*/
	// 
	// floats(17+4):   3: 26, 6: 29, 8: 32
	// doubles(17+4):         6: 30, 8: 32
	// floats(17+8):   3: 29
	// doubles(17+8):  
	// floats(17+4,lf): 3: >33
	// floats(17+8,lf): 
	static bool inited = false;
	if (!inited) {
		double a = 0.5 / TABLE_SIZE;
		//generate initial guesses:
		{
			double ox = 0;
			for (int i = 0; i < TABLE_SIZE-1; i++) {
				ox = table_x[i] = find_ziggurat_point(ox, a);
				table_y[i] = gaussian_PDF(table_x[i]);
			}
		}
		final_stripe_area = calc_ziggurat_final_area(table_x[TABLE_SIZE-2]);
		//refine guesses:
		for (int i = 0; i < 200; i++) {
			a += (final_stripe_area - a) / TABLE_SIZE / 1;
			double ox = 0;
			for (int i = 0; i < TABLE_SIZE-1; i++) {ox = table_x[i] = find_ziggurat_point(ox, a); table_y[i] = gaussian_PDF(table_x[i]);}
			final_stripe_area = calc_ziggurat_final_area(table_x[TABLE_SIZE-2]);
		}

		//precalc a few extra details
		tail_p = Tests::math_normaldist_to_pvalue(-table_x[TABLE_SIZE-2]);
		tail_scale = table_x[TABLE_SIZE-2] / (table_x[TABLE_SIZE-2] * table_y[TABLE_SIZE-2] / final_stripe_area);
		//for (int i = 0; i < TABLE_SIZE-1; i++) table_y[i] /= 0.39894228040143270;
		inited = true;
		std::printf("table_x = {"); for (int i = 0; i < TABLE_SIZE-1; i++) std::printf("%f, ", table_x[i]); std::printf("}\n"); 
		std::printf("table_y = {"); for (int i = 0; i < TABLE_SIZE-1; i++) std::printf("%f, ", table_y[i]); std::printf("}\n"); 
		std::printf("tail_p = %f; tail_scale = %f;\n", tail_p, tail_scale); 
	}
	while (true) {
		unsigned long stripe = rng.raw16() & (TABLE_SIZE-1);
		if (stripe < TABLE_SIZE-2) {//middle stripes
			double x = rng.randf(-1,1) * table_x[stripe+1];
			if (std::fabs(x) < table_x[stripe]) return x;
			double y = rng.randf(table_y[stripe], table_y[stripe+1]);
			if (y < gaussian_PDF(x)) return x;
		}
		else if (stripe == TABLE_SIZE-2) {//top stripe
			double x = rng.randf(-1,1) * table_x[0];
			double base_y = 0.39894228040143270;
			double y = rng.randf(table_y[0], base_y);
			if (y < gaussian_PDF(x)) return x;
		}
		else if (stripe == TABLE_SIZE-1) {//bottom stripe
			double x = rng.randf(-1, 1) * tail_scale;
			if (std::fabs(x) < table_x[TABLE_SIZE-2]) return x;
			double p = rng.randf() * tail_p; 
			double n = Tests::math_pvalue_to_normaldist(p);
			if (x < 0) return n;
			else return -n;
		}
		double p = rng.randlf();
		return Tests::math_pvalue_to_normaldist(p);
	}
	return 0;//*/

	/*
	//17+0: 
	//17+4: 
	//17+8: 33+
	double a, b, r;
	do {
		a = Sint32(rng.raw32()) / 2147483648.0;//rng.randf()*2-1;
		b = Sint32(rng.raw32()) / 2147483648.0;//rng.randf()*2-1;
		r = a*a + b*b;
	}
	while (r > 1.0);
	double prod = std::sqrt(-2 * std::log(r) / r);
	return a * prod;
//	return b * prod;//*/

/*	int s = sum_of_bytes(rng.raw32()) + sum_of_bytes(rng.raw32()) + 
		sum_of_bytes(rng.raw32()) + sum_of_bytes(rng.raw32());
	return (s - 2040) / 295.6;//*/

	/*enum {NORMTABLE_SIZE=1<<9};
	// 4: 17, 5: 19, 6: 21, 7: 22-23, 8: 24-25, 9: 26-27
	static float normtable_a[NORMTABLE_SIZE];
	static float normtable_b[NORMTABLE_SIZE];
	static bool inited = false;
	const double edge_cases = 0.1;
	if (!inited) {
		for (int i = 0; i < NORMTABLE_SIZE; i++) {
			double f1, f2;
			f1 = (i + 0.0) / NORMTABLE_SIZE;
			f2 = (i + 1.0) / NORMTABLE_SIZE;
			if (!i) f1 = edge_cases / NORMTABLE_SIZE;
			else if (i == NORMTABLE_SIZE-1) f2 = 1 - edge_cases / NORMTABLE_SIZE;
			double v1 = math_pvalue_to_normaldist(f1);
			double v2 = math_pvalue_to_normaldist(f2);
			normtable_a[i] = v1;
			normtable_b[i] = (v2 - v1) / 4294967296.0;
		}
		inited = true;
	}
	int i = rng.raw16() & (NORMTABLE_SIZE-1);
	return normtable_a[i] + normtable_b[i] * rng.raw32();//*/

	
	/*enum {NORMTABLE_SIZE=1<<10};
	// 3: 17, 4: 21, 5: 23, 6: 25, 7: 26, 8: 27-28
	static float normtable_a[NORMTABLE_SIZE-1];
	static float normtable_b[NORMTABLE_SIZE-1];
	static float normtable_c[NORMTABLE_SIZE];
	static float normtable_d[NORMTABLE_SIZE];
	static bool inited = false;
	const double edge_cases = 0.1;
	if (!inited) {
		for (int i = 0; i < NORMTABLE_SIZE-1; i++) {
			double f1, f2;
			f1 = (i + 0.5) / NORMTABLE_SIZE;
			f2 = (i + 1.5) / NORMTABLE_SIZE;
			double v1 = math_pvalue_to_normaldist(f1);
			double v2 = math_pvalue_to_normaldist(f2);
			normtable_a[i] = v1;
			normtable_b[i] = (v2 - v1) / 4294967296.0;
		}
		for (int i = 0; i < NORMTABLE_SIZE; i++) {
			double f1, f2;
			f1 = (i + 0.0) / NORMTABLE_SIZE / NORMTABLE_SIZE / 2;
			f2 = (i + 1.0) / NORMTABLE_SIZE / NORMTABLE_SIZE / 2;
			if (!i) f1 = edge_cases / NORMTABLE_SIZE;
			double v1 = math_pvalue_to_normaldist(f1);
			double v2 = math_pvalue_to_normaldist(f2);
			normtable_c[i] = v1;
			normtable_d[i] = (v2 - v1) / 4294967296.0;
		}
		inited = true;
	}
	int i = rng.raw16() & (NORMTABLE_SIZE-1);
	if (i) return normtable_a[i-1] + normtable_b[i-1] * rng.raw32();
	i = rng.raw16();
	double rv = normtable_c[i&(NORMTABLE_SIZE-1)] + normtable_d[i&(NORMTABLE_SIZE-1)] * rng.raw32();
	if (i & 32768) rv = -rv;
	return rv;//*/

	
	/*enum {NORMTABLE_SIZE_L2=7};
	enum {STRONG=1
		,  WEAK =3
	};
	// test A
	// Strong+Weak, Standards    3      4      5      6      7      8      9      10
	// 1+0          full         15     17     19     21     22.5   24.5   26.5   ?
	// 1+1          full         20     23.5   27     ?
	// 2+0          full         24     28     ?
	// 1+0          low          13     14     16     18     20     21     24     25
	// 1+1          low          17     21     25     28?
	// 2+0          low          22     25     28?    ?
	// 1+2          low          21     25     29?
	// test B
	// Strong+Weak, Stds -0/4/8   3          4          5          6          7          8          9         10
	// 1+0          full          ?          ?          ?          ?          ?          ?/16-      ?
	// 1+1          full          ?          ?          ?          ?          ?          ?/24       ?
	// 2+0          full          17         20         23         25         27         ?/26       ?
	// 1+2          full          18/16-     22/16-     24/16-     26/19      28+2/22    ?/25       31/30     ?/31
	// 1+3          full          20/16-     24/16-     25/18      27+1/22    31/28      32/30      ?
	// 1+4          full          ?/17       ?/21       ?/24       ?/26       ?/29       ?/31       ?
	// 1+7          full          ?/16-      ?/18       ?/22       ?/25       ?/27       ?          ?
	// 2+2          full          21         23         25         27         ?          ?          ?
	// 4+0          full          21         23         25         ?          ?          ?          ?
	// 0+4          full          ?          ?          ?          ?          ?          ?          ?
	// 1+8          full          23         25         26         27         ?          ?          ?
	// 8+0          full          ?          ?          25+1       ?          ?          ?          ?
	// ?+?          full          ?          ?          ?          ?          ?          ?          ?
	enum {NORMTABLE_SIZE=1<<NORMTABLE_SIZE_L2};
	static double normtable_a[NORMTABLE_SIZE];
	static double normtable_b[NORMTABLE_SIZE];
	static double normtable_c[NORMTABLE_SIZE];
	static bool inited = false;
	if (!inited) {
		const double edge_cases = 3. / 32 / NORMTABLE_SIZE;
		const double scale = std::sqrt(1.0 / double(STRONG+WEAK));
		for (int i = 0; i < NORMTABLE_SIZE; i++) {
			double f1, f2;
			f1 = (i + 0.0) / NORMTABLE_SIZE;
			f2 = (i + 1.0) / NORMTABLE_SIZE;
			if (f1 < edge_cases) f1 = edge_cases;
			if (f2 > 1-edge_cases) f2 = 1 - edge_cases;
			double v1 = math_pvalue_to_normaldist(f1) * scale;
			double v2 = math_pvalue_to_normaldist(f2) * scale;
			normtable_a[i] = (v1+v2)/2;
			normtable_b[i] = (v2 - v1) / 4294967296.0;
			normtable_c[i] = normtable_a[i];
		}

		if (false) {
			double sum = 0, sum2 = 0;
			int n = 1<<28;
			for (int i = 0; i < n; i++) {
				double norm = normtable_a[i & (NORMTABLE_SIZE-1)];
				norm += normtable_b[i & (NORMTABLE_SIZE-1)] * Sint32(rng.raw32());
				sum += norm; sum2 += norm * norm;
			}
			sum /= n; sum2 /= n;
			double dev = std::sqrt(sum2 - sum*sum);
			for (int i = 0; i < NORMTABLE_SIZE; i++) {normtable_a[i] /= dev/scale; normtable_b[i] /= dev/scale;}
		}
		if (false) {
			double sum = 0, sum2 = 0;
			int n = 1<<28;
			for (int i = 0; i < n; i++) {
				double norm = normtable_c[i & (NORMTABLE_SIZE-1)];
				sum += norm; sum2 += norm * norm;
			}
			sum /= n; sum2 /= n;
			double dev = std::sqrt(sum2 - sum*sum);
			for (int i = 0; i < NORMTABLE_SIZE; i++) normtable_c[i] /= dev/scale;
		}
		if (true) {
			double sum = 0, sum2 = 0;
			int n = 1<<28;
			for (int i = 0; i < n; i++) {
				double norm = 0;
				for (int a = 0; a < STRONG; a++) {int index = rng.raw32(); norm += normtable_a[index & (NORMTABLE_SIZE-1)] + normtable_b[index & (NORMTABLE_SIZE-1)] * Sint32(rng.raw32());}
				for (int a = 0; a < WEAK; a++) {int index = rng.raw32(); norm += normtable_a[index & (NORMTABLE_SIZE-1)];}
				sum += norm; sum2 += norm * norm;
			}
			sum /= n; sum2 /= n;
			double dev = std::sqrt(sum2 - sum*sum);
			for (int i = 0; i < NORMTABLE_SIZE; i++) normtable_a[i] /= dev;
			for (int i = 0; i < NORMTABLE_SIZE; i++) normtable_b[i] /= dev;
		}

		//for (int i = 0; i < NORMTABLE_SIZE; i++) std::printf("%s%+.11g, ", (i%16)?"":"\n", normtable_b[i]);
		inited = true;
	}
	Uint32 indeces = rng.raw32();
	int index; Sint32 u; double rv = 0;
	for (int x = 0; x < STRONG; x++) {
		index = indeces & (NORMTABLE_SIZE-1); indeces >>= NORMTABLE_SIZE_L2;
		//index = rng.raw16() & (NORMTABLE_SIZE-1);
		rv += normtable_a[index];
		u = rng.raw32(); rv += normtable_b[index] * u;
	}
	if ((STRONG+WEAK) * NORMTABLE_SIZE_L2 > 32) indeces = rng.raw32();
	for (int x = 0; x < WEAK; x++) {
		index = indeces & (NORMTABLE_SIZE-1); indeces >>= NORMTABLE_SIZE_L2;
		//index = rng.raw16() & (NORMTABLE_SIZE-1);
		rv += normtable_a[index];
	}
	return rv;//*/

}
double generate_gaussian_( PractRand::RNGs::vRNG *rng ) {return generate_gaussian(*rng);}
void test_normal_distribution_a() {
	PractRand::RNGs::LightWeight::sfc32 rng( PractRand::SEED_AUTO );
	//PractRand::RNGs::Polymorphic::sfc32 rng( PractRand::SEED_AUTO );
	generate_gaussian(rng);
	std::clock_t bench_start, bench_end;
	bench_start = std::clock();
	while (bench_start == (bench_end = std::clock())) ;
	bench_start = bench_end;
	Uint32 count = 0;
	while (CLOCKS_PER_SEC*0.5+1 > std::clock_t((bench_end = std::clock())-bench_start)) {
		double a = generate_gaussian(rng) + generate_gaussian(rng) + generate_gaussian(rng) + generate_gaussian(rng) + generate_gaussian(rng) + generate_gaussian(rng) + generate_gaussian(rng) + generate_gaussian(rng);
		//generate_gaussian_(&rng); generate_gaussian_(&rng); generate_gaussian_(&rng); generate_gaussian_(&rng);
		count += a != -123456789;
	}
	double rate = count*8.0 / (double(std::clock_t(bench_end-bench_start))/CLOCKS_PER_SEC);
	std::printf("gaussian speed: %.3f M / second\n", rate / 1000000.0 );
	if (false) {
		SampleSet ss; ss.rs.reserve(1<<27);
		for (int n = 0; n <= 27; n++) {
			for (int i = (1<<n) - ss.size(); i > 0; i--) ss._add(generate_gaussian(rng));
			ss._normalize();
			printf("test_normal_distribution_a0 : 2^%02d: mean:%+.5f, stddev:%.5f\n", n, ss.get_mean(), ss.get_stddev());
		}
	}
	if (false) {
		SampleSet ss; ss.rs.reserve(1<<27);
		for (int n = 0; n <= 27; n++) {
			for (int i = (1<<n) - ss.size(); i > 0; i--) ss._add(Tests::math_normaldist_to_pvalue(generate_gaussian(rng)));
			ss._normalize();
			printf("test_normal_distribution_a1 : 2^%02d: %+f\n", n, Tests::test_uniformity(ss));
		}
	}
	if (true) {
		enum {TBITS=17};
		std::vector<Uint64> counts; counts.resize(1<<TBITS, 0);
		Uint64 total = 0;
		enum {DISCARD_BITS = 4};
		const double scale = std::pow(2.0, TBITS*1.0+DISCARD_BITS);
		const double p_thresh = std::pow(0.5, DISCARD_BITS*1.0);
		double n_thresh = 999999999999999.0;
		if (p_thresh < 1) n_thresh = Tests::math_pvalue_to_normaldist(p_thresh);
		for (int n = 16; n <= 40; n++) {
			while (!(total >> n)) {
				for (long i = 0; i < 1<<16; ) {
					double norm = generate_gaussian(rng);
					if (norm >= n_thresh) continue;
					double p = Tests::math_normaldist_to_pvalue(norm);
					Uint32 index = Uint32(std::floor(p*scale));
					if (index > (1 << TBITS)) PractRand::issue_error();
					counts[index]++;
					i++;
				}
				total += 1<<16;
			}
			printf("test_normal_distribution_a2 : 2^%02d: %+f\n", n, Tests::g_test_flat_merge_normal(1<<TBITS, &counts[0], 1ull<<n) );
		}
	}
}

Uint64 count_period(PractRand::RNGs::vRNG *rng) {
	enum {BYTES = 32};//must be a power of 2 greater than or equal to 8
	if (rng->get_native_output_size() == 8) {
		typedef Uint8 Word;
		enum { BUFSIZE = BYTES / sizeof(Word) };
		Word buff1[BUFSIZE];
		Word buff2[BUFSIZE];
		rng->autoseed();
		for (int i = 0; i < BUFSIZE; i++) buff1[i] = rng->raw8();
		for (Uint64 p = 0; p < (1ull << 48); p++) {
			if ((buff2[p & (BUFSIZE - 1)] = rng->raw8()) == buff1[BUFSIZE - 1]) {
				bool match = true;
				for (int i = 1; i < BUFSIZE; i++) if (buff2[(p - i) & (BUFSIZE - 1)] != buff1[BUFSIZE - 1 - i]) match = false;
				if (match) {
					Uint64 rv = p + 1;
					std::printf("cycle found in [%s] of length %.0f\n", rng->get_name().c_str(), double(rv));
					if (!(rv & ((1 << 20)-1))) std::printf("%.0fM exactly\n", double(rv >> 20));
					else if (!(rv & 1023)) std::printf("%.0fK exactly\n", double(rv >> 10));
					return rv;
				}
			}
		}
	}
	return -1;
}

void test_sfc16() {
	if (1) {//search for short cycles
		enum {
			BUFFER_SIZE = 16,//plenty to tell if the state matched (6 would probably be enough, but more doesn't hurt)
			SHORT_CYCLE_L2 = 40,//the log-based-2 of the cycle length we consider "too short"
		};
		Uint16 buffy[BUFFER_SIZE];
		PractRand::RNGs::Raw::sfc16 rng;
		for (Uint64 seed = 0; true; seed++) {
			rng.seed(seed);
			for (int i = 0; i < BUFFER_SIZE; i++) buffy[i] = rng.raw16();
			for (Uint64 i = 0; i < ((1ull << SHORT_CYCLE_L2) - BUFFER_SIZE); i++) rng.raw16();
			int match = true;
			for (int i = 0; i < BUFFER_SIZE; i++) if (buffy[i] != rng.raw16()) match = false;
			if (match || !(seed & 15)) {
				std::printf("seed 0x");
				if (seed >> 32) {
					std::printf("%X", Uint32(seed >> 32));
					std::printf("%08X", Uint32(seed >> 0));
				}
				else std::printf("%X", Uint32(seed));
				if (match) std::printf(" had a short cycle\n\n  !!!!!!!!!!!!!!!!!!!\n\n  !!!!!!!!!!!!!!!!!!!\n\n  !!!!!!!!!!!!!!!!!!!\n");
				else std::printf(" was good\n");
			}
		}

	}
	if (0) {
		long double p248 = std::pow(2, 48);
		long double base = std::pow(0.5, 48);
		long double base_inv = 1 - base;
		long double after2_16 = 1;
		for (int i = 1; i < 65536; i++) {
			after2_16 *= 1 - (1 / (p248 - i));
		}
		std::printf("chance of a seed leading to a cycle < 2**32: %g\n", 1 / (1 - after2_16));
		long double after2_24 = 1;
		for (int i = 1; i < 65536 * 256; i++) {
			after2_24 *= 1 - (1 / (p248 - i));
		}
		std::printf("chance of a seed leading to a cycle < 2**40: %g\n", 1 / (1 - after2_24));
		long double after2_32 = 1;
		for (unsigned long i = 1; i < 65536 * 65536ul; i++) {
			after2_32 *= 1 - (1 / (p248 - i));
		}
		std::printf("chance of a seed leading to a cycle < 2**48: %f\n", 1 / (1 - after2_32));
	}
}


int main(int argc, char **argv) {
	PractRand::initialize_PractRand();
	PractRand::self_test_PractRand();

	//test_sfc16();
	//blah_bcfn();
	//blah_fpf_all2();
	//blah_fpf();
	find_test_distributions();
	//count_period(new PractRand::RNGs::Polymorphic::NotRecommended::xlcg8of64_varqual(28));
	//verify_test_distributions();
	//test_normal_distribution_a();
	//print_data();

	/*	
	for (int i = 0; i < 117; i++) {
		std::printf(" %5f : %5f %5f %5f %5f %5f %5f\n", ref_p[i], Tests::math_pvalue_to_chisquared(ref_p[i], 1), 
			Tests::math_pvalue_to_chisquared(ref_p[i], 2), 
			Tests::math_pvalue_to_chisquared(ref_p[i], 3), 
			Tests::math_pvalue_to_chisquared(ref_p[i], 4),
			Tests::math_pvalue_to_chisquared(ref_p[i], 5),
			Tests::math_pvalue_to_chisquared(ref_p[i], 6)
		);
	}//*/


	return 0;
}




