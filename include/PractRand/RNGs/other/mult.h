
/*
RNGs in the mediocre directory are not intended for real world use
only for research; as such they may get pretty sloppy in some areas

This set is of RNGs that:
1. use multiplication
2. don't use much indirection, flow control, variable shifts, etc
3. have only a few words of state
4. are likely to have easily detectable bias
*/

namespace PractRand {
	namespace RNGs {
		namespace Polymorphic {
			namespace NotRecommended {
				//similar to the classic LCGs, but with a longer period
				class lcg16of32_extended : public vRNG16 {
					Uint32 state, add;
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class lcg32_extended : public vRNG32 {
					Uint32 state, add;
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				//simple classic LCGs
				class lcg32of64_varqual : public vRNG32 {
					Uint64 state;
					int outshift;
				public:
					lcg32of64_varqual(int lcg1_discard_bits) : outshift(lcg1_discard_bits) {}
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class lcg16of64_varqual : public vRNG16 {
					Uint64 state;
					int outshift;
				public:
					lcg16of64_varqual(int lcg1_discard_bits) : outshift(lcg1_discard_bits) {}
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class lcg8of64_varqual : public vRNG8 {
					Uint64 state;
					int outshift;
				public:
					lcg8of64_varqual(int lcg1_discard_bits) : outshift(lcg1_discard_bits) {}
					Uint8 raw8();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class lcg32of128_varqual : public vRNG32 {
					Uint64 low, high;
					int outshift;
				public:
					lcg32of128_varqual(int lcg1_discard_bits) : outshift(lcg1_discard_bits) {}
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class lcg16of128_varqual : public vRNG16 {
					Uint64 low, high;
					int outshift;
				public:
					lcg16of128_varqual(int lcg1_discard_bits) : outshift(lcg1_discard_bits) {}
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class lcg8of128_varqual : public vRNG8 {
					Uint64 low, high;
					int outshift;
				public:
					lcg8of128_varqual(int lcg1_discard_bits) : outshift(lcg1_discard_bits) {}
					Uint8 raw8();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				//two LCGs combined
				class clcg8of96_varqual : public vRNG8 {
					Uint64 lcg1;
					Uint32 lcg2;
					int outshift;
				public:
					clcg8of96_varqual(int lcg1_discard_bits) : outshift(lcg1_discard_bits) {}
					Uint8 raw8();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class clcg16of96_varqual : public vRNG16 {
					Uint64 lcg1;
					Uint32 lcg2;
					int outshift;
				public:
					clcg16of96_varqual(int lcg1_discard_bits) : outshift(lcg1_discard_bits) {}
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class clcg32of96_varqual : public vRNG32 {
					Uint64 lcg1;
					Uint32 lcg2;
					int outshift;
				public:
					clcg32of96_varqual(int lcg1_discard_bits) : outshift(lcg1_discard_bits) {}
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				//LCGs modified by suppressing the carries
				class xlcg32of64_varqual : public vRNG32 {
					Uint64 state;
					int outshift;
				public:
					xlcg32of64_varqual(int lcg1_discard_bits) : outshift(lcg1_discard_bits) {}
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class xlcg16of64_varqual : public vRNG16 {
					Uint64 state;
					int outshift;
				public:
					xlcg16of64_varqual(int lcg1_discard_bits) : outshift(lcg1_discard_bits) {}
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class xlcg8of64_varqual : public vRNG8 {
					Uint64 state;
					int outshift;
				public:
					xlcg8of64_varqual(int lcg1_discard_bits) : outshift(lcg1_discard_bits) {}
					Uint8 raw8();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				//modified LCG combined with regular LCG
				class cxlcg8of96_varqual : public vRNG8 {
					Uint64 lcg1;
					Uint32 lcg2;
					int outshift;
				public:
					cxlcg8of96_varqual(int lcg1_discard_bits) : outshift(lcg1_discard_bits) {}
					Uint8 raw8();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class cxlcg16of96_varqual : public vRNG16 {
					Uint64 lcg1;
					Uint32 lcg2;
					int outshift;
				public:
					cxlcg16of96_varqual(int lcg1_discard_bits) : outshift(lcg1_discard_bits) {}
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class cxlcg32of96_varqual : public vRNG32 {
					Uint64 lcg1;
					Uint32 lcg2;
					int outshift;
				public:
					cxlcg32of96_varqual(int lcg1_discard_bits) : outshift(lcg1_discard_bits) {}
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};


				//
				class garthy16 : public vRNG16 {
					Uint16 value, scale, counter;
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class garthy32 : public vRNG32 {
					Uint32 value, scale, counter;
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				//both sides of the multiply are pseudo-random values in this RNG
				class binarymult16 : public vRNG16 {
					Uint16 a, b, c, d;
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class binarymult32 : public vRNG32 {
					Uint32 a, b, c, d;
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};

				//uses multiplication, rightshifts, xors, that kind of stuff
				class rxmult16 : public vRNG16 {
					Uint16 a, b, c, d;
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};

				//these are similar to my mwlac algorithm, but lower quality
				class multish2x64 : public vRNG64 {
					Uint64 a, b;
					Uint64 raw64();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class multish3x32 : public vRNG32 {
					Uint32 a, b, c;
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class multish4x16 : public vRNG16 {
					Uint16 a, b, c, d;
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};

				//the 16 bit variant of the old version of my mwlac algorithm
				class old_mwlac16 : public vRNG16 {
					Uint16 a, b, c, d;
					Uint16 raw16();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};

				class cxm64_varqual : public vRNG64 {
					Uint64 low, high;
					int num_mult;
				public:
					cxm64_varqual(int num_mult_) : num_mult(num_mult_) {}
					Uint64 raw64();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};

				class mo_Cmfr32 : public vRNG32 {
					Uint32 state;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
				class mo_Cmr32 : public vRNG32 {
					Uint32 state;
				public:
					Uint32 raw32();
					std::string get_name() const;
					void walk_state(StateWalkingObject *);
				};
			}
		}
	}
}
