
/*
RNGs in the mediocre directory are not intended for real world use
only for research; as such they may get pretty sloppy in some areas

This set is of RNGs that:
1. use an array with dynamic access patterns
2. don't use complex flow control or advanced math functions (sin/cos, log/exp, etc)
3. are likely to have detectable bias
*/

namespace PractRand {
	namespace RNGs {
		namespace Polymorphic {
			namespace NotRecommended {
				//the classic cryptographic RNG
				class rc4 : public vRNG8 {
				protected:
					Uint8 arr[256];
					Uint8 a, b;
				public:
					Uint8 raw8();
					std::string get_name() const;
					//void seed (Uint64 s);
					void walk_state(StateWalkingObject *);
				};

				//weaker variant of the classic cryptographic RNG
				//(same state transitions, different output)
				class rc4_weakenedA : public rc4 {
				public:
					Uint8 raw8();
					std::string get_name() const;
				};
				class rc4_weakenedB : public rc4 {
				public:
					Uint8 raw8();
					std::string get_name() const;
				};
				class rc4_weakenedC : public rc4 {
				public:
					Uint8 raw8();
					std::string get_name() const;
				};
				class rc4_weakenedD : public rc4 {
				public:
					Uint8 raw8();
					std::string get_name() const;
				};


				//based upon the IBAA algorithm by Robert Jenkins
				//but reduced strength via smaller integers and smaller table sizes
				class ibaa8 : public vRNG8 {
					int table_size_L2;
					Uint8 *table;
					Uint8 a, b, left;
				public:
					Uint8 raw8();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
					ibaa8(int table_size_L2_);
					~ibaa8();
				};
				class ibaa16 : public vRNG16 {
					int table_size_L2;
					Uint16 *table;
					Uint16 a, b, left;
				public:
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
					ibaa16(int table_size_L2_);
					~ibaa16();
				};
				class ibaa32 : public vRNG32 {
					int table_size_L2;
					Uint32 *table;
					Uint32 a, b, left;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
					ibaa32(int table_size_L2_);
					~ibaa32();
				};
				//based upon the ISAAC algorithm by Robert Jenkins
				//but adapted slightly to permit smaller minimum table sizes
				class isaac32_varqual : public vRNG32 {
					int table_size_L2;
					Uint32 *table;
					Uint32 a, b, c, left;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
					isaac32_varqual(int table_size_L2_);
					~isaac32_varqual();
				};
				//as isaac32_small, but adapted to use 16 bit integers instead of 32
				class isaac16_varqual : public vRNG16 {
					int table_size_L2;
					Uint16 *table;
					Uint16 a, b, c, left;
				public:
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
					isaac16_varqual(int table_size_L2_);
					~isaac16_varqual();
				};
				class efiix8_varqual : public vRNG8 {
					enum {SHIFT_AMOUNT=2};
					//average cycle length, in bytes: 2**(4 * W - 2), where W=(4 + iteration_table_size + indirection_table_size)
					int iteration_table_size_m1;
					int indirection_table_size_m1;
					Uint8 *iteration_table;
					Uint8 *indirection_table;
					Uint8 a, b, c, i;
				public:
					Uint8 raw8();
					void walk_state(StateWalkingObject *);
					std::string get_name() const;
					efiix8_varqual(int iteration_table_size_L2, int indirection_table_size_L2);
					~efiix8_varqual();
				};
				//efiix algorithm, shrunk down to operate on 4 bit integers (reports itself as an 8 bit PRNG)
				class efiix4_varqual : public vRNG8 {
					//average cycle length, in bytes: 2**(4 * W - 2), where W=(4 + iteration_table_size + indirection_table_size)
					int iteration_table_size_m1;
					int indirection_table_size_m1;
					Uint8 *iteration_table;
					Uint8 *indirection_table;
					Uint8 a, b, c, i;
					Uint8 raw4();
					static Uint8 rotate4(Uint8 value, int bits);
				public:
					Uint8 raw8();
					void walk_state(StateWalkingObject *);
					std::string get_name() const;
					efiix4_varqual(int iteration_table_size_L2, int indirection_table_size_L2);
					~efiix4_varqual();
				};

				//generic indirection-based PRNGs, just for testing test suites
				class genindA : public vRNG16 {
					// 4:25, 5:27, 6:32, 7:38, 8:41, 9:46
					Uint16 *table;
					Uint16 a, i;
					int table_size_mask;
					int shift;
				public:
					genindA(int size_L2);
					~genindA();
					Uint16 raw16();
					void walk_state(StateWalkingObject *);
					std::string get_name() const;
				};
				class genindB : public vRNG16 {
					//efiix-style, single pool, irreversible
					// 0*:34, 1:30, 2:34, 3:39, 4:42
					int shift;
					int table_size_mask;
					Uint16 *table;
					Uint16 a, b, i;
				public:
					genindB(int size_L2);
					~genindB();
					Uint16 raw16();
					void walk_state(StateWalkingObject *);
					std::string get_name() const;
				};
				class genindC : public vRNG16 {
					//split pool, bits fed in to the accumulator is limited to tsL2 per output
					// 1*:23, 2:31, 3:36, 4:42
					Uint16 *table;
					Sint16 left;
					Uint16 a;
					int table_size_L2;
				public:
					Uint16 raw16() {
						if (left >= 0) return table[left--];
						else return refill();
					}
					genindC(int size_L2);
					~genindC();
					Uint16 refill();
					void walk_state(StateWalkingObject *);
					std::string get_name() const;
				};
				class genindD : public vRNG16 {
					//fairly solid, RC4-style
					// 2:25, 3:27, 4:30, 5:32, 6:35, 7:37, 8:39, 9:41, 10:43
					Uint16 *table;
					Uint16 a, i;
					Uint16 mask;
					Uint16 table_size_L2;
				public:
					Uint16 raw16();
					genindD(int size_L2);
					~genindD();
					void walk_state(StateWalkingObject *);
					std::string get_name() const;
				};
				class genindE : public vRNG16 {
					//split pool
					// 1:33, 2:39, 3:46
					Uint16 *table1;
					Uint16 *table2;
					Uint16 a, i;
					Uint16 mask;
					Uint16 table_size_L2;
				public:
					Uint16 raw16();
					genindE(int size_L2);
					~genindE();
					void walk_state(StateWalkingObject *);
					std::string get_name() const;
				};
				class genindF : public vRNG16 {
					//split pool
					// 1:28, 2:34, 3:37, 4:40, 5:45
					Uint16 *table1;
					Uint16 *table2;
					Uint16 a, i;
					Uint16 mask;
					Uint16 table_size_L2;
				public:
					Uint16 raw16();
					genindF(int size_L2);
					~genindF();
					void walk_state(StateWalkingObject *);
					std::string get_name() const;
				};
			}
		}
	}
}
