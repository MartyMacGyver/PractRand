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
			//parameters:
			//  rng
			//      RNG metadata effects which tests/foldings are used
			//      may be NULL to disregard RNG metadata and use default tests/foldings
			//  expanded_test_set
			//      if 0, PractRand will favor speed over sensitivity
			//      if 9 it will favor sensitivity over speed
			//      at intermediate values it may produce intermediate results
			//  folding_level
			//      if 0, PractRand will test only raw RNG output
			//      if 1, it will focus additional effort on the lowest bits of the RNG output (recommended)
			//      if 2 or more it will test additional transforms of the PRNG output
			//ListOfTests get_test_battery(const RNGs::vRNG *rng, int expanded_test_set = 0, int folding_level = 1);

			//recommended tests
			//recommended data transformations
			//(which data transforms are recommended is a function of RNG metadata)
			ListOfTests get_standard_tests(const RNGs::vRNG *rng);

			//recommended tests
			//no data transformations
			ListOfTests get_core_tests();

			//recommended tests
			//extra data transformations
			ListOfTests get_folded_tests();

			//extra tests
			//no data transformations
			ListOfTests get_expanded_core_tests();

			//extra tests
			//recommended data transformations
			//(which data transforms are recommended is a function of RNG metadata)
			ListOfTests get_expanded_standard_tests(const RNGs::vRNG *rng);

			//extra tests
			//extra data transformations
			ListOfTests get_expanded_folded_tests();

		}//Batteries
	}//Tests
}//PractRand

#endif //__PRACTRAND_TEST_BATTERIES_H__
