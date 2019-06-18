#ifndef __PRACTRAND_TEST_BATTERIES_H__
#define __PRACTRAND_TEST_BATTERIES_H__

namespace PractRand {
	namespace Tests {
		class TestBaseclass;

		class ListOfTests {
		public:
			std::vector<TestBaseclass*> tests;
			ListOfTests ( ) {}
			ListOfTests ( TestBaseclass **tests_ );
			ListOfTests ( 
				TestBaseclass *test1, TestBaseclass *test2=NULL, TestBaseclass *test3=NULL, TestBaseclass *test4=NULL, 
				TestBaseclass *test5=NULL, TestBaseclass *test6=NULL, TestBaseclass *test7=NULL, TestBaseclass *test8=NULL,
				TestBaseclass *test9=NULL, TestBaseclass *test10=NULL, TestBaseclass *test11=NULL, TestBaseclass *test12=NULL,
				TestBaseclass *test13=NULL, TestBaseclass *test14=NULL, TestBaseclass *test15=NULL, TestBaseclass *test16=NULL
			);
		};

		//batteries
		namespace Batteries {
			//simply calls the destructors on all tests in list of tests
			void destruct_tests(const ListOfTests &tests);

			//gets a battery of of tests
			//
			// notes:
			//    bits
			//        bits is the number of bits produced at a time by the targetted PRNG
			//        its value can normally be produced with a call to vRNG::get_native_output_size()
			//    folding
			//        folding refers to the process by which additional datastreams are constructed from subsets of the original datastream
			//        each datastream, including the original (raw PRNG output) gets its own copy of the tests working on it
			//
			//core test set without folding:
			//    get_core_tests();
			//
			//core test set with standard folding:
			//    apply_standard_folding(bits, get_core_tests);
			//
			//core test set with extended folding:
			//    apply_extended_folding(get_core_tests);
			//
			//expanded test set without folding
			//    get_expanded_core_tests();
			//
			//expanded test set with standard folding
			//    apply_standard_folding(bits, get_expanded_core_tests);
			//
			//expanded test set with extended folding
			//    apply_extended_folding(get_expanded_core_tests);
			//

			//recommended tests
			ListOfTests get_core_tests();

			//extra tests
			ListOfTests get_expanded_core_tests();

			//apply standard foldings to a test set:
			ListOfTests apply_standard_foldings(int bits, ListOfTests(*base_tests)());
			ListOfTests apply_standard_foldings(const RNGs::vRNG *rng, ListOfTests(*base_tests)());
			//note: there is specific behavior for 8 bit, 16 bit, 32 bit, and 64 bit cases ; any other value produces an alternate "unknown format" targetted folding

			//apply extended foldings to a test set:
			ListOfTests apply_extended_foldings(ListOfTests(*base_tests)());

		}//Batteries
	}//Tests
}//PractRand

#endif //__PRACTRAND_TEST_BATTERIES_H__
