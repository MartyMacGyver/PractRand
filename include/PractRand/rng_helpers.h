#ifndef _practrand_rng_helpers_h
#define _practrand_rng_helpers_h

#ifndef __PRACTRAND_CONFIG_H__
#include "PractRand/config.h"
#endif //__PRACTRAND_CONFIG_H__

namespace PractRand {
	class StateWalkingObject {
	public:
		virtual ~StateWalkingObject() {}
		virtual void handle(bool  &) = 0;
		/*virtual void handle(unsigned char &) = 0;
		virtual void handle(unsigned short &) = 0;
		virtual void handle(unsigned int &) = 0;
		virtual void handle(unsigned long &) = 0;
		virtual void handle(unsigned long long &) = 0;*/
		virtual void handle(Uint8 &) = 0;
		virtual void handle(Uint16&) = 0;
		virtual void handle(Uint32&) = 0;
		virtual void handle(Uint64&) = 0;

		virtual void handle(float &) = 0;
		virtual void handle(double &) = 0;
		//virtual void handle(void *&) = 0;

		//purposes: 
		// 1. seeding
		// 2. serialization (serialize, deserialize, measure state size)
		// 3. avalanche style testing (measure state size / props, tweak bits in state, etc)
		// 4. ???
		enum {
			FLAG_READ_ONLY = 1, //does not make changes
			FLAG_CLUMSY = 2,    //may violate invariants (if also FLAG_READ_ONLY then only wants to see state visible to clumsy writers)
			FLAG_SEEDER = 4,    //some RNGs may have extra invariants enforced only on seeded states (minimum distance away from other seeded states on cycle)
		};
		virtual Uint32 get_properties() const = 0;
		bool is_read_only() const {return (get_properties() & FLAG_READ_ONLY) ? true : false;}
		bool is_clumsy() const {return (get_properties() & FLAG_CLUMSY) ? true : false;}
		bool is_seeder() const {return (get_properties() & FLAG_SEEDER) ? true : false;}

		/*void handle(signed char      &v) {handle((unsigned char)v);}
		void handle(signed short     &v) {handle((unsigned short)v);}
		void handle(signed int       &v) {handle((unsigned int)v);}
		void handle(signed long      &v) {handle((unsigned long)v);}
		void handle(signed long long &v) {handle((unsigned long long)v);}*/
		void handle(Sint8 &v) {handle((Uint8 &)v);}
		void handle(Sint16&v) {handle((Uint16&)v);}
		void handle(Sint32&v) {handle((Uint32&)v);}
		void handle(Sint64&v) {handle((Uint64&)v);}

		StateWalkingObject &operator<<(Uint8 &v) {handle(v);return *this;}
		StateWalkingObject &operator<<(Uint16&v) {handle(v);return *this;}
		StateWalkingObject &operator<<(Uint32&v) {handle(v);return *this;}
		StateWalkingObject &operator<<(Uint64&v) {handle(v);return *this;}
		StateWalkingObject &operator<<(Sint8 &v) {handle(v);return *this;}
		StateWalkingObject &operator<<(Sint16&v) {handle(v);return *this;}
		StateWalkingObject &operator<<(Sint32&v) {handle(v);return *this;}
		StateWalkingObject &operator<<(Sint64&v) {handle(v);return *this;}
		StateWalkingObject &operator<<(float  &v){handle(v);return *this;}
		StateWalkingObject &operator<<(double &v){handle(v);return *this;}
	};
	namespace RNGs {
		class vRNG;
	}
	Uint32 randi_fast_implementation(Uint32 random_value, Uint32 max);
	StateWalkingObject *int_to_rng_seeder(Uint64);//must be deleted after use
	StateWalkingObject *vrng_to_rng_seeder(RNGs::vRNG *);//must be deleted after use
	StateWalkingObject *get_autoseeder(const void *);//must be deleted after use
}
#define PRACTRAND__POLYMORPHIC_RNG_BASICS_H(RNG) public:\
		enum {OUTPUT_TYPE = OUTPUT_TYPES::NORMAL_ALL,OUTPUT_BITS = Raw:: RNG ::OUTPUT_BITS,FLAGS = Raw:: RNG ::FLAGS};\
		Raw:: RNG implementation;\
		RNG (Uint64 s) {seed(s);}\
		RNG (vRNG *seeder) {seed(seeder);}\
		RNG (SEED_AUTO_TYPE ) {autoseed();}\
		RNG (SEED_NONE_TYPE ) {}\
		Uint8  raw8 ();\
		Uint16 raw16();\
		Uint32 raw32();\
		Uint64 raw64();\
		using vRNG::seed;\
		Uint64 get_flags() const;\
		std::string get_name() const;\
		void walk_state(StateWalkingObject *walker);

#if defined PRACTRAND_NO_LIGHT_WEIGHT_RNGS
#define PRACTRAND__LIGHT_WEIGHT_RNG(RNG)
#define PRACTRAND__LIGHT_WEIGHT_ENTROPY_POOL(RNG)
#else // ! PRACTRAND_NO_LIGHT_WEIGHT_RNGS
#ifndef __PRACTRAND_RNG_ADAPTORS_H__
#include "rng_adaptors.h"
#endif//__PRACTRAND_RNG_ADAPTORS_H__
#define PRACTRAND__LIGHT_WEIGHT_RNG(RNG) 	\
	namespace LightWeight {\
		typedef PractRand::RNGs::Adaptors::RAW_TO_LIGHT_WEIGHT_RNG<PractRand::RNGs::Raw:: RNG > RNG;\
	}
#endif//PRACTRAND_PROVIDE_LIGHT_WEIGHT_RNGS


#endif //_practrand_rng_helpers_h
