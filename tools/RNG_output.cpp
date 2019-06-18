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
#include <iostream>
#include <sstream>
//#include <map>
#include <signal.h>     /* signal, sig_atomic_t */

#ifdef WIN32 // needed to allow binary stdout on windows
#include <fcntl.h>
#include <io.h>
#endif

//master header, includes everything in PractRand for both 
//  practical usage and research... 
//  EXCEPT it does not include specific algorithms
//  also it does not include PractRand/RNG_adaptors.h as that includes lots of templated stuff
#include "PractRand_full.h"
//the full version is needed because non-recommended RNGs are supported

//specific RNG algorithms, to produce (pseudo-)random numbers
#include "PractRand/RNGs/all.h"

#include "PractRand/RNGs/other/transform.h"
#include "PractRand/RNGs/other/mult.h"
#include "PractRand/RNGs/other/simple.h"
#include "PractRand/RNGs/other/fibonacci.h"
#include "PractRand/RNGs/other/indirection.h"
#include "PractRand/RNGs/other/special.h"

//not actually part of the library headers, just some inline code for an abstract factory for PractRand RNG name -> in
#include "RNG_from_name.h"

using namespace PractRand;
#include "Candidate_RNGs.h"


bool interpret_seed(const std::string &seedstr, Uint64 &seed) {
	//would prefer strtol, but that is insufficiently portable when it has to handle 64 bit values
	Uint64 value = 0;
	Uint64 position = 0;
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
void print_usage(const char *program_name) {
	std::cerr << "usage:\n\t" << program_name << " RNG_name bytes_to_output [64bit_hexadecimal_seed]\n";
	std::cerr << "  example:\n\t" << program_name << " jsf32 16\n";
	std::cerr << "  prints 16 bytes using the jsf32 RNG with a randomly chosen seed, with an \n";
	std::cerr << "    error message if fewer than 16 bytes were successfully outputted.\n";
	std::cerr << "usage:\n\t" << program_name << " RNG_name inf [64bit_hexadecimal_seed]\n";
	std::cerr << "  as above, but it prints indefinitely, with no error message when aborted\n";
	std::cerr << "usage:\n\t" << program_name << " RNG_name name\n";
	std::cerr << "  It prints the result of (RNG)->get_name()\n";
	std::cerr << "  Which is often the same as the RNG_name parameter, but not always.\n";
	exit(0);
}

sig_atomic_t signaled = 0;

void signal_handler(int param)
{
	signaled = param;
}

#include "SeedingTester.h"

int main(int argc, char **argv) {
#ifdef WIN32
	_setmode( _fileno(stdout), _O_BINARY); // needed to allow binary stdout on windows
#endif
	if (argc < 3 || argc > 4) print_usage(argv[0]);
	PractRand::initialize_PractRand();

	RNG_Factories::register_recommended_RNGs();
	RNG_Factories::register_nonrecommended_RNGs();
	RNG_Factories::register_candidate_RNGs();
	Seeder_MetaRNG::register_name();
	EntropyPool_MetaRNG::register_name();
	std::string errmsg;
	RNGs::vRNG *rng = RNG_Factories::create_rng(argv[1], &errmsg);

	if (!rng) {
		if (errmsg.empty()) { std::fprintf(stderr, "RNG_output ERROR: unrecognized RNG name\n"); print_usage(argv[0]); }
		else { std::fprintf(stderr, "RNG_output ERROR: RNG_Factories returned error message:\n%s\n", errmsg.c_str()); exit(1); }
	}

	double _n = atof(argv[2]);//should be atol, but on 32 bit systems that's too limited
	Uint64 n;
	if (_n <= 0 || _n >= 18446744073709551616.0) {
		if (!strcmp(argv[2], "name")) {
			std::printf("%s\n", rng->get_name().c_str());
			exit(0);
		}
		else if (!strcmp(argv[2], "inf")) {
			_n = 0;
			n = 0xFFFFffffFFFFffffull;
		}
		else {
			std::fprintf(stderr, "RNG_output ERROR: invalid number of output bytes\n"); print_usage(argv[0]);
		}
	}
	else n = Uint64(_n);

	if (argc == 3) rng->autoseed();
	else {
		Uint64 seed;
		if (!interpret_seed(argv[3],seed)) {std::fprintf(stderr, "RNG_output ERROR: \"%s\" is not a valid 64 bit hexadecimal seed\n", argv[3]); std::exit(0);}
		rng->seed(seed);
	}

	void(*prev_handler)(int);
	prev_handler = signal(SIGINT, signal_handler);  if (prev_handler == SIG_ERR) { std::cerr << "WARNING: Setting signal handler for SIGINT has failed." << std::endl; }
	prev_handler = signal(SIGTERM, signal_handler); if (prev_handler == SIG_ERR) { std::cerr << "WARNING: Setting signal handler for SIGTERM has failed." << std::endl; }
#ifdef __linux__
	prev_handler = signal(SIGPIPE, signal_handler); if (prev_handler == SIG_ERR) { std::cerr << "WARNING: Setting signal handler for SIGPIPE has failed." << std::endl; }
#endif  

	enum {BUFFER_SIZE = 8};
	//Uint64 buffer[BUFFER_SIZE];
	PractRand::Tests::TestBlock buffer[BUFFER_SIZE];
	while (n && !signaled) {
		//for (int i = 0; i < BUFFER_SIZE; i++) buffer[i] = rng->raw64();
		std::size_t bytes_this_loop = n > (BUFFER_SIZE * PractRand::Tests::TestBlock::SIZE) ? (BUFFER_SIZE * PractRand::Tests::TestBlock::SIZE) : n;
		buffer[0].fill(rng, (bytes_this_loop + PractRand::Tests::TestBlock::SIZE - 1) >> PractRand::Tests::TestBlock::SIZE_L2);
		size_t bytes_written = std::fwrite(&buffer[0], 1, bytes_this_loop, stdout);
		n -= bytes_written;
		if ( bytes_written != bytes_this_loop ) {
			//if (std::ferror(stdout)) std::perror("I/O error when writing to standard output"); // this was generating spurious error messages on windows
			break;
		}
	}
	std::fflush(stdout);
	if (signaled) {
		//std::cerr << "WARNING: Received signal " << signaled << ". Closing the application." << std::endl; // this was generating spurious error messages on linux
	}
	if (n && _n) {
		std::cerr << "RNG_output ERROR: " << Uint64(_n) << " bytes were requested, but only " << (Uint64(_n) - n) << " bytes were written." << std::endl;
	}
	return 0;
}
