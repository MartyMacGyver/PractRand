
/*
RNGs in the mediocre directory are not intended for real world use
only for research; as such they may get pretty sloppy in some areas

This set is of RNGs that:
1. use an array with repetitive access patterns - generally a Fibonacci-style cyclic buffer
2. don't use much flow control, variable shifts, etc
3. are likely to have easily detectable bias
*/

namespace PractRand {
	namespace RNGs {
		namespace Polymorphic {
			namespace NotRecommended {
				//large-state LCGs with very poor constants
				class bigbadlcg64X : public vRNG64 {
					enum { MAX_N = 16 };
					Uint64 state[MAX_N];
					int n;
				public:
					int discard_bits;
					int shift_i;
					int shift_b;
					Uint64 raw64();
					bigbadlcg64X(int discard_bits_, int shift_);
					//~bigbadlcgX();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class bigbadlcg32X : public vRNG32 {
				public:
					bigbadlcg64X base_lcg;
					bigbadlcg32X(int discard_bits_, int shift_);
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class bigbadlcg16X : public vRNG16 {
				public:
					bigbadlcg64X base_lcg;
					bigbadlcg16X(int discard_bits_, int shift_);
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class bigbadlcg8X : public vRNG8 {
				public:
					bigbadlcg64X base_lcg;
					bigbadlcg8X(int discard_bits_, int shift_);
					Uint8 raw8();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};

				//Mitchell-Moore: LFib32(Uint32, 55, 24, ADD)
				class mm32 : public vRNG32 {
					Uint32 cbuf[55];
					Uint8 index1, index2;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				//Mitchell-Moore modified: LFib16(Uint32, 55, 24, ADD) >> 16
				class mm16of32 : public vRNG16 {
					Uint32 cbuf[55];
					Uint8 index1, index2;
				public:
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				//Mitchell-Moore modified: LFib32(Uint32, 55, 24, ADC)
				class mm32_awc : public vRNG32 {
					Uint32 cbuf[55];
					Uint8 index1, index2, carry;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				//Mitchell-Moore modified: LFib16(Uint32, 55, 24, ADC)
				class mm16of32_awc : public vRNG16 {
					Uint32 cbuf[55];
					Uint8 index1, index2, carry;
				public:
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};

				class lfsr_medium : public vRNG8 {
					enum {SIZE=55, LAG=25};//0 < LAG < SIZE-2
					Uint8 cbuf[55];
					Uint8 table1[256], table2[256];
					Uint8 used;
				public:
					lfsr_medium();
					Uint8 raw8();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};


				//proposed by Marsaglia
				class mwc4691 : public vRNG32 {
					Uint32 cbuf[4691];
					unsigned int index, carry;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				//proposed by Marsaglia
				//class cwsb4288;

				class cbuf_accum : public vRNG32 {
					enum {L = 32};
					Uint32 cbuf[L], accum;
					Uint8 index;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class cbuf_accum_big : public vRNG32 {
					enum { L = 128 };
					Uint32 cbuf[L], accum;
					Uint32 index;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class cbuf_2accum_small : public vRNG32 {
					enum { L = 3 };
					Uint32 cbuf[L], accum1, accum2;
					Uint8 index;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class cbuf_2accum : public vRNG32 {
					enum { L = 12 };
					Uint32 cbuf[L], accum1, accum2;
					Uint8 index;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class dual_cbuf_small : public vRNG32 {
					enum { L1 = 3, L2 = 5 };
					Uint32 cbuf1[L1], cbuf2[L2];
					Uint8 index1, index2;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class dual_cbuf : public vRNG32 {
					enum { L1 = 13, L2 = 19 };
					Uint32 cbuf1[L1], cbuf2[L2];
					Uint8 index1, index2;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class dual_cbufa_small : public vRNG32 {
					enum { L1 = 4, L2 = 5 };
					Uint32 cbuf1[L1], cbuf2[L2], accum;
					Uint8 index1, index2;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class dual_cbuf_accum : public vRNG32 {
					enum { L1 = 13, L2 = 19 };
					Uint32 cbuf1[L1], cbuf2[L2], accum;
					Uint8 index1, index2;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class ranrot32small : public vRNG32 {
					enum { LAG1 = 7, LAG2 = 3, ROT1 = 9, ROT2 = 13 };
					Uint32 buffer[LAG1]; // LAG1 > LAG2 > 0
					Uint8 position;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class ranrot32 : public vRNG32 {
					enum { LAG1 = 17, LAG2 = 9, ROT1 = 9, ROT2 = 13 };
					Uint32 buffer[LAG1]; // LAG1 > LAG2 > 0
					Uint8 position;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class ranrot32big : public vRNG32 {
					enum { LAG1 = 57, LAG2 = 13, ROT1 = 9, ROT2 = 13 };
					Uint32 buffer[LAG1]; // LAG1 > LAG2 > 0
					Uint8 position;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class ranrot3tap32small : public vRNG32 {
					enum { LAG1 = 7, LAG2 = 3, LAG3 = 1, ROT1 = 3, ROT2 = 17, ROT3 = 9 };//7,3:29, 9,4:33, 11,5:34, 13,6:34, 15,7:35, 17,9:38
					Uint32 buffer[LAG1]; // LAG1 > LAG2 > LAG3, LAG3 = 1
					Uint8 position;
					static Uint32 func(Uint32 a, Uint32 b, Uint32 c);
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class ranrot3tap32 : public vRNG32 {
					enum { LAG1 = 17, LAG2 = 9, LAG3 = 1, ROT1 = 3, ROT2 = 17, ROT3 = 9 };//7,3:29, 9,4:33, 11,5:34, 13,6:34, 15,7:35, 17,9:38
					Uint32 buffer[LAG1]; // LAG1 > LAG2 > LAG3, LAG3 = 1
					Uint8 position;
					static Uint32 func(Uint32 a, Uint32 b, Uint32 c);
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class ranrot3tap32big : public vRNG32 {
					enum { LAG1 = 57, LAG2 = 13, LAG3 = 1, ROT1 = 3, ROT2 = 17, ROT3 = 9 };//7,3:29, 9,4:33, 11,5:34, 13,6:34, 15,7:35, 17,9:38
					Uint32 buffer[LAG1]; // LAG1 > LAG2 > LAG3, LAG3 = 1
					Uint8 position;
					static Uint32 func(Uint32 a, Uint32 b, Uint32 c);
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class ranrot32hetsmall : public vRNG32 {
					enum { LAG1 = 7, LAG2 = 4, LAG3 = 1, ROT1 = 3, ROT2 = 17, ROT3 = 9 };//7,3:32, 9,4:36, 11,5:37, 13,6:38-, 15,6:38, 17,9:40?
					Uint32 buffer[LAG1]; // LAG1 > LAG2 > LAG3, LAG3 = 1
					Uint8 position;
					static Uint32 func(Uint32 a, Uint32 b, Uint32 c);
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class ranrot32het : public vRNG32 {
					enum { LAG1 = 17, LAG2 = 9, LAG3 = 1, ROT1 = 3, ROT2 = 17, ROT3 = 9 };//7,3:32, 9,4:36, 11,5:37, 13,6:38-, 15,6:38, 17,9:40?
					Uint32 buffer[LAG1]; // LAG1 > LAG2 > LAG3, LAG3 = 1
					Uint8 position;
					static Uint32 func(Uint32 a, Uint32 b, Uint32 c);
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class ranrot32hetbig : public vRNG32 {
					enum { LAG1 = 57, LAG2 = 13, LAG3 = 1, ROT1 = 3, ROT2 = 17, ROT3 = 9 };//7,3:32, 9,4:36, 11,5:37, 13,6:38-, 15,6:38, 17,9:40?
					Uint32 buffer[LAG1]; // LAG1 > LAG2 > LAG3, LAG3 = 1
					Uint8 position;
					static Uint32 func(Uint32 a, Uint32 b, Uint32 c);
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class fibmul16of32 : public vRNG16 {// 31 @ 17/9
					enum {LAG1 = 17, LAG2 = 5};
					Uint32 buffer[LAG1]; // LAG1 > LAG2 > 0
					Uint8 position;
				public:
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class fibmul32of64 : public vRNG32 {// 35 @ 3/2, 39 @ 7/5
					enum {LAG1 = 7, LAG2 = 5};
					Uint64 buffer[LAG1]; // LAG1 > LAG2 > 0
					Uint8 position;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class mt19937_unhashed : public vRNG32 {// 
					PractRand::RNGs::Raw::mt19937 implementation;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
			}
		}
	}
}
