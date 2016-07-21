#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <cstring>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <iostream>
//#include <map>

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
	std::cerr << "example:\n\t" << program_name << " jsf32 16\n";
	std::cerr << "prints 16 bytes using the jsf32 RNG with a randomly chosen seed\n";
	exit(0);
}



int main(int argc, char **argv) {
#ifdef WIN32
	_setmode( _fileno(stdout), _O_BINARY); // needed to allow binary stdout on windows
#endif
	if (argc < 3 || argc > 4) print_usage(argv[0]);
	PractRand::initialize_PractRand();

	RNG_Factories::register_recommended_RNGs();
	RNG_Factories::register_nonrecommended_RNGs();
	RNGs::vRNG *rng = RNG_Factories::create_rng(argv[1]);

	if (!rng) {std::fprintf(stderr, "unrecognized RNG name\n"); print_usage(argv[0]);}

	double _n = atof(argv[2]);//should be atol, but on 32 bit systems that's too limited
	if (_n <= 0) {
		if (!strcmp(argv[2], "name")) {
			std::printf("%s\n", rng->get_name().c_str());
			exit(0);
		}
		std::fprintf(stderr, "invalid number of output bytes\n"); print_usage(argv[0]);
	}

	if (argc == 3) rng->autoseed();
	else {
		Uint64 seed;
		if (!interpret_seed(argv[3],seed)) {std::fprintf(stderr, "\"%s\" is not a valid 64 bit hexadecimal seed\n", argv[3]); std::exit(0);}
		rng->seed(seed);
	}

	Uint64 n = Uint64(_n);
	Uint64 buffer[64];
	while (n) {
		for (int i = 0; i < 64; i++) buffer[i] = rng->raw64();
		std::size_t b = 64 * 8;
		b = (b > n) ? std::size_t(n) : b;
		n -= b;
		//std::fwrite(&buffer[0], b, 1, stdout);
		std::cout.write((char*)&buffer[0], b);
		if (std::cout.bad()) {
			//std::cerr << "output error occured" << std::endl;
			//std::exit(1);
			std::exit(0);
		}
	}
	std::cout.flush();
	return 0;
}
