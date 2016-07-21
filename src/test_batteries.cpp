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
#include "PractRand/Tests/CoupGap.h"
#include "PractRand/Tests/BRank.h"
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
					new Tests::BCFN(2, 13, true), //	2.8 s/GB (from 3.4)
					new Tests::DistC6(9, 0, 1, 0, 0),//	2.7 s/GB (from 3.3)
					new Tests::Gap16(),//				3.1 s/GB (from 3.4)
					new Tests::FPF(4, 14, 6), //		3.4 s/GB (from 5.0?)
					new Tests::BRank(12), //			?? s/GB (negligable cost, mostly)
					NULL
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
			static Tests::ListOfTests apply_standard_foldings( const RNGs::vRNG *rng, ListOfTests (*base_tests)() ) {
				switch (rng ? rng->get_native_output_size() : 0) {
					case 16: return standard_foldings16(base_tests);
					case 32: return standard_foldings32(base_tests);
					case 64: return standard_foldings64(base_tests);
					case  8: return standard_foldings8(base_tests);
					default: return standard_foldings_generic(base_tests);
				}
			}
			Tests::ListOfTests get_standard_tests( const RNGs::vRNG *rng ) {
				return apply_standard_foldings(rng, get_core_tests);
			}
			static ListOfTests apply_extended_foldings(ListOfTests (*base_tests)()) {
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
					//new Tests::BCFN(0, 13, true ), //	6.8 seconds/GB
					new Tests::BCFN_FF(2, 13),  //	4.0 seconds/GB
					//new Tests::BCFN_FF(1, 13),  //	6.9 seconds/GB
					//new Tests::BCFN_FF(0, 13),  //	12.0 seconds/GB
					//new Tests::BCFNMT(1, 11),  //	19.6 seconds/GB

					//medium range tests:
					new Tests::BRank(18), //			~4.0 s/GB

					//short range tests:
					new Tests::DistC6(9,0, 1,0,0),//	3.3->2.7 s/GB
					new Tests::DistC6(6,1, 1,0,0), //	2.5->2.2 s/GB
					new Tests::DistC6(5,2, 1,0,0),//	2.1->1.8 s/GB
					//new Tests::DistC6(5,3, 1,0,1),//	2.0->1.8 s/GB
					//new Tests::DistC6(4,3, 0,0,1),//	2.0->1.8 s/GB

					new Tests::Pat5(),//                2.7 s/GB

					//new Tests::FPMulti(5, 0),//		slow?
					new Tests::FPF(5, 14, 6), //		2.2 s/GB
					new Tests::FPF(4, 14, 6), //		5.0->3.6 s/GB
					//new Tests::FPF(3, 14, 6), //		8.0->5.8 s/GB
					new Tests::FPF(2, 14, 6), //		14.1->10.5 s/GB
					//new Tests::FPF(1, 14, 6), //		26.0 s/GB
					//new Tests::FPF(0, 14, 6), //		51.5 s/GB

					//gap tests
					new Tests::Gap16(),//				3.4->3.1 s/GB
					//new Tests::CoupGap(),*/
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
