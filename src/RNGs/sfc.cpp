#include <string>
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_helpers.h"
#include "PractRand/rng_internals.h"

#include "PractRand/RNGs/sfc16.h"
#include "PractRand/RNGs/sfc32.h"
#include "PractRand/RNGs/sfc64.h"

using namespace PractRand;

//polymorphic:
PRACTRAND__POLYMORPHIC_RNG_BASICS_C64(sfc64)
void PractRand::RNGs::Polymorphic::sfc64::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::sfc64::seed_fast(Uint64 s) {implementation.seed_fast(s);}
void PractRand::RNGs::Polymorphic::sfc64::seed(Uint64 s1, Uint64 s2, Uint64 s3) {implementation.seed(s1,s2,s3);}
std::string PractRand::RNGs::Polymorphic::sfc64::get_name() const {return "sfc64";}

PRACTRAND__POLYMORPHIC_RNG_BASICS_C32(sfc32)
void PractRand::RNGs::Polymorphic::sfc32::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::sfc32::seed_fast(Uint64 s) {implementation.seed_fast(s);}
void PractRand::RNGs::Polymorphic::sfc32::seed(Uint32 s1, Uint32 s2, Uint32 s3) {implementation.seed(s1,s2,s3);}
std::string PractRand::RNGs::Polymorphic::sfc32::get_name() const {return "sfc32";}

PRACTRAND__POLYMORPHIC_RNG_BASICS_C16(sfc16)
void PractRand::RNGs::Polymorphic::sfc16::seed(Uint64 s) {implementation.seed(s);}
void PractRand::RNGs::Polymorphic::sfc16::seed_fast(Uint64 s) {implementation.seed_fast(s);}
void PractRand::RNGs::Polymorphic::sfc16::seed(Uint16 s1, Uint16 s2, Uint16 s3) {implementation.seed(s1,s2,s3);}
std::string PractRand::RNGs::Polymorphic::sfc16::get_name() const {return "sfc16";}

//raw:
Uint16 PractRand::RNGs::Raw::sfc16::raw16() {
	enum {BARREL_SHIFT = 6, RSHIFT = 5, LSHIFT = 3};//good sets include {4,3,2},{6,5,2},{4,5,3},{6,5,3},{7,5,3}; older versions used {7,3,2}
	Uint16 tmp = a + b + counter++;
	a = b ^ (b >> RSHIFT);
	b = c + (c << LSHIFT);
	c = ((c << BARREL_SHIFT) | (c >> (16-BARREL_SHIFT))) + tmp;
	return tmp;
}
void PractRand::RNGs::Raw::sfc16::seed(Uint64 s) {
	a = Uint16(s);
	b = Uint16(s >> 16);
	c = Uint16(s >> 32);
	counter = Uint16(s >> 48);
	for (int i = 0; i < 10; i++) raw16();//10
/*
_				e0f0	e0f1	e0f2		e1f0	e1f1	e1f2	732	632	532	432	332	832	932	A32	433	633	343	443	543	643	743	843	943	242	342	442	542	642	742	842	942	344	544	644	744	252	352	452	552	652	752	852	952	253	353	453	553	653	753	853	953
sfc	16	5		?		?		?			?		14		?		17	15	14	15	14	14	14	14																																		14
sfc	16	6		?		?		?			?		25		?		20	22	17	25	22	18	19	17	20	20	21	22	22	21	20	20	19	22	23	23	23	22	23	19	21	20	20	21	20	25	21	23	26	26	24	22	21	25	24	27	22	25	27	23	22
sfc 16  7		?		?		?			?		33		?		26	29	29	29	24	24	28	25	24	24	28	29	29	32	30	27	27	23	27	29	29	26	28	23	26	26	23	27	24-	27	26	30	29	31	31	26	28	27	30	31	31	33	31	26	30
sfc 16  8		?		?		?			?		37+		?		32	35	33	38?										33																				37?								37+			.
_													47,50,53,56			*		**			.				.	*	*	**	*	.			*	*	*	.	*		.			.		*	.	**	**	***	**	.	.	*	**	***	**	***	***	.	*
_																	78	86	79	92?										86																				94?								95+			.
_		8/10/10
*/
}
void PractRand::RNGs::Raw::sfc16::seed_fast(Uint64 s) {
	a = Uint16(s);
	b = Uint16(s >> 16);
	c = Uint16(s >> 32);
	counter = Uint16(s >> 48);
	for (int i = 0; i < 8; i++) raw16();
}
void PractRand::RNGs::Raw::sfc16::seed(Uint16 s1, Uint16 s2, Uint16 s3) {
	a = s1;
	b = s2;
	c = s3;
	counter = 1;
	for (int i = 0; i < 10; i++) raw16();
}
void PractRand::RNGs::Raw::sfc16::walk_state(StateWalkingObject *walker) {
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
	walker->handle(counter);
}
Uint32 PractRand::RNGs::Raw::sfc32::raw32() {
	enum {BARREL_SHIFT = 21, RSHIFT = 9, LSHIFT = 3};//good sets include {21,9,3},{15,8,3}; older versions used {25,8,3} which wasn't as good
	Uint32 tmp = a + b + counter++;
	a = b ^ (b >> RSHIFT);
	b = c + (c << LSHIFT);
	c = ((c << BARREL_SHIFT) | (c >> (32-BARREL_SHIFT))) + tmp;
	return tmp;
}
void PractRand::RNGs::Raw::sfc32::seed(Uint64 s) {
	a = 0;//a gets mixed in the slowest
	b = Uint32(s >> 0);
	c = Uint32(s >> 32);
	counter = 1;
	for (int i = 0; i < 12; i++) raw32();//12
/*
_				e0f0	e0f1	e0f2		e1f0	e1f1	e1f2
sfc 16	5											14
sfc 16	6											25
sfc 16	7											33
sfc 16	8											37+
		8/10/10

sfc	32	4											12
sfc	32	5											12
sfc	32	6											21
sfc	32	7											32
sfc	32	8											36?
		8/12/15

sfc 64  5											13
sfc 64  6											16
sfc 64  7											29
sfc 64  8											39+
		8/12/18

jsf	32	2											?		10
jsf	32	3											14		12
jsf	32	4											19		18
jsf	32	5									24		24		23
jsf	32	6													35
jsf	32	7													42+
	*/
}
void PractRand::RNGs::Raw::sfc32::seed_fast(Uint64 s) {
	a = 0;
	b = Uint32(s >> 0); 
	c = Uint32(s >> 32);
	counter = 1;
	for (int i = 0; i < 8; i++) raw32();//8
}
void PractRand::RNGs::Raw::sfc32::seed(Uint32 s1, Uint32 s2, Uint32 s3) {
	a = s1;
	b = s2;
	c = s3;
	counter = 1;
	for (int i = 0; i < 15; i++) raw32();
}
void PractRand::RNGs::Raw::sfc32::walk_state(StateWalkingObject *walker) {
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
	walker->handle(counter);
}
Uint64 PractRand::RNGs::Raw::sfc64::raw64() {
	enum {BARREL_SHIFT = 24, RSHIFT = 11, LSHIFT = 3};//good sets include {24,11,3},{25,12,3},{},{} ; older versions used {25,12,3}, which is decent
	Uint64 tmp = a + b + counter++;
	a = b ^ (b >> RSHIFT);
	b = c + (c << LSHIFT);
	c = ((c << BARREL_SHIFT) | (c >> (64-BARREL_SHIFT))) + tmp;
	return tmp;
}
void PractRand::RNGs::Raw::sfc64::seed(Uint64 s) {
	a = b = c = s;
	counter = 1;
	for (int i = 0; i < 12; i++) raw64();//12
	//16 outputs skipped - sfc64 is held to a higher standards for seeding than sfc32 because it is rated for more parallel scenarios
	//no wait, it's rated the same as sfc32, there's no reason for 16 rounds, stick to 12
/*
_																	X,12,3					X,11,3								X,13,3					24,X,3						
_				e0f0	e0f1	e0f2		e1f0	e1f1	e1f2	25	23	21	10	37	46	10	16	21	22	24	25	36	37	46	10	30	44	45	46	47	8	9	10	11	27	29	30	40,18,3
sfc	64	5		?		?		?			?		13		?		13	14	12	9	13	13	9	12	13	12	14	14	12	12	13	9	14	13	14	13	12	12	13	14	.	12	13	13	13	.
sfc	64	6		?		?		?			?		16		?		18	18	18	18	19	17	18	18	20	14	16	16	18	19	16	16	17	18	17	17	14	14	13	14	.	17	17	15	18	.
sfc 64	7		?		?		?			?		29		?		25	26	26	25	26	27	27	26	26	27	29	25	25	28	25	26	26	27	26	26	25	27	25	25	.	25	26	25	28	.
sfc 64	8		?		?		?			?		39+		?		40	36	37	22	29	32	22				39+			30									37		.				*	.
_																	83	80	81	65	74	76	67		*		84+			77				*					75		.					.
_																	
_		8/12/18
*/
}
void PractRand::RNGs::Raw::sfc64::seed_fast(Uint64 s) {
	a = b = c = s;
	counter = 1;
	for (int i = 0; i < 8; i++) raw64();
}
void PractRand::RNGs::Raw::sfc64::seed(Uint64 s1, Uint64 s2, Uint64 s3) {
	a = s1;
	b = s2;
	c = s3;
	counter = 1;
	for (int i = 0; i < 18; i++) raw64();
}
void PractRand::RNGs::Raw::sfc64::walk_state(StateWalkingObject *walker) {
	walker->handle(a);
	walker->handle(b);
	walker->handle(c);
	walker->handle(counter);
}


