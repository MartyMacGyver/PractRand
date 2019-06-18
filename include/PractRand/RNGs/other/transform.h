
#include <deque>
//RNGs in the "other" directory are not intended for real world use
//only for research; as such they may get pretty sloppy in some areas
//and are usually not optimized
namespace PractRand {
	namespace RNGs {
		namespace Polymorphic {
			namespace NotRecommended {
				class Transform64 : public vRNG64 {
				public:
					vRNG *base_rng;
					void seed(Uint64 seed);
					Uint64 get_flags() const;
					void walk_state(StateWalkingObject *walker);
					Transform64(vRNG *rng) : base_rng(rng) {}
					~Transform64();
				};
				class Transform32 : public vRNG32 {
				public:
					vRNG *base_rng;
					void seed(Uint64 seed);
					Uint64 get_flags() const;
					void walk_state(StateWalkingObject *walker);
					Transform32(vRNG *rng) : base_rng(rng) {}
					~Transform32();
				};
				class Transform16 : public vRNG16 {
				public:
					vRNG *base_rng;
					void seed(Uint64 seed);
					Uint64 get_flags() const;
					void walk_state(StateWalkingObject *walker);
					Transform16(vRNG *rng) : base_rng(rng) {}
					~Transform16();
				};
				class Transform8 : public vRNG8 {
				public:
					vRNG *base_rng;
					void seed(Uint64 seed);
					Uint64 get_flags() const;
					void walk_state(StateWalkingObject *walker);
					Transform8(vRNG *rng) : base_rng(rng) {}
					~Transform8();
				};

				class GeneralizedTableTransform : public vRNG8 {//written for self-shrinking-generators, but also useful for other transforms
				public:
					struct Entry {
						Uint8 data;
						Uint8 count;
					};
					//the transform table
					const Entry *table;

					//for buffering fractional bytes of output
					Uint32 buf_data;
					Uint32 buf_count;

					//for buffering full bytes of output
					std::deque<Uint8> finished_bytes;

					std::string name;
					vRNG *base_rng;
					void seed(Uint64 seed);
					Uint64 get_flags() const;
					std::string get_name() const ;
					GeneralizedTableTransform(vRNG *rng, const Entry *table_, std::string name_) : base_rng(rng), table(table_), buf_count(0), buf_data(0), name(name_) {}
					~GeneralizedTableTransform();
					void walk_state(StateWalkingObject *);
					Uint8 raw8();
				};
				vRNG *apply_SelfShrinkTransform(vRNG *base_rng);
				//vRNG *apply_SimpleShrinkTransform(vRNG *base_rng);

				class ReinterpretAsUnknown : public Transform8 {
					Uint8 *buffer;//don't feel like requiring a header for TestBlock
					int index;
					void refill();
				public:
					ReinterpretAsUnknown( vRNG *rng );
					~ReinterpretAsUnknown();
					Uint8 raw8();
					//to do: fix endianness issues
					std::string get_name() const;
					virtual int get_native_output_size() const {return -1;}
				};
				class ReinterpretAs8 : public Transform8 {
					Uint8 *buffer;//don't feel like requiring a header for TestBlock
					int index;
					void refill();
				public:
					ReinterpretAs8( vRNG *rng );
					~ReinterpretAs8();
					Uint8 raw8();
					std::string get_name() const;
				};
				class ReinterpretAs16 : public Transform16 {
					Uint16 *buffer;
					int index;
					void refill();
				public:
					ReinterpretAs16( vRNG *rng );
					~ReinterpretAs16();
					Uint16 raw16();
					std::string get_name() const;
				};
				class ReinterpretAs32 : public Transform32 {
					Uint32 *buffer;
					int index;
					void refill();
				public:
					ReinterpretAs32( vRNG *rng );
					~ReinterpretAs32();
					Uint32 raw32();
					std::string get_name() const;
				};
				class ReinterpretAs64 : public Transform64 {
					Uint64 *buffer;
					int index;
					void refill();
				public:
					ReinterpretAs64( vRNG *rng );
					~ReinterpretAs64();
					Uint64 raw64();
					std::string get_name() const;
				};

				class Discard16to8 : public Transform8 {
					typedef Uint16 InWord;
					typedef Uint8 OutWord;
					enum { INPUT_BITS = sizeof(InWord)* 8 };
					InWord *buffer;
					int index;
					void refill();
				public:
					Discard16to8(vRNG *rng);
					Uint8 raw8();
					std::string get_name() const;
				};
				class Discard32to8 : public Transform8 {
					typedef Uint32 InWord;
					typedef Uint8 OutWord;
					enum { INPUT_BITS = sizeof(InWord)* 8 };
					InWord *buffer;
					int index;
					void refill();
				public:
					Discard32to8(vRNG *rng);
					Uint8 raw8();
					std::string get_name() const;
				};
				class Discard64to8 : public Transform8 {
					typedef Uint64 InWord;
					typedef Uint8 OutWord;
					enum { INPUT_BITS = sizeof(InWord)* 8 };
					InWord *buffer;
					int index;
					void refill();
				public:
					Discard64to8(vRNG *rng);
					Uint8 raw8();
					std::string get_name() const;
				};
				class Discard32to16 : public Transform16 {
					typedef Uint32 InWord;
					typedef Uint16 OutWord;
					enum { INPUT_BITS = sizeof(InWord)* 8 };
					InWord *buffer;
					int index;
					void refill();
				public:
					Discard32to16(vRNG *rng);
					Uint16 raw16();
					std::string get_name() const;
				};
				class Discard64to16 : public Transform16 {
					typedef Uint64 InWord;
					typedef Uint16 OutWord;
					enum { INPUT_BITS = sizeof(InWord)* 8 };
					InWord *buffer;
					int index;
					void refill();
				public:
					Discard64to16(vRNG *rng);
					Uint16 raw16();
					std::string get_name() const;
				};
				class Discard64to32 : public Transform32 {
					typedef Uint64 InWord;
					typedef Uint32 OutWord;
					enum { INPUT_BITS = sizeof(InWord)* 8 };
					InWord *buffer;
					int index;
					void refill();
				public:
					Discard64to32(vRNG *rng);
					Uint32 raw32();
					std::string get_name() const;
				};

				class BaysDurhamShuffle64 : public Transform64 {
					Uint64 table[256];
					Uint8 prev;
					Uint8 index_mask;
					Uint8 index_shift;
				public:
					Uint64 raw64();
					void seed(Uint64 s);
					void walk_state(StateWalkingObject *);
					std::string get_name() const ;
					BaysDurhamShuffle64(vRNG64 *rng, int table_size_L2, int shift=0) 
						: Transform64(rng), index_mask((1<<table_size_L2)-1), index_shift(shift) {}
				};
				class BaysDurhamShuffle32 : public Transform32 {
					Uint32 table[256];
					Uint8 prev;
					Uint8 index_mask;
					Uint8 index_shift;
				public:
					Uint32 raw32();
					void seed(Uint64 s);
					void walk_state(StateWalkingObject *);
					std::string get_name() const ;
					BaysDurhamShuffle32(vRNG32 *rng, int table_size_L2, int shift=0) 
						: Transform32(rng), index_mask((1<<table_size_L2)-1), index_shift(shift) {}
				};
				class BaysDurhamShuffle16 : public Transform16 {
					Uint16 table[256];
					Uint8 prev;
					Uint8 index_mask;
					Uint8 index_shift;
				public:
					Uint16 raw16();
					void seed(Uint64 s);
					void walk_state(StateWalkingObject *);
					std::string get_name() const ;
					BaysDurhamShuffle16(vRNG16 *rng, int table_size_L2, int shift=0) 
						: Transform16(rng), index_mask((1<<table_size_L2)-1), index_shift(shift) {}
				};
				class BaysDurhamShuffle8 : public Transform8 {
					Uint8 table[256];
					Uint8 prev;
					Uint8 index_mask;
					Uint8 index_shift;
				public:
					Uint8 raw8();
					void seed(Uint64 s);
					void walk_state(StateWalkingObject *);
					std::string get_name() const ;
					BaysDurhamShuffle8(vRNG8 *rng, int table_size_L2, int shift=0) 
						: Transform8(rng), index_mask((1<<table_size_L2)-1), index_shift(shift) {}
				};
				vRNG *apply_BaysDurhamShuffle(vRNG *base_rng, int table_size_L2=8, int shift=-1);
			}
		}
	}
}
