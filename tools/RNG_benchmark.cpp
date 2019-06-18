#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <string>
#include <sstream>
//#include <map>
//#include <vector>
//#include <list>
//#include <map>


//master header, includes everything in PractRand for both 
//  practical usage and research... 
//  EXCEPT it does not include specific algorithms
#include "PractRand.h"


/*
	Take a bunch of RNG algorithms, measure how fast they are.  
	The RNGs include all the PractRand recommended RNGs plus a few candidates 
		implemented here.  The non-recommended RNGs from the "other" directory are 
		not included, as they are implemented in a fashion that is not intended for 
		high performance, only for easy testing.  
*/


#include "PractRand/RNGs/all.h"
//#include "PractRand/RNGs/rarns16.h"
//#include "PractRand/RNGs/rarns32.h"
//#include "PractRand/RNGs/rarns64.h"

using namespace PractRand;
#include "Candidate_RNGs.h"
#include "measure_RNG_performance.h"


template<typename RNG> 
double benchmark_seeding(/*PractRand::RNGs::vRNG *rng*/) {
	//no real point to benchmarking non-polymorphic RNGs - seeding is sufficiently slow that the overhead will be insignificant
	RNG _rng(PractRand::SEED_AUTO);
	PractRand::RNGs::vRNG *rng = &_rng;
	enum {NUM_CLOCKS_TO_TEST = int(CLOCKS_PER_SEC * 0.1 + 0.5)};
	PractRand::RNGs::LightWeight::sfc64 known_fast(13);
	int clock0 = clock();
	int clock1, clock2;
	while ((clock1 = clock()) == clock0) ;

	Uint64 sum = 0;
	int j = 0;
	do {
		rng->seed(known_fast.raw64());
		sum += rng->raw32();
		j++;
	} while ((clock2=clock())-clock1 < NUM_CLOCKS_TO_TEST);

	double delta = (clock2 - clock1) / double(CLOCKS_PER_SEC);//seconds
	double amount = j * 1;//seedings
	double rate = amount / delta;

	//just to make very sure that some smart compiler won't optimize everything away:
	if (sum == 0) std::printf("unlikely!");

	return rate;
}

void benchmark_RNG_speeds() {
//#define PERF(RNG) printf("  %5.3f GB/s  :  %5.3f GB/s  :  %s\n", measure_RNG_performance< PractRand::RNGs::LightWeight:: RNG >()/1024, measure_RNG_performance<PractRand::RNGs::Polymorphic:: RNG >()/1024, #RNG );
#define PERF(RNG) printf("  %5.3f GB/s  :  %5.3f GB/s  :%7.0f KHz  :  %s\n", measure_RNG_performance< PractRand::RNGs::LightWeight:: RNG >()/1024, measure_RNG_performance<PractRand::RNGs::Polymorphic:: RNG >()/1024, benchmark_seeding<PractRand::RNGs::Polymorphic:: RNG >()/1000, #RNG );
//#define PERF_POLYMORPHIC_ONLY(RNG) printf("  ----- GB/s  :  %5.3f GB/s  :  %s\n", measure_RNG_performance<PractRand::RNGs::Polymorphic:: RNG >()/1024, #RNG );
#define PERF_POLYMORPHIC_ONLY(RNG) printf("  ----- GB/s  :  %5.3f GB/s  :%7.0f KHz  :  %s\n", measure_RNG_performance<PractRand::RNGs::Polymorphic:: RNG >()/1024, benchmark_seeding<PractRand::RNGs::Polymorphic:: RNG >()/1000, #RNG );
#define PERF_CANIDATE(rng) { typedef Candidates:: raw_ ## rng RawRNG;typedef Candidates:: polymorphic_ ## rng PolymorphicRNG; const char *name = #rng ; printf("  %5.3f GB/s  :  %5.3f GB/s  :%7.0f KHz  :  %s\n", measure_RNG_performance< PractRand::RNGs::Adaptors::RAW_TO_LIGHT_WEIGHT_RNG< RawRNG > >()/1024, measure_RNG_performance<PolymorphicRNG>()/1024, benchmark_seeding<PolymorphicRNG>()/1000, name ); }
#define PERF_CHACHA(RNG,ROUNDS) { PractRand::RNGs::LightWeight::RNG light_rng(PractRand::SEED_AUTO); PractRand::RNGs::Polymorphic::RNG poly_rng(PractRand::SEED_AUTO); light_rng.set_rounds(ROUNDS); poly_rng.set_rounds(ROUNDS); printf("  %5.3f GB/s  :  %5.3f GB/s  :%7.0f KHz  :  %s(%d)\n", _measure_RNG_performance_32(&light_rng)/1024, _measure_RNG_performance_32(&poly_rng)/1024, benchmark_seeding<PractRand::RNGs::Polymorphic:: RNG >()/1000, #RNG, ROUNDS );}
//	printf("  light-weight   polymorphic    name\n");
	printf("  light-weight   polymorphic    seeding       name\n");
	printf("small fast RNGs:\n");
	PERF(jsf32);
	PERF(jsf64);
	PERF(sfc32);
	PERF(sfc64);
	printf("random access RNGs:\n");
	PERF(xsm32);
	PERF(xsm64);
	printf("entropy pooling RNGs:\n");
	PERF(arbee);
	PERF_POLYMORPHIC_ONLY(sha2_based_pool);
	printf("crypto / high quality RNGs:\n");
	PERF(hc256);
	PERF(trivium);
	PERF(isaac32x256);
	PERF(isaac64x256);
	PERF(efiix32x48);
	PERF(efiix64x48);
	//PERF(chacha);
	PERF_CHACHA(chacha, 8);
	PERF_CHACHA(chacha, 12);
	PERF_CHACHA(chacha, 20);
	//PERF(salsa);
	PERF_CHACHA(salsa, 8);
	PERF_CHACHA(salsa, 12);
	PERF_CHACHA(salsa, 20);
	printf("popular RNGs:\n");
	PERF(mt19937);
	printf("16 bit variants:\n");
	PERF(sfc16);
	PERF(efiix16x48);
	printf("8 bit variants:\n");
	PERF(efiix8x48);
	printf("candidate RNGs: (not recommended, but almost)\n");
	PERF_CANIDATE(siphash);
	PERF_CANIDATE(VeryFast32)
	PERF_CANIDATE(VeryFast64)
	PERF_CANIDATE(sfc_alternative32)
	PERF_CANIDATE(sfc_alternative64)
	PERF_CANIDATE(ranrot_variant32)
	PERF_CANIDATE(ranrot_variant64)
	printf("16 & 8 bit candidate RNGs: (not recommended, but almost)\n");
	PERF_CANIDATE(VeryFast16)
	PERF_CANIDATE(ranrot_variant16)
	PERF_CANIDATE(ranrot_variant8)
	PERF_CANIDATE(sfc_alternative16)
	//	PERF_CANIDATE(mcx32)
//	PERF_CANIDATE(mcx64)
#undef PERF
#undef PERF_CANIDATE
#undef PERF_POLYMORPHIC_ONLY
}
union DataBlock {
	enum {
		SIZE_L2 = 10,
		SIZE = 1<<SIZE_L2
	};
	Uint8  as8 [SIZE  ];
	Uint16 as16[SIZE/2];
	Uint32 as32[SIZE/4];
	Uint64 as64[SIZE/8];
};
#define DECLARE_EP_BENCH_FUNC(bits) double benchmark_entropy_pool_ ## bits (PractRand::RNGs::vRNG &entropy_pool, DataBlock *data) {\
	enum {NUM_CLOCKS_TO_TEST = int(CLOCKS_PER_SEC * .15) + 1};\
	int clock0 = clock();\
	int clock1, clock2;\
	while ((clock1 = clock()) == clock0) ;\
	int j = 0;\
	do {\
		for (int i = 0; i < DataBlock::SIZE*8 / bits; i++) entropy_pool.add_entropy ## bits (data->as ## bits [i]);\
		j++;\
	} while ((clock2=clock())-clock1 < NUM_CLOCKS_TO_TEST);\
	double delta = (clock2 - clock1) / double(CLOCKS_PER_SEC);\
	double amount = j / 1024.0;\
	double rate = amount / delta;\
	entropy_pool.flush_buffers();\
	Uint64 a = entropy_pool.raw64() & entropy_pool.raw64();\
	if (a == 0) printf("unlikely!");\
	return rate;\
}
DECLARE_EP_BENCH_FUNC(8)
DECLARE_EP_BENCH_FUNC(16)
DECLARE_EP_BENCH_FUNC(32)
DECLARE_EP_BENCH_FUNC(64)
double benchmark_entropy_pool_N (PractRand::RNGs::vRNG &entropy_pool, DataBlock *data) {\
	enum {NUM_CLOCKS_TO_TEST = int(CLOCKS_PER_SEC * .15) + 1};
	int clock0 = clock();
	int clock1, clock2;
	while ((clock1 = clock()) == clock0) ;
	int j = 0;
	do {
		entropy_pool.add_entropy_N (&data->as8[0], DataBlock::SIZE);
		j++;
	} while ((clock2=clock())-clock1 < NUM_CLOCKS_TO_TEST);
	double delta = (clock2 - clock1) / double(CLOCKS_PER_SEC);
	double amount = j / 1024.0;
	double rate = amount / delta;
	entropy_pool.flush_buffers();
	Uint64 a = entropy_pool.raw64() & entropy_pool.raw64();
	if (a == 0) printf("unlikely!");
	return rate;
}
void benchmark_random_access_rngs() {
	enum {NUM_CLOCKS_TO_TEST = int(CLOCKS_PER_SEC * .15) + 1};
	RNGs::Polymorphic::isaac64x256 rng(PractRand::SEED_AUTO);
	DataBlock data;
	for (int i = 0; i < DataBlock::SIZE / sizeof(Uint64); i++) data.as64[i] = rng.raw64();
	RNGs::Polymorphic::arbee poly_arbee;
	RNGs::Polymorphic::sha2_based_pool sha2_based;
#define POLYPERF(a) {printf("  %s\n", a.get_name().c_str() ); printf("    add_entropy8  :%6.1f MB/s\n    add_entropy16 :%6.1f MB/s\n    add_entropy32 :%6.1f MB/s\n    add_entropy64 :%6.1f MB/s\n    add_entropy_N :%6.1f MB/s\n", benchmark_entropy_pool_8(a, &data), benchmark_entropy_pool_16(a, &data), benchmark_entropy_pool_32(a, &data), benchmark_entropy_pool_64(a, &data), benchmark_entropy_pool_N(a, &data));}
	POLYPERF(poly_arbee)
	POLYPERF(sha2_based)
}
void benchmark_entropy_pool_input() {
	enum {NUM_CLOCKS_TO_TEST = int(CLOCKS_PER_SEC * .15) + 1};
	RNGs::Polymorphic::isaac64x256 rng(PractRand::SEED_AUTO);
	DataBlock data;
	for (int i = 0; i < DataBlock::SIZE / sizeof(Uint64); i++) data.as64[i] = rng.raw64();
	RNGs::Polymorphic::arbee poly_arbee;
	RNGs::Polymorphic::sha2_based_pool sha2_based;
#define POLYPERF(a) {printf("  %s\n", a.get_name().c_str() ); printf("    add_entropy8  :%6.1f MB/s\n    add_entropy16 :%6.1f MB/s\n    add_entropy32 :%6.1f MB/s\n    add_entropy64 :%6.1f MB/s\n    add_entropy_N :%6.1f MB/s\n", benchmark_entropy_pool_8(a, &data), benchmark_entropy_pool_16(a, &data), benchmark_entropy_pool_32(a, &data), benchmark_entropy_pool_64(a, &data), benchmark_entropy_pool_N(a, &data));}
	POLYPERF(poly_arbee)
	POLYPERF(sha2_based)
}

int main(int argc, char **argv) {
	PractRand::initialize_PractRand();
//	PractRand::self_test_PractRand();

	printf("Random number generation speeds:\n");
	benchmark_RNG_speeds();
	printf("\n");
	printf("Entropy pool input speeds:\n");
	benchmark_entropy_pool_input();
	printf("\n");

	return 0;
}
