#include <string>
//#include <ostream>
#include <sstream>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <cmath>
#include <cstdlib>

#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/tests.h"
#include "PractRand/test_helpers.h"
#include "PractRand/test_batteries.h"
#include "PractRand/Tests/Gap16.h"
#include "PractRand/Tests/DistC6.h"
#include "PractRand/Tests/Pat5.h"
#include "PractRand/Tests/BCFN.h"
#include "PractRand/Tests/BCFN_MT.h"
#include "PractRand/Tests/FPF.h"
#include "PractRand/Tests/FPMulti.h"
#include "PractRand/Tests/birthday.h"
#include "PractRand/Tests/CoupGap.h"
#include "PractRand/Tests/BRank.h"
#include "PractRand/Tests/mod3.h"
#include "PractRand/Tests/NearSeq.h"
#include "PractRand/Tests/Coup16.h"
#include "PractRand/Tests/DistFreq4.h"
#include "PractRand/Tests/transforms.h"


namespace PractRand {
	namespace Tests {
		namespace Batteries {
			void destruct_tests(ListOfTests &tests) {
				for (unsigned i = 0; i < tests.tests.size(); i++) {
					delete tests.tests[i];
				}
			}
			ListOfTests get_core_tests() {
				return Tests::ListOfTests(
					//major tests - we spend a lot of time on them, and they catch a lot of stuff
					new Tests::BCFN(2, 13, true), //        1.6 s/GB in isolation, +1.4 seconds in combined test runs
					new Tests::DistC6(9, 0, 1, 0, 0), //    1.8 s/GB in isolation, +1.5 seconds in combined test runs
					new Tests::Gap16(),//                   2.4 s/GB in isolation, +2.1 seconds in combined test runs
					new Tests::FPF(4, 14, 6), //            1.8 s/GB in isolation, +1.5 seconds in combined test runs

					//minor tests - carefully arranged to use very little resources, and catch a few categories that slip by the major tests
					new Tests::BRank(12), //                0.7 s/GB in isolation, +0.3 seconds in combined test runs
					new Tests::mod3n(5),  //                0.5 s/GB in isolation, +0.1 seconds in combined runs
					new Tests::TripleMirrorFreqN(2),	//	about 0.1 s/GB in combined test runs

					//experimentals - just for testing, disable for releases
					//new Tests::Birthday32(),
					//new Tests::Birthday64(),
					//new Tests::BirthdayLambda1(26),
					//new Tests::BirthdaySystematic(),
					//new Tests::FPMulti(5, 0),  //		12.6
					//new Tests::FPMulti(5, 1),  //		7.5
					//new Tests::FPMulti(5, 2),  //		3.9
					//new Tests::FPMulti(5, 3),  //		2.3
					//new Tests::FPMulti(5, 4),  //		1.4
					//new Tests::FPMulti(5, 5),  //		0.9
					//new Tests::FPMulti(4, 0),  //		27.1
					//new Tests::FPMulti(4, 1),  //		13.4
					//new Tests::FPMulti(4, 2),  //		7.2
					//new Tests::FPMulti(4, 3),  //		4.2
					//new Tests::FPMulti(4, 4),  //		2.6
					//new Tests::FPMulti(4, 5),  //		1.7
					//new Tests::BCFN_MT(1, 11),  //	
					//new Tests::FPMulti(5, 0),//			slow?
					//new Tests::TripleFreq(8, 1),  //	2.5 s/GB in isolation, +1.4 seconds in combined test runs
					//new Tests::LPerm16(32),//			?? s/GB
					//new Tests::DistFreq4(1),  //		?? s/GB
					//new Tests::QuadFreq(8,1),  //		?? s/GB
					//new Tests::NearSeq(),  //			?? s/GB
					NULL // 0.8 s/GB in isolation
				);
			}
			static Tests::ListOfTests standard_foldings_generic(ListOfTests (*base_tests)()) {
				Tests::ListOfTests l = base_tests();
				l.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 0, 0));
				//l.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 1, 1));
				Tests::ListOfTests sub4of32 = base_tests();
				sub4of32.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 0, -1));
				l.tests.push_back(new Tests::Transforms::lowbits(NULL, sub4of32, 2, 2));
				return l;
			}
			static Tests::ListOfTests standard_foldings8(ListOfTests (*base_tests)()) {
				Tests::ListOfTests l = base_tests();
				l.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 0, 0));
				return l;
			}
			static Tests::ListOfTests standard_foldings16(ListOfTests (*base_tests)()) {
				Tests::ListOfTests l = base_tests();
				Tests::ListOfTests sub4 = base_tests();
				sub4.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 0, -1));
				//sub4.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 1, -1));
				l.tests.push_back(new Tests::Transforms::lowbits(NULL, sub4, 2, 1));
				return l;
			}
			static Tests::ListOfTests standard_foldings32(ListOfTests (*base_tests)()) {
				Tests::ListOfTests l = base_tests();
				l.tests.push_back(new Tests::Transforms::lowbits(NULL, standard_foldings8(base_tests), 3, 2));
				return l;
			}
			static Tests::ListOfTests standard_foldings64(ListOfTests (*base_tests)()) {
				Tests::ListOfTests l = base_tests();
				Tests::ListOfTests sub16 = base_tests();
				Tests::ListOfTests sub4 = base_tests();
				sub4.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 0, -1));
				//sub4.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 1, -1));
				sub16.tests.push_back(new Tests::Transforms::lowbits(NULL, sub4, 2, 1));
				l.tests.push_back(new Tests::Transforms::lowbits(NULL, sub16, 4, 3));
				return l;
			}
			Tests::ListOfTests apply_standard_foldings( int bits, ListOfTests (*base_tests)() ) {
				switch (bits) {
					case 16: return standard_foldings16(base_tests);
					case 32: return standard_foldings32(base_tests);
					case 64: return standard_foldings64(base_tests);
					case  8: return standard_foldings8(base_tests);
					default: return standard_foldings_generic(base_tests);
				}
			}
			Tests::ListOfTests apply_standard_foldings( const RNGs::vRNG *rng, ListOfTests (*base_tests)() ) {
				return apply_standard_foldings(rng->get_native_output_size(), base_tests);
			}
			Tests::ListOfTests get_standard_tests( const RNGs::vRNG *rng ) {
				return apply_standard_foldings(rng, get_core_tests);
			}
			ListOfTests apply_extended_foldings(ListOfTests (*base_tests)()) {
				ListOfTests rv = base_tests();
				rv.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 0, 0));
				rv.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 0, 1));
				rv.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 0, 2));
				rv.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 0, 3));
				rv.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 2, 1));
				rv.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 2, 2));
				rv.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 2, 3));
				rv.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 3, 2));
				rv.tests.push_back(new Tests::Transforms::lowbits(NULL, base_tests(), 3, 3));
				return rv;
			}
			ListOfTests get_folded_tests() {
				return apply_extended_foldings(get_core_tests);
			}
			ListOfTests get_expanded_core_tests() {
				return Tests::ListOfTests(
					//long range linear tests:
					//new Tests::BCFN(2, 13, false), //	3.4 seconds/GB
					//new Tests::BCFN(1, 13, false), //	5.8 seconds/GB
					//new Tests::BCFN(1, 14, false), //	5.8 seconds/GB
					//new Tests::BCFN(0, 13, false), //	9.9 seconds/GB
					//new Tests::BCFN(2, 13, true ), //	2.8 seconds/GB
					//new Tests::BCFN(1, 13, true ), //	4.1 seconds/GB
					new Tests::BCFN(0, 13, true ), //	6.8 seconds/GB
					//new Tests::BCFN_FF(2, 13),    //	4.0 seconds/GB		the :freq results p-values aren't very good atm
					//new Tests::BCFN_FF(1, 13),  //	6.9 seconds/GB
					//new Tests::BCFN_FF(0, 13),  //	12.0 seconds/GB
					//new Tests::BCFN_MT(1, 11),  //	19.6 seconds/GB

					//new Tests::Birthday64(), //uses a 64 megabyte buffer, ~13 seconds/GB
					//new Tests::BirthdayLambda1(), //I keep adjusting the buffer size, but a 1 GB buffer is common
					//new Tests::BirthdaySystematic(25), //uses (up to) a 512 megabyte buffer
					//new Tests::BirthdaySystematic(26), //uses (up to) a 1 gigabyte buffer
					//new Tests::BirthdaySystematic(27), //uses (up to) a 2 gigabyte buffer
					//new Tests::BirthdaySystematic(28), //uses (up to) a 4 gigabyte buffer

					//medium range tests:
					new Tests::mod3n(0),//				7.5 s/GB with EXP==9, 8.1 with EXP==10
					new Tests::BRank(18), //			~4.0 s/GB          18
					new Tests::TripleMirrorFreqN(0),//	?

					//short range tests:
					new Tests::DistC6(9,0, 1,0,0),//	3.3->2.7 s/GB
					new Tests::DistC6(6,1, 1,0,0), //	2.5->2.2 s/GB
					new Tests::DistC6(5,2, 1,0,0),//	2.1->1.8 s/GB
					//new Tests::DistC6(5,3, 1,0,1),//	2.0->1.8 s/GB
					//new Tests::DistC6(4,3, 0,0,1),//	2.0->1.8 s/GB

					//new Tests::mod3_simple(), //		2.8 s/GB in isolation, +2.2 seconds in combined test runs

					new Tests::Pat5(),//                2.7 s/GB

					//new Tests::FPMulti(5, 0),//			slow?
					new Tests::FPF(6, 14, 6), //		? s/GB
					new Tests::FPF(5, 14, 6), //		2.2 s/GB
					new Tests::FPF(4, 14, 6), //		5.0->3.6 s/GB
					//new Tests::FPF(3, 14, 6), //		8.0->5.8 s/GB
					new Tests::FPF(2, 14, 6), //		14.1->10.5 s/GB
					//new Tests::FPF(1, 14, 6), //		26.0 s/GB
					//new Tests::FPF(0, 14, 6), //		51.5 s/GB

					//ambiguous to long range tests
					new Tests::Gap16(),//				3.4->3.1 s/GB
					//new Tests::NearSeq(),  //			???
					//new Tests::Coup16(),  //			???
					//new Tests::CoupGap(), //			???
					NULL
				);
			}
			ListOfTests get_expanded_standard_tests(const RNGs::vRNG *rng) {
				return apply_standard_foldings(rng, get_expanded_core_tests);
			}
			ListOfTests get_expanded_folded_tests() {
				return apply_extended_foldings(get_expanded_core_tests);
			}
		}//Batteries
	}//Tests
}//PractRand
