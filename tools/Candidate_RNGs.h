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
		//good speed, 16 bit version fails @ 32 GB, 32 bit version passed 8 TB
		/*old = a + b;
		a = b ^ (b >> RSHIFT);
		b = c + (c << LSHIFT);
		c = old + rotate(c,ROTATE);// RSHIFT,LSHIFT,ROTATE : 7,3,9 @ 32 bit
		return old;//*/
		//best quality: 16 bit fails @ 1 TB, but not as fast ;; switching "a += b ^ c;" for "a ^= b + c;" increases that to 2 TB
		old = a + (a << LSHIFT);
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
		FLAGS = PractRand::RNGs::FLAG::NEEDS_GENERIC_SEEDING
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

		a += b; b -= c;
		c += a; a ^= counter++;
		c = rotate(c, 43 % OUTPUT_BITS);
		return a;

		//a = (a << 6) - a;
		//a ^= Word(0x123456789abcdef);
		//b += a;
		//return a ^ rotate(b, 21 % OUTPUT_BITS);
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
class raw_sfc_alternative8  : public _sfc_alternative<Uint8 , 3,2,1> {public: Uint8  raw8 () {return _raw_native();}};
class raw_sfc_alternative16 : public _sfc_alternative<Uint16, 7,3,2> {public: Uint16 raw16() {return _raw_native();}};
class raw_sfc_alternative32 : public _sfc_alternative<Uint32,25,8,3> {public: Uint32 raw32() {return _raw_native();}};
class raw_sfc_alternative64 : public _sfc_alternative<Uint64,25,12,3> {public: Uint64 raw64() {return _raw_native();}};
POLYMORPHIC_CANDIDATE(sfc_alternative, 64)
POLYMORPHIC_CANDIDATE(sfc_alternative, 32)
POLYMORPHIC_CANDIDATE(sfc_alternative, 16)
POLYMORPHIC_CANDIDATE(sfc_alternative, 8)

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

class raw_xsm16 {
public:
	enum {
		OUTPUT_TYPE = PractRand::RNGs::OUTPUT_TYPES::NORMAL_1,
		OUTPUT_BITS = 16,
		FLAGS = PractRand::RNGs::FLAG::ENDIAN_SAFE | PractRand::RNGs::FLAG::USES_SPECIFIED | PractRand::RNGs::FLAG::USES_MULTIPLICATION | PractRand::RNGs::FLAG::SUPPORTS_FASTFORWARD
		//FLAGS = PractRand::RNGs::FLAG::NEEDS_GENERIC_SEEDING | PractRand::RNGs::FLAG::ENDIAN_SAFE | PractRand::RNGs::FLAG::USES_SPECIFIED | PractRand::RNGs::FLAG::USES_MULTIPLICATION | PractRand::RNGs::FLAG::SUPPORTS_FASTFORWARD
	};
public:
	//PractRand::RNGs::Raw::xsm32 base;
	//Uint16 raw16() {return Uint16(base.raw32()>>0);}
	//void walk_state(StateWalkingObject *walker) {base.walk_state(walker);}
protected:
	Uint16 lcg_low, lcg_high, lcg_adder, history;
	void step_backwards() {
		if (!lcg_low && !lcg_high) lcg_adder -= 2;
		bool carry = lcg_low < lcg_adder;
		lcg_low -= lcg_adder;
		lcg_high -= lcg_low + carry;
	}
	Uint16 rotate16(Uint16 value, int bits) {return (value << bits) | (value >> (16-bits));}
	Uint64 rotate64(Uint64 value, int bits) {return (value << bits) | (value >> (64-bits));}
public:
	Uint16 raw16() {
		const Uint16 K = 0xa395;
		
		// 16 GB, medium avalanche, good speed
		/*Uint16 tmp = lcg_high;
		tmp ^= tmp >> 8;
		tmp *= K;
		tmp += rotate16(tmp ^ lcg_low, 6);
		Uint16 old_lcg_low = lcg_low;
		lcg_low += lcg_adder;
		lcg_high += old_lcg_low + ((lcg_low < lcg_adder) ? 1 : 0);
		old = history;
		history = tmp;
		if (!lcg_low) if (!lcg_high) lcg_adder += 2;
		return tmp + rotate16(old, 8);//*/

		// 128 GB, good avalanche, acceptable speed
		/*Uint16 old = history * K;
		Uint16 tmp = lcg_high;
		tmp += rotate16(tmp ^ lcg_low, 6);
		tmp *= K;
		old ^= old >> 8;
		history = tmp ^=  tmp >> 8;

		Uint16 old_lcg_low = lcg_low;
		lcg_low += lcg_adder|1;
		lcg_high += old_lcg_low + ((lcg_low < lcg_adder) ? 1 : 0);
		if (!lcg_low) if (!lcg_high) lcg_adder += 2;

		return tmp + old;//*/

		//*
		// 256 GB, good avalanche, 
		Uint16 rv = history * K;
		Uint16 tmp = lcg_high ^ rotate16(lcg_high + lcg_low, 6);
		tmp *= K;

		Uint16 old_lcg_low = lcg_low;
		lcg_low += lcg_adder;
		old_lcg_low += ((lcg_low < lcg_adder) ? 1 : 0);
		rv = rotate16(rv + lcg_high, 0) ^ rotate16(rv, 4);
		lcg_high += old_lcg_low;
		if (!lcg_low) if (!lcg_high) lcg_adder += 2;

		rv += history = tmp ^= tmp >> 8;
		return rv;//*/
	}
	void seed(Uint64 s) {
		s ^= rotate64(s, 21) ^ rotate64(s, 39);
		lcg_low = Uint16(s);
		lcg_high = Uint16(s>>16);
		lcg_adder = Uint16(s>>32) | 1;
		history = 0;
		raw16();
	}
	void walk_state(StateWalkingObject *walker) {
		walker->handle(lcg_low);
		walker->handle(lcg_high);
		walker->handle(lcg_adder);
		walker->handle(history);
		if (walker->is_clumsy() && !walker->is_read_only()) {
			lcg_adder |= 1;
			step_backwards();
			raw16();
		}
	}//*/
	//void seek_forward (Uint64 how_far);
	//void seek_backward(Uint64 how_far);
};
POLYMORPHIC_CANDIDATE(xsm, 16)
}
#if defined RNG_from_name_h
namespace RNG_Factories {
	void register_candidate_RNGs() {
		RNG_factory_index["xsm16"] = _generic_recommended_RNG_factory<Candidates::polymorphic_xsm16>;
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
	}
}
#endif