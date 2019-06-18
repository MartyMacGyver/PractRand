namespace Candidates {


/*
Canidates currently under consideration:


1. a variant of Ranrot (RanrotVariant64/32/16/8)
	OVERALL EVALUTION: no close competitors atm, but no clear need for it either;
		and a reduced-size version of efiix might be superior anyway
	niche: medium speed, medium quality, medium size
		particularly for 8 bit, which currently has nothing smaller than efiix8x384
		would be the smallest / lightest weight buffered RNG, for whatever that's worth
	speed: occasionally faster than any recommended RNG, usually only middling though
	statistical quality: decent at word sizes past 16 bit, poor @ 8 & 16 bit
		no short term correlation, and the long term correlation seems to be under control
		but the medium term correlation, while not detected @ 32 or 64 bit, is definitely 
		there
	portability: uses only simple operations
	word sizes: viable at 8, 16, 32, & 64 bit ; quality suffers @ 8 & 16 bit though
	full word output: yes
	buffered: yes
	random access: no
	cryptographic security: none
	short cycle protection: limited @ 8 & 16 bit, good @ 32 & 64 bit
	cycle type: reversible multicyclic
	size: 48 words
	statespace: full on 47 words, 45 states on the last word
		(45 * 2**376 @ 8 bit, 45 * 2**752 @ 16 bit, 45 * 2**1504 @ 32 bit, 45 * 2**3008 @ 64 bit)

2. very fast RNG (VeryFast64/32/16)
	OVERALL EVALUTION: insufficiently different from sfc64/32/16
	niche: 
		fastest inline RNG of reasonable quality
		with only 2-3 words of state, 1 temp var, and six or so ops, it's cheap to inline
		lack of a counter variable is bad at small sizes though - too easy to wind up in a bad cycle
		and adding a counter variable makes it little different from sfc or the older versions of sfc
	speed: often faster than any recommended RNG, generally at least competitive with the fastest
	statistical quality: decent or better @ 32 bit & 64 bit, poor @ 16 bit
	portability: uses only simple operations
	word sizes: viable at 16, 32, & 64 bit (some variants are marginal at 16 bit)
	full word output: yes
	buffered: no
	random access: no
	cryptographic security: none
	short cycle protection: none
	cycle type: reversible multicyclic (also considering some irreversible variants)
	size: 3 words
	statespace: full on all 3 words, except for the all-zeroes case
		(2**48-1 @ 16 bit, 2**96-1 @ 32 bit, 2**192-1 @ 64 bit)

3. possible revisions to the current sfc RNG (sfc_alternative64/32/16)
	OVERALL EVALUTION: hard to justify
	niche: similar to sfc - 
		small size, fast speed, guaranteed cycle length at some word sizes
		this version substantially improves quality
		but... one major use of sfc is as an inline RNG
			in that context, the fact that it takes just 3 registers plus 1 temp register is nice
			this expands that to 4 registers plus 1 temp register, the same as jsf
	speed: fast to very fast - sometimes appears faster than sfc, usually appears faster than jsf
	statistical quality: better than jsf
	portability: uses only simple operations
	word sizes: viable at 16, 32, & 64 bit; almost viable at 8 bit
	full word output: yes
	buffered: no
	random access: no
	cryptographic security: none
	short cycle protection: limited
	cycle type: reversible multicyclic
	size: 4 words
	statespace: full on all 4 words
		(2**64 @ 16 bit, 2**128 @ 32 bit, 2**256 @ 64 bit)


The current sfc occupies three distinct niches:
	1. general small fast RNG
	2. RNGs that are good for inlining due to small state, few ops, few temp vars, and high speed
	3. RNGs that combine the general strengths of small fast RNGs with a guaranteed minimum cycle length
For #1, sfc_alternative is probably better than sfc, plus jsf is comparable and already a recommended RNG.  
For #2, VeryFast might be better than the current sfc, at least at 32 & 64 bit.  
For #3, sfc_alternative looks better than the current sfc.  

*/
#define POLYMORPHIC_CANDIDATE(rng, bits) \
class polymorphic_ ## rng ## bits : public PractRand::RNGs::vRNG ## bits {\
public:\
	typedef raw_ ## rng ## bits ImplementationType;\
	enum {OUTPUT_BITS = ImplementationType ::OUTPUT_BITS,FLAGS = ImplementationType ::FLAGS};\
	polymorphic_ ## rng ## bits (PractRand::SEED_NONE_TYPE) {}\
	polymorphic_ ## rng ## bits (PractRand::SEED_AUTO_TYPE) {autoseed();}\
	polymorphic_ ## rng ## bits (Uint64 seed_value) {seed(seed_value);}\
	ImplementationType implementation;\
	Uint ## bits raw ## bits () {return implementation.raw ## bits ();}\
	void walk_state(StateWalkingObject *walker) {implementation.walk_state(walker);}\
	std::string get_name() const {return std::string(#rng) + #bits ;}\
};
#define SEEDED_POLYMORPHIC_CANDIDATE(rng, bits) \
class polymorphic_ ## rng ## bits : public PractRand::RNGs::vRNG ## bits {\
public:\
	typedef raw_ ## rng ## bits ImplementationType;\
	enum {OUTPUT_BITS = ImplementationType ::OUTPUT_BITS,FLAGS = ImplementationType ::FLAGS};\
	ImplementationType implementation; \
	polymorphic_ ## rng ## bits(PractRand::SEED_NONE_TYPE) {}\
	polymorphic_ ## rng ## bits (PractRand::SEED_AUTO_TYPE) {autoseed();}\
	polymorphic_ ## rng ## bits (Uint64 seed_value) {implementation.seed(seed_value);}\
	void seed(Uint64 seed_value) {implementation.seed(seed_value);}\
	Uint ## bits raw ## bits () {return implementation.raw ## bits ();}\
	void walk_state(StateWalkingObject *walker) {implementation.walk_state(walker);}\
	std::string get_name() const {return std::string(#rng) + #bits ;}\
};

template<typename Word, int LAG1, int LAG2, int SHIFT1, int SHIFT2, int SHIFT3>
class _RanrotVariant {
public:
	enum {
		OUTPUT_TYPE = PractRand::RNGs::OUTPUT_TYPES::NORMAL_1,
		OUTPUT_BITS = 8 * sizeof(Word),
		FLAGS = PractRand::RNGs::FLAG::NEEDS_GENERIC_SEEDING
	};
	Word buffer[LAG1], position, counter1, counter2, extra;
	static Word rotate(Word value, int bits) {return (value << bits) | (value >> (8*sizeof(value)-bits));}
	Word step( Word newest, Word oldest, Word middle ) {
		/*newest ^= middle + (middle << 3);
		extra ^= (rotate(newest, SHIFT3) + extra);
		return extra + oldest;*/
		return (rotate(oldest, SHIFT1) ^ rotate(middle, SHIFT2)) + rotate(newest, SHIFT3);
		//return (rotate(oldest, SHIFT1) ^ rotate(middle, SHIFT2)) + (rotate(newest, SHIFT3) ^ prior);
		//newest += newest << 2; newest ^= newest >> 3; return current + other + newest;
	}
	Word refill() {
		Word prior = buffer[LAG1-1] ^ counter1++;
		for (int i = 0; i < LAG2; ) {
			prior = buffer[i] = step(prior, buffer[i], buffer[i+LAG1-LAG2]); i++;
			prior = buffer[i] = step(prior, buffer[i], buffer[i+LAG1-LAG2]); i++;
			prior = buffer[i] = step(prior, buffer[i], buffer[i+LAG1-LAG2]); i++;
		}
		counter2 += counter1 ? 0 : 1; prior ^= counter2;
		for (int i = LAG2; i < LAG1; ) {
			prior = buffer[i] = step(prior, buffer[i], buffer[i     -LAG2]); i++;
			prior = buffer[i] = step(prior, buffer[i], buffer[i     -LAG2]); i++;
			prior = buffer[i] = step(prior, buffer[i], buffer[i     -LAG2]); i++;
		}
		position = LAG1 - 1;
		return buffer[position];
	}
	Word _raw_native() {
		if (position) return buffer[--position];
		//refill();
		return refill();
	}
	void walk_state(StateWalkingObject *walker) {
		for (int i = 0; i < LAG1; i++) walker->handle(buffer[i]);
		walker->handle(position);
		walker->handle(counter1);
		walker->handle(counter2);
		if (position > LAG1) position = 0;
	}
};
class raw_ranrot_variant64 : public _RanrotVariant<Uint64,45,24,0,0,29> {public: Uint64 raw64() {return _raw_native();}};
class raw_ranrot_variant32 : public _RanrotVariant<Uint32,45,24,0,0,13> {public: Uint32 raw32() {return _raw_native();}};
class raw_ranrot_variant16 : public _RanrotVariant<Uint16,45,24,0,0, 5> {public: Uint16 raw16() {return _raw_native();}};
class raw_ranrot_variant8  : public _RanrotVariant<Uint8 ,45,24,0,0, 3> {public: Uint8  raw8 () {return _raw_native();}};
POLYMORPHIC_CANDIDATE(ranrot_variant, 64)
POLYMORPHIC_CANDIDATE(ranrot_variant, 32)
POLYMORPHIC_CANDIDATE(ranrot_variant, 16)
POLYMORPHIC_CANDIDATE(ranrot_variant,  8)

template<typename Word, int ROTATE, int RSHIFT, int LSHIFT>
class _VeryFast {
public:
	enum {
		OUTPUT_TYPE = PractRand::RNGs::OUTPUT_TYPES::NORMAL_1,
		OUTPUT_BITS = 8 * sizeof(Word),
		FLAGS = PractRand::RNGs::FLAG::NEEDS_GENERIC_SEEDING
	};
	Word a, b, c;
	Word table[256];
	static Word rotate(Word value, int bits) {return ((value << bits) | (value >> (OUTPUT_BITS - bits)));}
	_VeryFast() {
		PractRand::RNGs::Polymorphic::hc256 good(PractRand::SEED_AUTO);
		for (int i = 0; i < 256; i++) table[i] = Word(good.raw64());
	}
	Word _raw_native() {
		Word old;
		const Word K = Word(0x92ec64765925a395ull);
		//now based upon alearx:
		a ^= rotate(b + c, LSHIFT);
		b ^= rotate(c + (c << 3), RSHIFT);
		c ^= a + (a << 3); c = rotate(c, ROTATE);
		return a;//*/
		//good speed, 16 bit version fails @ 32 GB, 32 bit version passed 8 TB
		/*old = a + b;
		a = b ^ (b >> RSHIFT);
		b = c + (c << LSHIFT);
		c = old + rotate(c,ROTATE);// RSHIFT,LSHIFT,ROTATE : 7,3,9 @ 32 bit
		return old;//*/
		//best quality: 16 bit fails @ 1 TB, but not as fast ;; switching "a += b ^ c;" for "a ^= b + c;" increases that to 2 TB
		/*old = a + (a << LSHIFT);
		a += b ^ c;
		b = c ^ (c >> RSHIFT);
		c = old + rotate(c,ROTATE);
		return old;//*/
		//faster, simpler, lower quality - just 4-6 ops, very few dependent
		//16 bit: 128 MB, 32 bit: 32 GB
		/*old = a + b;
		a = b + rotate(c,ROTATE);
		b = c + (c << LSHIFT);
		c = old;
		return c;//*/
		//another alternative
		//16 bit: 1 GB, 32 bit: 2 TB
		/*old = a + b;
		a = b;
		b = c + (c << LSHIFT);
		c = rotate(c, ROTATE);
		a += c;
		c += old;
		return a;//*/
		//uses multiplication, only 2 words, but pretty good asside from that:
		//16: 1 GB, 32 bit: > 32 TB
		/*old = a * Word(0x92ec64765925a395ull);
		a = b ^ rotate(a, OUTPUT_BITS/2);
		b = old;
		return a+b;//*/
		/*old = a * Word(0x92ec64765925a395ull);
		a = rotate(a, OUTPUT_BITS/2) ^ b ^ c++;
		b = old;
		return a;*/
		/*old = (a ^ (a >> (OUTPUT_BITS/2)));
		//c += (c << 3) + 1;
		a += b + (b << 3);
		b ^= old + ++c;
		return a;*/
	}
	void walk_state(StateWalkingObject *walker) {
		walker->handle(a);
		walker->handle(b);
		walker->handle(c);
	}
};
class raw_VeryFast64 : public _VeryFast<Uint64,29,9,3> {public: Uint64 raw64() {return _raw_native();}};
class raw_VeryFast32 : public _VeryFast<Uint32,13,7,3> {public: Uint32 raw32() {return _raw_native();}};
class raw_VeryFast16 : public _VeryFast<Uint16, 7,3,2> {public: Uint16 raw16() {return _raw_native();}};
class raw_VeryFast8  : public _VeryFast<Uint8 , 3,2,2> {public: Uint8  raw8 () {return _raw_native();}};
POLYMORPHIC_CANDIDATE(VeryFast, 64)
POLYMORPHIC_CANDIDATE(VeryFast, 32)
POLYMORPHIC_CANDIDATE(VeryFast, 16)
POLYMORPHIC_CANDIDATE(VeryFast,  8)

template<typename Word, int SHIFT1, int SHIFT2, int SHIFT3>
class _sfc_alternative {
public:
	enum {
		OUTPUT_TYPE = PractRand::RNGs::OUTPUT_TYPES::NORMAL_1,
		OUTPUT_BITS = 8 * sizeof(Word),
		FLAGS = 0//PractRand::RNGs::FLAG::NEEDS_GENERIC_SEEDING
	};
	Word a, b, c, d, counter, counter2;
	static Word rotate(Word value, int bits) {return (value << bits) | (value >> (8*sizeof(value)-bits));}
	static Word nluf(Word value) {
		value *= 9;//15;
		//value += Word(3442999295u);
		//value ^= Word(1702153011u);
		value = rotate(value, OUTPUT_BITS/3+1) ^ value; return value;
	}
	Word _raw_native() {
		//experiment with larger pseudo-counter
		/*++counter;
		counter2 += counter + (counter ? 0 : 1);//2-word LCG
		Word tmp = a + b;//counter2;
		//a = b ^ (b >> SHIFT2);
		//a = b + (b << SHIFT3);
		a = b + counter2;
		b = rotate(b, SHIFT1) + tmp;
		return a;//*/
		//SFC 3:
		/*Word tmp = a + b + counter++;
		a = b ^ (b >> SHIFT2);
		b = rotate(b,SHIFT1) + tmp;
		return tmp;//*/
		//SFC 4, 16 bit version >8 TB (64 GB w/o counter)
		/*Word old = a + b + counter++;//64 GB on counter, 8 TB on b
		a = b ^ (b >> SHIFT2);//128 GB?
		b = c + (c << SHIFT3);//1 TB
		c = old + rotate(c,SHIFT1);//important!
		return old;//*/
		//okay speed, 16 bit version >2 TB (256 GB w/o counter), 32 bit @ ?
		/*Word old = a + (a << SHIFT3);
		a = b + c + counter++;
		b = c ^ (c >> SHIFT2);
		c = rotate(c,SHIFT1) + old;
		return old;//*/
		//too slow, 16 bit version ??? (4 TB w/o counter)
		/*Word old = a + b + counter++;
		a = old ^ rotate(a, SHIFT2);
		b = c + (c << SHIFT3);
		c = old + rotate(c,SHIFT1);
		return old;//*/
		//too slow, 16 bit version ??? (2 TB w/o counter)
		/*Word old = a + (a << SHIFT3);
		a += b ^ c;
		b = c ^ (c >> SHIFT2) ^ counter++;
		c = old + rotate(c,SHIFT1);
		return old;//*/
		//faster, 16 bit version failed @ 64-128 GB (4 GB w/o counter), 32 bit @ ? (passed 16 TB w/o counter)
		/*Word old = a + b;
		a = b + counter++;
		b = c ^ (c >> SHIFT2);
		c = old + rotate(c,SHIFT1);
		return old;//*/
		//good speed, 16 bit version failed @ >512 GB (32 GB w/o counter), 32 bit @ ? (? w/o counter)
		/*Word old = a + b + counter++;
		a = b + (b << SHIFT3);
		b = c ^ (c >> SHIFT2);
		c = old + rotate(c,SHIFT1);
		return old;//*/
		//???
		/*Word old = a + counter++;
		a = rotate(a, 3) ^ (a+b);
		b = rotate(b, 7) ^ (b+c);
		c = rotate(c, 11) ^ (c+old);
		return old^b;*/
		/*a += rotate(a,7);
		b = rotate(b,13) + b + (b<<3);
		c = (c + (c << 7)) ^ rotate(c,11);
		return a^b^c;*/

		//VERY good speed, 16 bit version failed @ 256 GB (2 GB w/o counter), 32 bit @ ?
		/*enum { SHIFT = (OUTPUT_BITS == 64) ? 43 : ((OUTPUT_BITS == 32) ? 23 : ((OUTPUT_BITS == 16) ? 11 : -1)) };//43, 11, 9
		a += b; b -= c;
		c += a; a ^= counter++;
		c = rotate(c, SHIFT);//*/
		//w/ counter	32:29->24, 28->37?, 27->36		16:14->22, 13->23, 12->32, 11->37, 10->37, 9->30, 8->19, 7->30, 6->38, 5->38, 4->29, 3->19, 2->18
		//w/o counter	32:29->  , 28->  , 27->			16:14->17, 13->19, 12->26, 11->30, 10->31, 9->31, 8->17, 7->31, 6->31, 5->31, 4->30, 3->19, 2->17

		//xoroshiro128+:
		/*  //
		Word result = a + b;
		b ^= a;
		a = rotate(a, 55 % OUTPUT_BITS) ^ b ^ (b << (14 % OUTPUT_BITS));
		b = rotate(b, 36 % OUTPUT_BITS);
		return result;//*/

		//??? speed, 16 bit version failed @ ? GB (16 GB w/o counter), 32 bit @ ?
		enum { _SHIFT1 = (OUTPUT_BITS == 64) ? 43 : ((OUTPUT_BITS == 32) ? 23 : ((OUTPUT_BITS == 16) ? 10 : -1)) };
		enum { _SHIFT2 = (OUTPUT_BITS == 64) ? 25 : ((OUTPUT_BITS == 32) ?  9 : ((OUTPUT_BITS == 16) ?  7 : -1)) };


		//a += b; b -= c;
		//c += a; a ^= counter++;
		//b = rotate(b, _SHIFT1);//*/
		//c = rotate(c, _SHIFT2);//*/
		//w/ counter	32:29->24, 28->37?, 27->36		16:14->22, 13->23, 12->32, 11->37, 10->37, 9->30, 8->19, 7->30, 6->38, 5->38, 4->29, 3->19, 2->18
		//w/o counter	32:29->  , 28->  , 27->			16:14->17, 13->19, 12->26, 11->30, 10->31, 9->31, 8->17, 7->31, 6->31, 5->31, 4->30, 3->19, 2->17

		//VERY good speed, 16 bit version failed @ 16 TB (1 TB w/o counter), 32 bit @ > 4 TB w/o counter ; 16 bit version passes gjrand --huge (--??? w/o counter
		/*
		enum { SH1 = (OUTPUT_BITS == 64) ? 48 : ((OUTPUT_BITS == 32) ? 14 : ((OUTPUT_BITS == 16) ? 9 : ((OUTPUT_BITS == 8) ? 5 : -1))) };
		enum { SH2 = (OUTPUT_BITS == 64) ?  3 : ((OUTPUT_BITS == 32) ?  3 : ((OUTPUT_BITS == 16) ? 3 : ((OUTPUT_BITS == 8) ? 2 : -1))) };// using LEA on x86
		a += b; b -= c;
		c += a; //a ^= counter++;
		c = rotate(c, SH1);//cb  with count: ?, 14, 9, ?  ; w/o count: 16, 8, 9, ?
		b += (b << SH2);//ba*/
		//						1	2	3	4	5	6	7	8	9	10	11	12	13	14	15	16	17	18	19	20	21	22	23	24	25	26	27	28	29	30	31	32
		//		16 nocount				19	37	39	40	39	32	39	38	34	34	19			40,39,39,39,38,37,34
		//		16B nocount				19	34	37	37	36	36	41	41	38	34	19			41,41,38,37,37,36,36
		//		16 w/ count		17	18	19	40	42	41	42	43	44	44	44	41	23	22	18
		//								0	3	3	1	3	11	5	6	10	7	4
		//								38	77	81	81	81	75	83	82	78	75	42
		//		16:6					18	13	18	20	22	17	24	21	20	19	14
		//		16:7					20	21	24	23	26	20	26	23	21	20	16				+3.1
		//		16:8					27	24	29	34	31	30	31	29	30	30	22				+7.0
		//		16:9					32	31	33	38	37	37	39	31	37	38	26				+5.6
		//		16:6+7+8				65	58	71	77	79	67	81	73	71	69	52
		//		16:7+8+9				79	76	86	95	94	87	96	83	88	88	64
		//
		//		32 nocount				31	34	36	36	38	46?						>45		39		>43						>44	27	27	27	28	28
		//		32						30	35	38	39	42									>43									38	37	37	36	32
		//						1	2	3	4	5	6	7	8	9	10	11	12	13	14	15	16	17	18	19	20	21	22	23	24	25	26	27	28	29	30	31	32
		//		32:13 nocount			-	16	20	25	21	26	26	25	23	23	25	24	25	25	22	22	22	25	22	23	22	26	22	24	20	19
		//		32:14 nocount				17	24	22	27	27	26	24	24	23	27	24	26	25	24	24	25	26	25	24	24	25	22	24	23	20
		//		32:15 nocount				21	25	23	25	30	28	26	27	27	25	25	26	27	28	24	25	26	25	27	26	29	26	25	23	25		
		//		32:16 nocount				22	25	26	27	28	28	28	28	27	29	27	27	30	26	27	29	28	25	27	29	29	29	27	26	27
		//		32:17 nocount				26	27	26	27	31	29	29	28	26	28	28	26	30	30	27	29	29	29	29	29	27	29	29	25	25
		//		32:18 nocount				24	28	29	29	33	30	31	30	30	31	28	30	27	30	28	30	31	30	30	32	32	30	29	28	27
		//		32:19 nocount				28	30	29	30	34	33	33	27	32	32	31	30	30	32	31	31	30	32	30	29	31	32	30	31	27
		//		32:20 nocount				27	29	30	30	34	33	35	29	32	34	32	34	32	33	30	31	34	33	33	32	34	32	33	31	29
		//		32:21 nocount				28	32	32	30	36	33	33	33	35	34	31	34	35	36	32	33	34	35	34	32	36	34	33	33	30
		//		32:22 nocount				31	31	34	35	37	38	35	36	35	35	34	35	37	36	32	33	34	34	34	39	36	33	34	33	33
		//		32:23 nocount				31	33	34	33	37	36	37	35	39	36	35	36	36	37	34	35	38	36	36	34	36	37	37	34	32
		//		32:24 nocount				30	35	36	37	39	40	39	36	36	36	36	38	37	37	36	35	36	36	38	35	37	37	36	36	34
		//		32nc:22+23+24				92	99	104	105	113	114	111	107	110	107	105	109	110	110	102	103	108	106	108	108	109	107	107	103	99
		//		32nc:19+20+21				83	91	91	90	104	99	101	89	99	100	94	98	97	101	93	95	98	100	97	93	101	98	96	95	86
		//		32nc:19...24				175	190	195	195	217	213	212	196	209	207	199	207	207	211	195	198	206	206	205	201	210	205	203	198	185				8,,,,,9,,10,,17,,24,,12,,,13,15,16,,21
		//		32nc:17+18+19				78	85	84	86	98	92	93	85	88	91	87	86	87	92	86	90	90	91	89	90	90	91	88	84	79			
		//		32nc:17+...+22				164	177	180	181	205	196	196	183	190	194	184	189	191	197	180	187	192	193	190	193	196	190	188	181	171				8,,,,,,,,,17,,9,10,24,,,13,,21,23
		//		32nc:17..22+32:12..17		296	333	353	358	395	376	395	388	383	392	393	387	364	364	357	369	389	386	373	386	381	368	367	362	337				8,10,,,14,,13,,,,20,,11,,15
		//		32nc:19+20+21				83	91	91	90	104	99	101	89	99	100	94	98	97	101	93	95	98	100	97	93	101	98	96	95	86
		//		32nc:13+14+...+18			126	149	151	156	175	167	163	160	156	165	156	158	162	160	152	160	164	156	160	162	168	158	158	145	143
		//		32:12						19	22	20	25	28	22	25	30	25	27	28	29	25	20	24	23	28	26	26	27	28	22	25	22	20				11,,15,,8,14,20,24,,13,23,,21,22		616
		//		32:13					-	20	25	28	25	28	26	27	27	32	32	30	30	28	22	24	26	29	32	27	28	29	25	28	26	22	-			12,13,21,,,14,15,,20,24,,6,8,16,23,26	676,60/25=2.40
		//		32:14						18	24	25	30	30	27	35	34	30	31	35	29	30	26	30	25	32	30	31	31	28	29	27	30	27				10,14,,11,,,20,,13,22,23				724,48/25=1.92
		//		32:15						23	29	31	34	33	34	35	37	34	33	36	35	30	32	32	35	34	33	31	33	31	33	31	32	30				11,,14,,10,15,19,,7,9,12,20,,			811,87/25=3.48
		//		32:16						24	28	32	36	34	34	38	37	35	38	38	35	29	32	32	37	36	35	32	37	33	32	29	34	33				14,10,13,,11,23,19,,7,20,,12,15,21,,	840,29/25=1.16
		//		32:17						28	28	37	37	37	37	39	40	37	37	>41	40	31	37	35	36	38	37	36	37	36	37	38	37	34				14,,11,10,						56/20=2.80
		//		32:12+13+...17				132	156	173	187	190	180	199	205	193	198	209	198	173	167	177	182	197	193	183	193	185	178	179	181	166				14...11...10,,13,15,,20...12,21,23
		//		32nc:19..24 + 32:12..17		307	346	368	382	407	393	411	401	402	405	408	405	380	378	372	380	403	399	388	394	395	383	382	379	351				10,,,,14,,8,,,13,15,,,20,,12,,11
		//		32:17-12					9	6	17	12	9	15	14	10	12	10	14	6	6	17	11	13	10	11	10	10	8	15	13	15	14

		//						1	2	3	4	5	6	7	8	9	10	11	12	13	14	15	16	17	18	19	20	21	22	23	24	25	26	27	28	29	30	31	32
		//		64:19:1-32 nc				18	18	19	25	25	26	25	28	28	28	28	29	30	29	29	27	26	28	25	26	28	26	28	27	29	26	27	26	26
		//		64:20:1-32 nc				18	19	21	22	27	28	27	30	30	30	27	30	31	29	29	29	29	31	26	28	28	27	31	30	30	30	29	29	25
		//		64:21:1-32 nc				17	19	23	25	27	30	31	31	31	29	30	30	32	31	29	30	31	30	29	30	28	30	31	32	33	29	31	31	30
		//		64:22:1-32 nc				19	22	24	26	29	29	27	32	33	32	31	33	32	32	32	30	32	32	28	31	30	34	32	31	32	33	32	31	32
		//		64:23:1-32 nc											32	33			33	34	32				35					33	32	34				.
		//		64:24:1-32 nc											35	34			34	37	34				35					32	32	36				.
		//		sum:64nc:19..22				72	78	87	98	108	113	110	121	122	119	116	122	125	121	119	116	118	121	108	115	114	117	122	120	124	118	119	117	113		41,,16,,28,37,48,,,12,15,26,,11,17,21,,27
		//		sum:64nc:19..23											153	155			155	159	153				156					155	152	158				.		16,37,41,,28,,48,,21,,12,15,26
		//		sum:64nc:19..24											188	189			189	196	187				191					187	184	194				.		16,,37,,28,,41,,,,21,48
		//						33	34	35	36	37	38	39	40	41	42	43	44	45	46	47	48	49	50	51	52	53	54	55	56	57	58	59	60	61	62	63	64
		//		64:19:33-64 nc	24	21	24	24	29	29	27	27	30	29	29	27	26	27	27	30	27	26	28	27	28	26	27	23	27	23	19	19
		//		64:20:33-64 nc	26	25	25	27	31	30	30	29	31	27	29	28	28	29	29	31	31	29	29	25	28	26	30	26	27	22	22	18
		//		64:21:33-64 nc	27	25	26	29	31	29	30	30	31	32	28	30	30	29	29	32	28	29	29	31	29	28	28	28	27	25	20	20
		//		64:22:33-64 nc	29	27	29	28	33	30	30	32	34	29	30	28	30	31	31	31	31	31	31	30	31	30	30	28	29	26	22	23
		//		64:23:33-64 nc					35				33							33												.						.
		//		64:24:33-64 nc					36				35							34												.
		//		sum:64nc:19..22	106	98	104	108	124	118	117	118	126	117	116	113	114	116	116	124	117	115	117	113	116	110	115	105	110	96	83	80
		//		sum:64nc:19..23					159				159							157												.
		//		sum:64nc:19..24					195				194							191												.
		return a;
	}
	void _seed(Uint64 s) {enum { SEEDING_ROUNDS = (OUTPUT_BITS == 64) ? 24 : ((OUTPUT_BITS == 32) ? 24 : ((OUTPUT_BITS == 16) ? 16 : ((OUTPUT_BITS == 8) ? 12 : -1))) };
		//16 bit: 9/10/12/16
		//32 bit: 15/18/21/24
		//64 bit: ?
		if (OUTPUT_BITS == 64) { a = b = c = s; counter = 0; }
		else if (OUTPUT_BITS == 32) { a = s; b = s >> 32; c = a ^ b; counter = 0; }
		else if (OUTPUT_BITS == 16) {a = s;b = s >> 16;c = s >> 32;counter = s >> 48;}
		else if (OUTPUT_BITS == 8) {s ^= s >> 32;a = s; b = s >> 8; c = s >> 16; counter = s >> 24;}
		for (int i = 0; i < SEEDING_ROUNDS; i++) _raw_native();
	}
	void walk_state(StateWalkingObject *walker) {
		walker->handle(a);
		walker->handle(b);
		walker->handle(c);
		walker->handle(d);
		walker->handle(counter);
		walker->handle(counter2);
	}
};
/*
	8 bit: 3,2,1 or 4,2,3
	16 bit: 7,3,2 or 7,5,2 ?
	32 bit: 25,8,3
	64 bit: 25,12,3
*/
class raw_sfc_alternative8  : public _sfc_alternative<Uint8 , 3, 2,1> { public: Uint8  raw8() {return _raw_native();} void seed(Uint64 s) {_seed(s);};};
class raw_sfc_alternative16 : public _sfc_alternative<Uint16, 7, 3,2> {public: Uint16 raw16() {return _raw_native();} void seed(Uint64 s) {_seed(s);};};
class raw_sfc_alternative32 : public _sfc_alternative<Uint32,25, 8,3> {public: Uint32 raw32() {return _raw_native();} void seed(Uint64 s) {_seed(s);};};
class raw_sfc_alternative64 : public _sfc_alternative<Uint64,25,12,3>{public: Uint64 raw64() {return _raw_native();} void seed(Uint64 s) {_seed(s);};};
SEEDED_POLYMORPHIC_CANDIDATE(sfc_alternative, 64)
SEEDED_POLYMORPHIC_CANDIDATE(sfc_alternative, 32)
SEEDED_POLYMORPHIC_CANDIDATE(sfc_alternative, 16)
SEEDED_POLYMORPHIC_CANDIDATE(sfc_alternative, 8)

/*#include <intrin.h>
static Uint32 bswap32a(Uint32 v) {v = (v << 16) | (v >> 16); v = ((v & 0xff00ff00) >> 8) | ((v & 0x00ff00ff) << 8); return v;}
static Uint32 bswap32b(Uint32 v) {union {Uint32 whole; Uint8 parts[4];};whole=v; parts[0]^=parts[3];parts[3]^=parts[0];parts[0]^=parts[3]; parts[1]^=parts[2];parts[2]^=parts[1];parts[1]^=parts[2]; return whole;}
static Uint32 bswap32c(Uint32 v) {union {Uint32 whole; Uint8 parts[4];};whole=v; Uint8 tmp=parts[0];parts[0]=parts[3];parts[3]=tmp; tmp=parts[1];parts[1]=parts[2];parts[2]=tmp; return whole;}
static Uint32 bswap32d(Uint32 v) {return _byteswap_ulong(v);}
static Uint32 bswap32e(Uint32 v) {__asm {mov eax, v} __asm {bswap eax}}*/


/*static Uint64 bswap64a(Uint64 v) {v = (v << 32) | (v >> 32); v = ((v & 0xff00ff00ff00ff00) >> 8) | ((v & 0x00ff00ff00ff00ff) << 8); v = ((v & 0xffff0000ffff0000) >> 16) | ((v & 0x0000ffff0000ffff) << 16); return v;}
static Uint64 bswap64b(Uint64 v) {
	union {Uint64 whole; Uint8 parts[8];};whole=v;
	parts[0]^=parts[7];parts[7]^=parts[0];parts[0]^=parts[7];
	parts[1]^=parts[6];parts[6]^=parts[1];parts[1]^=parts[6];
	parts[2]^=parts[5];parts[5]^=parts[2];parts[2]^=parts[5];
	parts[3]^=parts[4];parts[4]^=parts[3];parts[3]^=parts[4];
	return whole;
}
static Uint64 bswap64c(Uint64 v) {
	union {Uint64 whole; Uint8 parts[8];};whole=v;
	Uint8 tmp=parts[0];parts[0]=parts[7];parts[7]=tmp;
	tmp=parts[1];parts[1]=parts[6];parts[6]=tmp;
	tmp=parts[2];parts[2]=parts[5];parts[5]=tmp;
	tmp=parts[3];parts[3]=parts[4];parts[4]=tmp;
	return whole;
}
static Uint64 bswap64d(Uint64 v) {return __builtin_bswap64(v);}
static Uint64 bswap64e(Uint64 v) {
	__asm__("bswap %0" : "+r" (v) : "0" (v) );
	return v;
}//*/


template<typename Word, int rotate_bits, Uint64 K_>
class _mcx {
	//Word a, b, c, d, e, f, g, h, i;
	//Uint64 a64, b64, c64;
	Uint32 state_0, state_1, state_2, state_3;
	Uint32 lfsr_0, lfsr_1, lfsr_01;
	Uint32 cycle;
public:
	enum {
		OUTPUT_TYPE = PractRand::RNGs::OUTPUT_TYPES::NORMAL_1,
		OUTPUT_BITS = sizeof(Word) * 8,
		FLAGS = PractRand::RNGs::FLAG::NEEDS_GENERIC_SEEDING
	};
	Word rotate(Word value, int bits) {return (value << bits) | (value >> (OUTPUT_BITS - bits));}
	Word lfsr(Word n, Word m) {return (n >> 1) ^ (-(n & 1) & m);}
	Word _raw_native() {
		Uint32 save = state_0 ^ (state_0 << 13);
		state_0 = state_1;
		state_1 = state_2;
		state_2 = state_3;
		state_3 = state_3 ^ (state_3 >> 17) ^ (save ^ (save >> 7));//*/
		/*Uint32 save = state_0;
		state_0 = state_1;
		state_1 = state_2 ^ (state_2 >> 7);
		state_2 = state_3 ^ (state_3 << 13);
		state_3 = state_3 ^ (state_3 >> 17) ^ save;//*/

		lfsr_0 = lfsr(lfsr_0, 0xE28F75ED);
		//lfsr_1 = lfsr(lfsr_1, 0x400005B6);
		//lfsr_01 = (lfsr_0 & ~lfsr_01) | (lfsr_1 & lfsr_01);

		cycle += 0x5D31995A;
		return (state_3 + lfsr_0) ^ cycle; 
		return state_3 ^ cycle;

		//const Word K = (Word)K_;
		//Word old = a + b + counter++;//64 GB? on counter, 8 TB on b
		//a = b ^ (b >> SHIFT2);//128 GB?
		//b = c + (c << SHIFT3);//1 TB
		//c = old + rotate(c,SHIFT1);//important!
		//return old;
		/*Uint64 rv;
		asm (
			"lea (%1, %2), %0\n"
			"mov %2, %1\n"
			"shr $9, %2\n"
			"xor %2, %1\n"
			"lea (%3,%3,8),%2\n"
			"add %0, %3\n"
			:"=r"(rv), "=r"(a), "=r"(b), "=r"(c)//outputs
			:"1"(a), "2"(b), "3"(c)//inputs
			://clobbers
		);*/
		//return rv;//*/
		/*Uint64 rv;
		asm (
			"mov    0(%%rdi), %%rax \n"
			"mov    8(%%rdi), %%rbx \n"
			"mov    16(%%rdi), %%rcx \n"
			"mov    24(%%rdi), %%rdx \n"
			"add    %%rbx, %%rax \n"
			"add    %%rdx, %%rax \n"
			"inc    %%rdx \n"
			"mov    %%rdx, 24(%%rdi) \n"
			"mov    %%rbx, %%rdx \n"
			"shr    $9, %%rbx \n"
			"xor    %%rdx, %%rbx \n"
			"lea    (%%rcx,%%rcx,8), %%rdx\n"
			"rol    $29, %%rcx \n"
			"add    %%rax, %%rcx \n"
			"mov    %%rbx, 0(%%rdi)\n"
			"mov    %%rcx, 16(%%rdi) \n"
			"mov    %%rdx, 8(%%rdi) \n"
			:"=a"(rv)//outputs
			:"D"(this)//inputs
			:"memory", "%rbx", "%rcx", "%rdx"//clobbers
		);
		return rv;//*/

//	printf("%8x%8x\n", Uint32(Candidates::bswap64e(0x123456789abcdefull)>>32), Uint32(Candidates::bswap64e(0x123456789abcdefull)));
		/*Uint64 x = b;
		x *= K;
		Uint32 old_a = a;
		a += K;
		b += old_a + (a < K);
		x = Uint32(x) ^ Uint32(x >> 32) ^ old_a;
		x *= K;
		return Uint32(x) + Uint32(x >> 32) + old_a;*/
		//Uint64 rv;
		/*asm volatile (
			"mov    8(%%rdi), %%rax \n"
			"movabs $0x6595a395a1ec531b, %%rcx \n"
			"mov    (%%rdi), %%rsi \n"
			"mul    %%rcx \n"
			"add    %%rcx, (%%rdi) \n"
			"adc    %%rsi, 8(%%rdi) \n"
			"xor    %%rsi, %%rax \n"
			"xor    %%rdx, %%rax \n"
			"mul    %%rcx \n"
			"add    %%rsi, %%rax \n"
			"add    %%rdx, %%rax \n"
			:"=a"(rv)//outputs
			:"D"(this)//inputs
			:"memory", "%rcx", "%rsi", "%rdx"//clobbers
		);*/
		/*asm(
			"movq %2, %0\n"		//rv = b;
			"mulq %3\n"			//rv, high = mul_high(b, K)
			"addq %3, %1\n"		//a += K;
			"adcq %4, %2\n"		//b += old_a + (a < K);
			"xorq %4, %0\n"		//rv ^= old_a;
			"xorq %%rdx, %0\n"	//rv ^= high;
			"mulq %3\n"			//rv, high = mul_high(rv, K)
			"addq %4, %0\n"		//rv += old_a;
			"addq %%rdx, %0\n"	//rv += high;
			: "=%rax" (rv), "=r" (a), "=r" (b)
			: "r"(K), "r"(a), "1"(1), "2" (b)
			: "%rdx"
		);*/
		//return rv;
		/*Word rv = a ^ b;
		a += K;
		b += K + ((a < K) ? 1 : 0);
		return (rv * K) + a;*/
	}
	void walk_state(StateWalkingObject *walker) {
		//walker->handle(a); walker->handle(b);walker->handle(c); walker->handle(d); walker->handle(e);
		//walker->handle(f); walker->handle(g); walker->handle(h);
		walker->handle(state_0); walker->handle(state_1); walker->handle(state_2); walker->handle(state_3);
		walker->handle(lfsr_0); walker->handle(lfsr_1); walker->handle(lfsr_01);
		walker->handle(cycle);
	}
};
//class raw_mcx16 : public _mcx<Uint16, 7,0x6595a395a1ec531b> {public: Uint16 raw16() {return _raw_native();}};
class raw_mcx32 : public _mcx<Uint32,13,0x6595a395a1ec531b> {public: Uint32 raw32() {return _raw_native();}};
//class raw_mcx64 : public _mcx<Uint64,27,0x6595a395a1ec531b> {public: Uint64 raw64() {return _raw_native();}};
//POLYMORPHIC_CANDIDATE(mcx, 16)
POLYMORPHIC_CANDIDATE(mcx, 32)
//POLYMORPHIC_CANDIDATE(mcx, 64)

class raw_siphash {
public:
	typedef Uint64 Word;
	enum {
		OUTPUT_TYPE = PractRand::RNGs::OUTPUT_TYPES::NORMAL_1,
		OUTPUT_BITS = sizeof(Word)* 8,
		FLAGS = PractRand::RNGs::FLAG::NEEDS_GENERIC_SEEDING
	};
	Word raw64() { return _raw(); }
	enum {
		/*
			for non-cryptographic use, using the most limited round function (double-rounds w/ 5 shifts):
				ROUNDS_PER_OUTPUT >= 1
				ROUNDS_PER_INPUT >= 1? (guessing based upon rounds per output)
				(ROUNDS_PER_OUTPUT + EXTRA_ROUNDS + ROUNDS_PER_INPUT) >= 4?
				...thus, probably ROUNDS_PER_INPUT = 1, ROUNDS_PER_OUTPUT = 1, EXTRA_ROUNDS = 2
				...that works even if two state words are used for output
				...and for any word size from 16 to 64
			for cryptographic use, using the most limited round function (double-rounds w/ 5 shifts):
				ROUNDS_PER_OUTPUT >= 4?
				ROUNDS_PER_INPUT >= 4?
				(ROUNDS_PER_OUTPUT + EXTRA_ROUNDS + ROUNDS_PER_INPUT) >= 10?
				output function may need to be revised?
				...
		*/
		ROUNDS_PER_INPUT = 1,
		EXTRA_ROUNDS = 2,
		ROUNDS_PER_OUTPUT = 1,
		WORD_BITS = sizeof(Word)* 8,
		SH1 = (WORD_BITS == 64) ? 11 : ((WORD_BITS == 32) ?  5 : ((WORD_BITS == 16) ? 3 : -1)),
		SH2 = (WORD_BITS == 64) ? 16 : ((WORD_BITS == 32) ?  8 : ((WORD_BITS == 16) ? 4 : -1)),
		SH3 = (WORD_BITS == 64) ? 32 : ((WORD_BITS == 32) ? 16 : ((WORD_BITS == 16) ? 8 : -1)),
		SH4 = (WORD_BITS == 64) ? 13 : ((WORD_BITS == 32) ?  9 : ((WORD_BITS == 16) ? 3 : -1)),
		SH5 = (WORD_BITS == 64) ? 19 : ((WORD_BITS == 32) ? 11 : ((WORD_BITS == 16) ? 5 : -1)),
		SH6 = (WORD_BITS == 64) ? 32 : ((WORD_BITS == 32) ? 16 : ((WORD_BITS == 16) ? 8 : -1)),
	};
	Word k1, k2, counter;
	class SipHash {
	public:
		//typedef Word Word;//gcc is objecting to this
		Word v[4];
		//Word fx;
		//Word offset;
		void initstate() {
			v[0] = Word(0x736f6d6570736575ULL);
			v[1] = Word(0x646f72616e646f6dULL);
			v[2] = Word(0x6c7967656e657261ULL);
			v[3] = Word(0x7465646279746573ULL);
		}
		void kseed(Word k1, Word k2) {
			initstate();
			v[0] ^= k1; v[1] ^= k2;
			v[2] ^= k1; v[3] ^= k2;
			//fx = 0xff;
			//offset = 0;
		}
		Word rotate(Word value, int bits) { return (value << bits) | (value >> (WORD_BITS - bits)); }
		void round() {
			//64: 11, 16, 32, 13, 19, 32
			//32: 5, 8, 16, 9, 11, 16
			//16: 3, 4, 8, 3, 5, 8
			// 0 1
			// 2 3
			v[0] += v[1];
			v[2] += v[3];
			v[1] = rotate(v[1], SH1);
			v[3] = rotate(v[3], SH2);
			v[1] ^= v[0];
			v[3] ^= v[2];

			v[0] = rotate(v[0], SH3);
			v[2] += v[1];
			v[0] += v[3];
			v[1] = rotate(v[1], SH4);
			v[3] = rotate(v[3], SH5);
			v[1] ^= v[2];
			v[3] ^= v[0];

		}
		void extra_mixing() {
			for (int i = 0; i < EXTRA_ROUNDS; i++) round();
		}
		void feed_in_word(Word value) {
			v[1] ^= value;
			for (int i = 0; i < ROUNDS_PER_INPUT; i++) round();
			v[0] ^= value;
		}
		Word get_result() {
			v[2] = ~v[2];
			for (int i = 0; i < ROUNDS_PER_OUTPUT; i++) round();
			return v[3];
		}
		Word get_result(Word driven) {
			v[2] ^= driven;
			for (int i = 0; i < ROUNDS_PER_OUTPUT; i++) round();
			return v[3];
		}
	};
	SipHash s;
	Word _raw() {
		return s.get_result();
		//Word c = counter++; enum { X = 1 }; if (c & X) return s.v[c & X]; return s.get_result(c);
		/*PractRand::RNGs::LightWeight::arbee hasher;
		hasher.add_entropy64(counter++);
		for (int i = 0; i < 2; i++) hasher.raw64();
		return hasher.raw64();//*/
		SipHash hasher = s;
		hasher.feed_in_word(counter++);
		hasher.extra_mixing();
		if (WORD_BITS < 64 && !counter) {
			k2++; if (!k2) k1++;
			s.initstate();
			s.feed_in_word(k1);
			s.feed_in_word(k2);
		}
		return hasher.get_result();//*/
	}
	std::string get_name() const {
		std::ostringstream tmp;
		//tmp << "SipHash";
		tmp << "SipHash" << (8 * sizeof(Word)) << "-" << ROUNDS_PER_INPUT << "-" << EXTRA_ROUNDS << "-" << ROUNDS_PER_OUTPUT;
		return tmp.str();
	}
	void seed64(Uint64 seed) {
		SipHash hasher;
		hasher.initstate();
		hasher.feed_in_word(seed);
		hasher.feed_in_word(seed >> 32);
		hasher.feed_in_word(seed >> 48);
		hasher.extra_mixing();
		k1 = hasher.get_result();
		k2 = hasher.get_result();
		counter = 0;
		s.initstate();
		s.feed_in_word(k1);
		s.feed_in_word(k2);
	}
	void walk_state(PractRand::StateWalkingObject *walker) {
		walker->handle(k1); walker->handle(k2);
		walker->handle(counter);
	}
};
class polymorphic_siphash : public PractRand::RNGs::vRNG64 {
public:
	typedef raw_siphash ImplementationType;
	enum { OUTPUT_BITS = ImplementationType::OUTPUT_BITS, FLAGS = ImplementationType::FLAGS };
	polymorphic_siphash(PractRand::SEED_NONE_TYPE) {}
	polymorphic_siphash(PractRand::SEED_AUTO_TYPE) { autoseed(); }
	polymorphic_siphash(Uint64 seed_value) { seed(seed_value); }
	ImplementationType implementation;
	Uint64 raw64() { return implementation.raw64(); }
	void walk_state(StateWalkingObject *walker) { implementation.walk_state(walker); }
	std::string get_name() const { return implementation.get_name(); }
};



}//namespace Candidates
#if defined RNG_from_name_h
namespace RNG_Factories {
	void register_candidate_RNGs() {
		RNG_factory_index["sfc_alternative64"] = _generic_recommended_RNG_factory<Candidates::polymorphic_sfc_alternative64>;
		RNG_factory_index["sfc_alternative32"] = _generic_recommended_RNG_factory<Candidates::polymorphic_sfc_alternative32>;
		RNG_factory_index["sfc_alternative16"] = _generic_recommended_RNG_factory<Candidates::polymorphic_sfc_alternative16>;
		RNG_factory_index["sfc_alternative8" ] = _generic_recommended_RNG_factory<Candidates::polymorphic_sfc_alternative8>;
		RNG_factory_index["sfc_alt64"] = _generic_recommended_RNG_factory<Candidates::polymorphic_sfc_alternative64>;
		RNG_factory_index["sfc_alt32"] = _generic_recommended_RNG_factory<Candidates::polymorphic_sfc_alternative32>;
		RNG_factory_index["sfc_alt16"] = _generic_recommended_RNG_factory<Candidates::polymorphic_sfc_alternative16>;
		RNG_factory_index["sfc_alt8" ] = _generic_recommended_RNG_factory<Candidates::polymorphic_sfc_alternative8>;
		RNG_factory_index["vf64"] = _generic_recommended_RNG_factory<Candidates::polymorphic_VeryFast64>;
		RNG_factory_index["vf32"] = _generic_recommended_RNG_factory<Candidates::polymorphic_VeryFast32>;
		RNG_factory_index["vf16"] = _generic_recommended_RNG_factory<Candidates::polymorphic_VeryFast16>;
		RNG_factory_index["vf8" ] = _generic_recommended_RNG_factory<Candidates::polymorphic_VeryFast8>;
		RNG_factory_index["ranrot_var64"] = _generic_recommended_RNG_factory<Candidates::polymorphic_ranrot_variant64>;
		RNG_factory_index["ranrot_var32"] = _generic_recommended_RNG_factory<Candidates::polymorphic_ranrot_variant32>;
		RNG_factory_index["ranrot_var16"] = _generic_recommended_RNG_factory<Candidates::polymorphic_ranrot_variant16>;
		RNG_factory_index["ranrot_var8" ] = _generic_recommended_RNG_factory<Candidates::polymorphic_ranrot_variant8>;
		RNG_factory_index["ranrot_mcx32"] = _generic_recommended_RNG_factory<Candidates::polymorphic_mcx32>;
		RNG_factory_index["siphash"] = _generic_recommended_RNG_factory<Candidates::polymorphic_siphash>;
	}
}
#endif