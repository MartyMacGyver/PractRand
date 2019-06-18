#include <string>
//#include <ostream>
//#include <sstream>
#include <vector>
//#include <list>
#include <set>
#include <map>
#include <cmath>
#include <algorithm>
#include <cstdlib>

#include "PractRand.h"
#include "PractRand/config.h"
#include "PractRand/rng_basics.h"
#include "PractRand/rng_internals.h"
#include "PractRand/test_helpers.h"
#include "PractRand/RNGs/efiix64x384.h"

namespace PractRand {
	namespace Internals {
		static void add_128 ( const Uint32 *a, const Uint32 *b, Uint32 *result ) {
			Uint64 tmp = 0;
			for (int x = 0; x < 4; x++) {
				tmp += a[x];
				tmp += b[x];
				result[x] = Uint32(tmp);
				tmp >>= 32;
			}
		}
		static void multiply_128 ( const Uint32 *a, const Uint32 *b, Uint32 *result ) {
			Uint32 buffer[4];
			Uint64 current = 0, carry = 0, tmp;
			for (int x = 0; x < 4; x++) {
				for (int y = 0; y <= x; y++) {
					tmp = Uint64(a[y]) * b[x-y];
					carry += tmp >> 32;
					current += Uint32(tmp);
				}
				buffer[x] = Uint32(current);
				current = (current >> 32) + carry;
				carry = 0;
			}
			for (int i = 0; i < 4; i++) result[i] = buffer[i];
		}
		static void convert128_64to32( Uint64 low, Uint64 high, Uint32 *destination ) {
			destination[0] = Uint32(low);  destination[1] = Uint32(low >> 32);
			destination[2] = Uint32(high); destination[3] = Uint32(high >> 32);
		}
		void fast_forward_lcg128 ( Uint64 how_far_low, Uint64 how_far_high, Uint64 &value_low, Uint64 &value_high, Uint64 mul_low, Uint64 mul_high, Uint64 add_low, Uint64 add_high ) {
			Uint32 value[4], mul[4], add[4], tmp[4];
			convert128_64to32(value_low, value_high, value);
			convert128_64to32(mul_low, mul_high, mul);
			convert128_64to32(add_low, add_high, add);
			while (1) {
				if (how_far_low & 1) {
					multiply_128(value, mul, value);
					add_128(value, add, value);
					//val = val * mul + add;
				}
				how_far_low = (how_far_low >> 1) | (how_far_high << 63);
				how_far_high >>= 1;
				if (how_far_low == 0 && how_far_high == 0) break;
				multiply_128(add, mul, tmp);
				add_128(add, tmp, add);
				//add = add * mul + add;
				multiply_128(mul, mul, mul);
				//mul = mul * mul;
			}
			value_low  = value[0] | (Uint64(value[1]) << 32);
			value_high = value[2] | (Uint64(value[3]) << 32);
			return;
		}
		Uint64 fast_forward_lcg64 ( Uint64 how_far, Uint64 val, Uint64 mul, Uint64 add ) {
			while (1) {
				if (how_far & 1) val = val * mul + add;
				how_far >>= 1;
				if (how_far == 0) break;
				add = add * mul + add;
				mul = mul * mul;
			}
			return val;
		}
		Uint32 fast_forward_lcg32 ( Uint32 how_far, Uint32 val, Uint32 mul, Uint32 add ) {
			while (1) {
				if (how_far & 1) val = val * mul + add;
				how_far >>= 1;
				if (how_far == 0) break;
				add = add * mul + add;
				mul = mul * mul;
			}
			return val;
		}
		/*static Uint64 rewind_lcg64 ( Uint64 how_far, Uint64 val, Uint64 mul, Uint64 add ) {
			return fast_forward_lcg64( ~how_far + 1, val, mul, add );
		}
		static Uint32 pow_mod_N32(Uint32 x, Uint32 pow, Uint32 mod) {
			if (x <= 1) return x;
			if (pow == 0) return 1;
	
			pow -= 1;
			if (pow == 0) return x;

			Uint64 mul = x;

			while (1) {
				if (pow & 1) x = Uint32((x * mul) % mod);
				pow >>= 1;
				if (!pow) return x;
				mul = (mul * mul) % mod;
			}
		}*/
		Uint32 fast_forward_lcg32c ( Uint32 how_far, Uint32 val, Uint32 mul, Uint32 add, Uint32 mod ) {
			if (val >= mod) val %= mod;
			if (mul >= mod) mul %= mod;
			if (add >= mod) add %= mod;
			if (how_far >= mod) how_far %= mod;
			while (1) {
				if (how_far & 1) {
					val = Uint32((Uint64(val) * mul + add) % mod);
				}
				how_far >>= 1;
				if (how_far == 0) break;
				add = Uint32((Uint64(add) * mul + add) % mod);
				mul = Uint32((Uint64(mul) * mul) % mod);
			}
			return val;
		}


		/*XorshiftMatrix::XorshiftMatrix( int size_, bool identity ) {
			size = size_;
			bits.resize(size * size, false);
			if (identity) for (int i = 0; i < size; i++) set(i,i,true);
		}
		void XorshiftMatrix::apply(const std::vector<bool> &input, std::vector<bool> &output) {
			if (input.size() != size) issue_error();
			output.resize(size);
			for (int i = 0; i < size; i++) {
				bool r = false;
				for (int j = 0; j < size; j++) r ^= input[j] & get(j, i);
				output[i] = r;
			}
		}
		bool XorshiftMatrix::operator==(const XorshiftMatrix &other) const {
			return size == other.size && bits == other.bits;
		}
		XorshiftMatrix XorshiftMatrix::operator*(const XorshiftMatrix &other) const {
			XorshiftMatrix rv(size, false);
			if (other.size != size) issue_error();
			for (int i = 0; i < size; i++) {
				for (int j = 0; j < size; j++) {
					bool r = false;
					for (int k = 0; k < size; k++) {
						r ^= get(j, k) & other.get(k,i);
					}
					rv.set(j, i, r);
				}
			}
			return rv;
		}
		XorshiftMatrix XorshiftMatrix::exponent(Uint64 exponent_value) const {
			XorshiftMatrix rv(size, true), tmp(*this);
			if (!exponent_value) return rv;
			while (true) {
				if (exponent_value & 1) rv = rv * tmp;
				exponent_value >>= 1;
				if (!exponent_value) break;
				tmp = tmp * tmp;
			}
			return rv;
		}
		XorshiftMatrix XorshiftMatrix::exponent2Xminus1(Uint64 x) const {
			//exponent(2**X-1), X may be >=64
			XorshiftMatrix rv(size, true);
			while (x) {
				rv = rv * rv * *this;
				x--;
			}
			return rv;
		}
		bool XorshiftMatrix::verify_period_factorization(const std::vector<Uint64> &factors) const {
			for (size_t i = 0; i < factors.size(); i++) {
				XorshiftMatrix tmp = *this;
				for (size_t j = 0; j < factors.size(); j++) {
					if (i != j) tmp = tmp.exponent(factors[j]);
				}
				bool is_identity = (tmp * *this) == *this;
				if (is_identity != (i == factors.size())) return false;
			}
			return true;
		}*/
	}
	namespace Tests {
		//categories = old # of entries in tables
		//return value = new # of entries in tables
		//combines adjacent entries
		//N should be the minimum number of expected elements per bucket, more or less
		//if aggressive is true, it will treat N as a hard limit on how low probabilities can be
		//otherwise, it will treat it as a soft limit
		//linear combines only adjacent entries; non-linear is not yet implemented
		int simplify_prob_table ( unsigned long categories, double N, double *prob_table, Uint64 *counts, bool linear, bool aggressive ) {
			if (N < 2.0) N = 2.0;
			double E = 1.0 / N;
			int reduced_size = categories;
			if (!linear) {
				std::multimap<double,Uint64> indexed;
				for (unsigned long i = 0; i < categories; i++) indexed.insert(std::pair<double,Uint64>(prob_table[i], counts[i]));
				while (reduced_size > 2) {
					std::multimap<double,Uint64>::iterator a, b;
					a = b = indexed.begin(); b++;
					if (a->first >= E || (b->first >= E && !aggressive)) break;
					double ps = a->first + b->first;
					Uint64 cs = a->second + b->second;
					indexed.erase(a);
					indexed.erase(b);
					indexed.insert(std::pair<double,Uint64>(ps, cs));
					reduced_size -= 1;
				}
				int i = 0;
				for (std::multimap<double,Uint64>::iterator it = indexed.begin(); it != indexed.end(); it++,i++) {
					prob_table[i] = it->first;
					counts[i] = it->second;
				}
				if (i != reduced_size) issue_error();
				return reduced_size;
			}
			else {//linear
				double E2 = aggressive ? 0.5 : E;
				for (int i = 0; i < 100; i++) {
					int combined = 0;
					for (int j = 0; j < reduced_size; j++) {
						double below = 9;
						double above = 9;
						if (j > combined) below = prob_table[j-combined-1];
						if (j < reduced_size-1) above = prob_table[j+1];
						double other = (below < above) ? below : above;
						if ((prob_table[j] < E) && (other < E2)) {
							if (below < above) {
								prob_table[j-combined-1] += prob_table[j];
								counts[j-combined-1] += counts[j];
								combined ++;
							}
							else {
								prob_table[j-combined] = prob_table[j] + prob_table[j+1];
								counts[j-combined] = counts[j] + counts[j+1];
								combined ++;
								j++;
							}
						}
						else {
							prob_table[j-combined] = prob_table[j];
							counts[j-combined] = counts[j];
						}
					}
					reduced_size -= combined;
					if (!combined) break;
				}
			}
			return reduced_size;
		}


		double chi_squared_test ( unsigned long categories, const double *prob_table, const Uint64 *counts ) {
			unsigned long i;
			long double sum = 0, v = 0;

			for (i=0; i<categories; ++i) {
				sum += (long double) counts[i];
			}
			for (i=0; i<categories; ++i)
			{
				long double expected = sum * prob_table[i];
				long double diff = ((long double)counts[i]) - expected;
				diff = fabs(diff) - 0.5;
				v += (diff*diff)/expected;
			}
		//	double normal = (V-(categories-1))/sqrt((double)(categories-1));
			return (double)v;
		}
		double rarity_test(unsigned long categories, const double *prob_table, const Uint64 *counts) {
			long double total = 0;
			std::vector<double> logs; logs.resize(categories);
			long double mean = 0.0;
			for (unsigned long i = 0; i < categories; i++) {
				total += counts[i];
				logs[i] = std::log(prob_table[i]);
				mean += prob_table[i] * logs[i];
			}
			long double dev = 0.0;
			for (unsigned long i = 0; i < categories; i++) {
				double L = logs[i] - mean;
				dev += prob_table[i] * L * L;
			}
			dev = std::sqrt(dev);

			long double sum = 0.0;
			for (unsigned long i = 0; i < categories; i++) {
				double observed = counts[i];
				sum += observed * (logs[i] - mean);
			}
			sum /= dev;

			/*
			//correction for insufficient samples:
			long double p_sum = 0.0;
			long double error_factor = 0.0;
			long double worst = 0.0;
			std::multimap<double,int> probs2;
			for (unsigned long i = 0; i < categories; i++) probs2.insert(std::pair<double,int>(prob_table[i],i));
			for (std::multimap<double,int>::iterator it = probs2.begin(); it != probs2.end(); it++) {
				error_factor += it->first * logs[it->second];
				p_sum += it->first;
				if (p_sum >= 0.1) break;
				double e = error_factor / (std::log(p_sum) * p_sum);
				if (e > worst) worst = e;
			}
			error_factor = worst / total;
			//if (error_factor > )
			*/

			return sum / std::sqrt(total);
		}
		double my_test(unsigned long categories, const double *prob_table, const Uint64 *counts) {
			double score_actual = 0;
			double score_mean = 0;
			//double score_mean_sqr = 0;
			double prob_sum = 0;
			Uint64 total = 0;
			for (unsigned long i = 0; i < categories; i++) {
				double score = prob_table[i] > 0 ? -std::log(prob_table[i]) : 999.0;
				score_mean += prob_table[i] * score;
				//score_mean_sqr += prob_table[i] * score * score;
				prob_sum += prob_table[i];
			}
			score_mean /= prob_sum;
			//score_mean_sqr /= prob_sum;
			double score_deviation = 0;
			for (unsigned long i = 0; i < categories; i++) {
				double score = prob_table[i] > 0 ? -std::log(prob_table[i]) : 999.0;
				score -= score_mean;
				score_deviation += score * score * prob_table[i];
				score_actual += score * counts[i];
				total += counts[i];
			}
			score_deviation = std::sqrt(score_deviation * total);
			double score_normalized = score_actual;
			//score_normalized -= score_mean * total;
			score_normalized /= score_deviation;
			return score_normalized;
		}
		double g_test(unsigned long categories, const double *prob_table, const Uint64 *counts) {
			long double total = 0;
			long double sum = 0;
			for (unsigned long i = 0; i < categories; i++) {
				long double observed = counts[i];
				total += observed;
				if (observed) sum += observed * std::log(observed / prob_table[i]);
			}
			sum -= total * std::log(double(total));
			return (double)sum * 2.0;
		}
		double g_test_flat(unsigned long categories, const Uint64 *counts) {
			long double total = 0;
			long double sum = 0;
			for (unsigned long i = 0; i < categories; i++) {
				long double observed = counts[i];
				total += observed;
				if (observed) sum += observed * std::log(observed);
			}
			sum -= total * std::log(double(total) / double(categories));
			return (double)sum * 2.0;
		}
		double g_test_flat_merge_normal(unsigned long categories, const Uint64 *counts, Uint64 total, double target_ratio) {
			if (categories < 2) return 0;
			if (total == Uint64(-1)) {
				total = 0;
				for (unsigned long i = 0; i < categories; i++) total += counts[i];
			}
			if (!total) return 0;

			double ratio = total / double(categories);
			unsigned long merge = int(std::ceil(target_ratio / ratio));
			if (merge > categories/2) merge = (categories + 1) / 2;
			long double sum = 0;
			unsigned long max = categories - merge;
			Uint64 so_far = 0;
			for (unsigned long i = 0; i <= max; i+=merge) {
				long double observed = 0;
				for (unsigned int sub = 0; sub < merge; sub++) {
					observed += counts[i+sub];
				}
				if (observed) {
					sum += observed * std::log(observed);
					so_far += observed;
				}
			}
			sum -= so_far * std::log(merge * ratio);
			unsigned long cat_left = categories % merge;
			long double observed = 0;
			for (unsigned long i = categories - cat_left; i < categories; i++) observed += counts[i];
			if (observed != total - so_far) issue_error("g_test_flat_merge called with bad total");
			if (observed) sum += observed * std::log( observed / (ratio * cat_left));
			sum *= 2.0;
			return math_chisquared_to_normal(sum, ((categories+merge-1) / merge)-1);
		}

		double math_factorial(double a) {
			//only an aproximation, but a decent one
			if (!a) return 1;
			static double halfL2Pi = std::log(3.14159265358979 * 2) / 2;
			static double halfLPi = std::log(3.14159265358979) / 2;
			double L = std::log(a);
			double r = a * (L - 1) + std::log(a * (1 + 4 * a * (1 + 2 * a))) / 6 + halfLPi;
			//	double r = a * (L - 1) + L/2 + halfL2Pi;
			return exp(r);
		}
		double math_factorial_log(Uint64 a) {
			//only an aproximation, but a decent one
			//double actual = 0;
			/*if (a <= 1) return 0;
			else if (a <= 16) {
				Uint64 f = 1;
				for (int i = 2; i <= a; i++) {
					f *= i;
				}
				return std::log(double(f));
				//actual = std::log(double(f));
			}*/
			if (a < 32) {
				static const double lookup[32] = {//values found here: https://www.johndcook.com/blog/csharp_log_factorial/
					0.000000000000000, 0.000000000000000, 0.693147180559945, 1.791759469228055,    //0-3
					3.178053830347946, 4.787491742782046, 6.579251212010101, 8.525161361065415,    //4-7
					10.604602902745251, 12.801827480081469, 15.104412573075516, 17.502307845873887,//8-11
					19.987214495661885, 22.552163853123421, 25.191221182738683, 27.899271383840894,//12-15
					30.671860106080675, 33.505073450136891, 36.395445208033053, 39.339884187199495,//16-19
					42.335616460753485, 45.380138898476908, 48.471181351835227, 51.606675567764377,//20-23
					54.784729398112319, 58.003605222980518, 61.261701761002001, 64.557538627006323,//24-27
					67.889743137181526, 71.257038967168000, 74.658236348830158, 78.092223553315307,//28-31
				};
				return lookup[a];
			}
			static double halfL2Pi = std::log(3.14159265358979323 * 2) / 2;
			/*static double halfLPi = std::log(3.14159265358979323) / 2;
			double L = std::log(a);
			double r = a * (L - 1) + std::log(a * (1 + 4 * a * (1 + 2 * a))) / 6 + halfLPi;
			//return r;*/
			//	double r = a * (L - 1) + L/2 + halfL2Pi;

			a += 1;//this is an approximation of the gamma function rather than factorial, so add 1
			//algorithm found here: https://www.johndcook.com/blog/2010/08/16/how-to-compute-log-factorial/
			double inv = 1.0 / a;
			double rv = (a - 0.5) * std::log(a) - a + halfL2Pi;
			double inv2 = inv * inv;
			rv += (1.0 / 12) * inv;
			rv -= (1.0 / 360) * (inv * inv2);
			rv += (1.0 / 1260) * (inv * inv2 * inv2);
			return rv;
		}
		static double math_harmonic_series(int n) {
			if (n > 1000) return std::log(double(n)) + 0.57721566490153286;
			long double sum = 0;
			for (;n > 0; n--) sum += 1.0 / n;
			return sum;
		}
		static double math_erf ( double a ) {
			if (a < 0) return -math_erf(-a);
			if (a < 8) {
				double scale = 2 / std::sqrt(3.14159265358979323);
				double a2 = a*a;
				double x = a2*a;
				//double f = 1;
				double r = a - x / 3;
				int i = 1;
				while (x > 0.000000000000000001) {
					x *= a2;
					x /= i * 2;
					r += x / (4.0 * i + 1);
					x *= a2;
					x /= i * 2 + 1;
					r -= x / (4.0 * i + 3);
					i++;
				}
				return scale * r;
			}
			else return 1;
		}
		static double math_erfcx(double x) {//scaled complementary error function, may have the wrong scale or otherwise be wonky?
			//*
			//found this on stackoverflow, http://stackoverflow.com/questions/34723644/scaled-complementary-error-function-erfcxx-computation-avoiding-arithmetic-o
			//seems to work well

			if (x < 0) issue_error("math_erfcx(negative parameter)");
			if (x < 2) return (1 - math_erf(x)) * std::exp(x * x);
			double x2 = x * x;
			double a = 0, b = x2, c = x2, d = 0, e = 0, f = x2;
			for (double i = 1; i < 500; i += 1.0) {
				a = i - 0.5;
				b = 1;
				d = 1 / (b + a * d);
				c = b + a / c;
				e = c * d;
				f = f * e;
				if (e == 1) break;
				a = i;
				b = x2;
				d = 1 / (b + a * d);
				c = b + a / c;
				e = c * d;
				f = f * e;
				if (e == 1) break;
			}
			return x / f / std::sqrt(3.14159265358979);
			// */

			//a closed form approximation, from "Closed-form approximations to the Error and Complementary Error Functions and their applications in atmospheric science"
			//const double k = 2.7749;
			//return k / ((k - 1) * std::sqrt(3.14159265358979 * x * x) + std::sqrt(3.14159265358979 * x * x + k * k));

			/*
			// found this in the same paper, supposedly good for (x > 1+(small amount)), but it does not converge... probably I misinterpretted something

			double old = 0;
			double iterative = 1.0;
			double product = 1.0;
			double base = 2.0 * x * x;
			double e = base;
			double multiplier = 3;
			int i = 1;
			while (std::fabs(old - iterative) > 0.00000000000001) {
				double tmp = -product / e;
				e *= base; product *= multiplier; multiplier += 2;
				tmp += product / e;
				e *= base; product *= multiplier; multiplier += 2;
				old = iterative;
				iterative += tmp;
			}
			return iterative / std::sqrt(3.14159265358979 * x);*/
		}
		static double math_inverse_erf(double x) {
			if (x < 0) return -math_inverse_erf(-x);
			if (x > 1) {
				issue_error("inverse_erf: invalid input\n");
			}
			if (!x) return 0;
			double max = 0.5;
			while (math_erf(max) < x) {
				max *= 2;
				if (max > 999999) return max;
			}
			double min = 0;
			double emin = 0, emax = math_erf(max);
			while (1) {
				double mid = (min+max)/2;
				if (emin == emax) return mid;
				double emid = math_erf(mid);
				if (emid < x) {
					if (mid == min) return mid;
					emin = emid;
					min = mid;
				}
				else if (emid > x) {
					if (mid == max) return mid;
					emax = emid;
					max = mid;
				}
				else {
					return mid;
				}
			}
		}
		static double _math_lower_incomplete_gamma ( double a, double x, double scale = 1.0, double offset = 0.0) {
		recurse:
			if (a == 1) return (1 - std::exp(-x)) * scale + offset;
			if (a == 0.5) return (std::sqrt(3.14159265358979) * math_erf(std::sqrt(x))) * scale + offset;
			if (a > 1) {
				a -= 1;
				offset -= std::pow(x, a) * std::exp(-x) * scale;
				scale *= a;
				goto recurse;
				//return _math_lower_incomplete_gamma( a-1, x, scale, offset );
				//if (a > 1) return (a-1) * math_lower_incomplete_gamma( a-1, x ) - std::pow(x, a-1) * std::exp(-x);
			}
			issue_error();return -1;
		}
		static double math_lower_incomplete_gamma ( double a, double x) {
			return _math_lower_incomplete_gamma(a, x, 1.0, 0.0);
		}
		static double math_gamma_function ( double a ) {
			if (a == 0.5) return std::sqrt(3.14159265358979);
			if (a == 1) return 1;
			if (a == 2) return 1;
			if (a > 1) return math_gamma_function(a-1) * (a-1);
			issue_error();return -1;
		}
		double math_upper_incomplete_gamma ( double a, double x ) {
			if (a == 1) return std::exp(-x);
			if (fabs(floor(a+.5)-a) <= 0.00000000001) {
				if (!x) return math_factorial(a-1);
				int max = int(floor(a+.5) - 1);
				long double sum = 0;
				long double inv = 1;
				for (int i = 0; i < max; i++) {
					sum += std::pow(x,i) * inv;
					inv /= (i+1);
				}
				return (double)sum;
			}
			if (a == 0.5) return sqrt(3.14159265358979) * (1 - math_erf(sqrt(x)));
		//	if (a == 0.0 && x>0) return -math_exponent_integral(-x)
			if (a > 1) return (a-1) * math_upper_incomplete_gamma( a-1, x ) + pow(x, a-1) * ::exp(-x);
			issue_error();return -1;
		}
		double math_chisquared_to_normal ( double chisquared, double DoF ) {
			return ( chisquared - DoF ) / std::sqrt(DoF);
		}
		double math_chisquared_to_pvalue ( double chisquared, double DoF ) {
			if (DoF == 2) return 1 - std::exp(chisquared*-.5);
			long double n = math_chisquared_to_normal(chisquared, DoF);
			//if (fabs(n) > 100) return (n > 0) ? 1 : 0;
			//if (fabs(chisquared) > 100) return math_normaldist_to_pvalue(n);
			long double p = math_lower_incomplete_gamma(DoF/2,chisquared/2) / math_gamma_function(DoF/2);
			if (p < 0) p = 0;
			if (p > 1) p = 1;
			return (double)p;
		}
		double math_pvalue_to_chisquared ( double pvalue, double DoF ) {
			double chisquared = 1.0;
			double step = 4.0;
			int extra = 0;
			while (math_chisquared_to_pvalue(chisquared, DoF) > pvalue) chisquared /= step;
			while (extra < 10) {
				if (step <= 1.0000001) extra ++;
				step = sqrt(step);
				while (math_chisquared_to_pvalue(chisquared, DoF) < pvalue) chisquared *= step;
				chisquared /= step;
			}
			return chisquared;
		}
		double math_normaldist_to_suspicion(double norm) {
			if (norm < 0) return -math_normaldist_to_suspicion(-norm);
			norm *= std::sqrt(0.5);
			double scaled = math_erfcx(norm);
			double scale = norm * norm;
			double ec = 1 - math_erf(norm);
			double l = (std::log(scaled) - scale) / std::log(2.0);
			return -(l + 0);
		}
		double math_normaldist_to_pvalue(double norm) {
			/*double r;
			r = math_erf(norm * sqrt(0.5));
			r *= 0.5;
			r += 0.5;
			return r;*/
			double upper_p, lower_p;
			if (norm >= 0) {
				upper_p = 1;
				lower_p = 0.5;
			}
			else {
				upper_p = 0.5;
				lower_p = 0;
			}
			for (int i = 0; i < 55; i++) {
				double midp = (upper_p + lower_p) / 2;
				double midn = math_pvalue_to_normaldist(midp);
				if (midn >= norm) upper_p = midp;
				else lower_p = midp;
			}
			return (upper_p + lower_p) / 2;	
		}
		double math_normaldist_pdf( double normal ) {
			static double scale = 1 / sqrt(3.14159265358979 * 2);
			return scale * exp(normal*normal * -0.5);
		}
		double math_pvalue_to_normaldist( double pvalue ) {
			//public domain, originally by Peter J. Acklam
			const double a1 = -39.69683028665376;
			const double a2 = 220.9460984245205;
			const double a3 = -275.9285104469687;
			const double a4 = 138.3577518672690;
			const double a5 = -30.66479806614716;
			const double a6 = 2.506628277459239;

			const double b1 = -54.47609879822406;
			const double b2 = 161.5858368580409;
			const double b3 = -155.6989798598866;
			const double b4 = 66.80131188771972;
			const double b5 = -13.28068155288572;

			const double c1 = -0.007784894002430293;
			const double c2 = -0.3223964580411365;
			const double c3 = -2.400758277161838;
			const double c4 = -2.549732539343734;
			const double c5 = 4.374664141464968;
			const double c6 = 2.938163982698783;

			const double d1 = 0.007784695709041462;
			const double d2 = 0.3224671290700398;
			const double d3 = 2.445134137142996;
			const double d4 = 3.754408661907416;

			const double threshold_low = 0.02425;
			const double threshold_high = 1.0 - threshold_low;
			double q, x, r;

			if (pvalue <= 0) return -999999999.;
			if (pvalue >= 1) return +999999999.;

			//Rational approximation for lower region.
			if ((0 < pvalue) && (pvalue < threshold_low)) {
				q = std::sqrt(-2*std::log(pvalue));
				x = (((((c1*q+c2)*q+c3)*q+c4)*q+c5)*q+c6) / ((((d1*q+d2)*q+d3)*q+d4)*q+1);
			}

			//Rational approximation for central region.
			if ((threshold_low <= pvalue) && (pvalue <= threshold_high)) {
				q = pvalue - 0.5;
				r = q*q;
				x = (((((a1*r+a2)*r+a3)*r+a4)*r+a5)*r+a6)*q / (((((b1*r+b2)*r+b3)*r+b4)*r+b5)*r+1);
			}

			//Rational approximation for upper region.
			if ((threshold_high < pvalue) && (pvalue < 1)) {
				q = std::sqrt(-2*std::log(1-pvalue));
				x = -(((((c1*q+c2)*q+c3)*q+c4)*q+c5)*q+c6) / ((((d1*q+d2)*q+d3)*q+d4)*q+1);
			}

			return x;
		}


		//long double gap_probs( int first, int last, long double baseprob = (255.0 / 256.0) );
		double calculate_center_bit_combination_chance(int bits_L2) {
			static const double chance_skipped[25] = {
				0.0,              //1 bit
				0.5,              //2 bit
				0.375,            //4 bit
				0.2734375,        //8 bit
				0.196380615234375,//16 bit
				0.139949934091419,//32 bit
				0.0993467537479669,//64 bit
				0.07038609217001513,//128 bit
				0.04981910993614015,//256 bit
				0.03524463548583874,//512 bit
				0.02492780589297954,//1 Kbit
				0.01762877240484652,//2 Kbit
				0.01246618536376026,//4 Kbit
				0.008815193220481631,//8 Kbit
				0.006233378016746476,//16 Kbit
				0.004407697493275419,//32 Kbit
				0.003116724676252416,//64 Kbit
				0.002203861357197468,//128 Kbit
				0.001558366796642998,//256 Kbit
				0.001101932254924341,//512 Kbit
				0.0007791839556370945,//1 Mbit
				0.0005509663245030477,//2 Mbit
				0.0003895920474830278,//4 Mbit
				0.0002754831868816390,//8 Mbit
				0.0001947960324495749,//16 Mbit
			};
			if (bits_L2 < 25) return chance_skipped[bits_L2];
			else return chance_skipped[24] * std::pow(0.5, 0.5 * (bits_L2 - 24));
		}
		void get_hamming_weight_chances(int num_bits, std::vector<double> &pdf, std::vector<double> &cdf) {
			int n = num_bits/2;
			pdf.resize(n+1);
			cdf.resize(n+1);
			if (num_bits <= 512) {
				//calculate from edge normally
				long double p = std::pow(0.5, num_bits);
				long double prev_cdf = p;
				pdf[0] = p;
				cdf[0] = p;
				for (int i = 1; i <= n; i++) {
					p /= i;
					p *= num_bits + 1 - i;
					pdf[i] = p;
					prev_cdf += p;
					cdf[i] = prev_cdf;
				}
			}
			else if (num_bits < 16384) {
				//calculate from edge as log, to deal with numbers beyond range of double-precision floating point
				double log_p = std::log(0.5) * num_bits;
				double thresh = std::log(0.5) * 1000;
				for (int i = 0; i <= n; i++) {
					if (log_p < thresh) pdf[i] = 0;
					else pdf[i] = std::exp(log_p);
					cdf[i] = i ? cdf[i-1]+pdf[i] : pdf[i];
					log_p += std::log(double(num_bits-i) / double(i+1));
				}//*/
			}
			else {
				//normal approximation - bad for num_bits < 4096
				double mean = num_bits/2.0;
				double dev = sqrt(num_bits * 0.5 * 0.5);
				double delta = 1.0 / dev;
				for (int i = 0; i <= n; i++) {
					double norm = (i - mean) / dev;
					cdf[i] = math_normaldist_to_pvalue(norm + 0.5 * delta);
					//pdf[i] = i ? cdf[i] - cdf[i-1] : cdf[i];
					pdf[i] = math_normaldist_pdf(norm) * delta;
				}//*/
				//calculate from center ; DON'T USE - too much accumulated error on cdf at edges
				/*double p = calculate_center_bit_combination_chance(num_bits_L2);
				pdf[n] = p;
				cdf[n] = (p+1)/2;
				for (int i = n-1; i >= 0; i--) {
					double r = double(i+1)/double(num_bits-i);
					p *= r;
					pdf[i] = p;
					cdf[i] = cdf[i+1]-pdf[i+1];
				}//*/
				// TO DO: test idea: calculate from center near center, from edge far from center
			}
		}

		static const Uint8 reverse_table[256] = {
		//	  0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15
			0  , 128,  64, 192,  32, 160,  96, 224,  16, 144,  80, 208,  48, 176, 112, 240,//0
			8  , 136,  72, 200,  40, 168, 104, 232,  24, 152,  88, 216,  56, 184, 120, 248,//16
			4  , 132,  68, 196,  36, 164, 100, 228,  20, 148,  84, 212,  52, 180, 116, 244,//32
			12 , 140,  76, 204,  44, 172, 108, 236,  28, 156,  92, 220,  60, 188, 124, 252,//48
			2  , 130,  66, 194,  34, 162,  98, 226,  18, 146,  82, 210,  50, 178, 114, 242,//64
			10 , 138,  74, 202,  42, 170, 106, 234,  26, 154,  90, 218,  58, 186, 122, 250,//80
			6  , 134,  70, 198,  38, 166, 102, 230,  22, 150,  86, 214,  54, 182, 118, 246,//96
			14 , 142,  78, 206,  46, 174, 110, 238,  30, 158,  94, 222,  62, 190, 126, 254,//112
			1  , 129,  65, 193,  33, 161,  97, 225,  17, 145,  81, 209,  49, 177, 113, 241,//0+128
			9  , 137,  73, 201,  41, 169, 105, 233,  25, 153,  89, 217,  57, 185, 121, 249,//16+128
			5  , 133,  69, 197,  37, 165, 101, 229,  21, 149,  85, 213,  53, 181, 117, 245,//32+128
			13 , 141,  77, 205,  45, 173, 109, 237,  29, 157,  93, 221,  61, 189, 125, 253,//48+128
			3  , 131,  67, 195,  35, 163,  99, 227,  19, 147,  83, 211,  51, 179, 115, 243,//64+128
			11 , 139,  75, 203,  43, 171, 107, 235,  27, 155,  91, 219,  59, 187, 123, 251,//80+128
			7  , 135,  71, 199,  39, 167, 103, 231,  23, 151,  87, 215,  55, 183, 119, 247,//96+128
			15 , 143,  79, 207,  47, 175, 111, 239,  31, 159,  95, 223,  63, 191, 127, 255,//112+128
		};
		Uint8 reverse_bits8(Uint8 a) {return reverse_table[a];}
		Uint16 reverse_bits16(Uint16 a) {return reverse_bits8 (a >>  8) + (Uint16(reverse_bits8 (Uint8 (a)))<<8);}
		Uint32 reverse_bits32(Uint32 a) {return reverse_bits16(a >> 16) + (Uint32(reverse_bits16(Uint16(a)))<<16);}
		Uint64 reverse_bits64(Uint64 a) {return reverse_bits32(a >> 32) + (Uint64(reverse_bits32(Uint32(a)))<<32);}

		/*static double integral_of_ln_x(double x) {
			return x * (std::log(x) - 1);
		}
		static double distribution_helper(double min_x, double delta_x) {
			double avg1 = (integral_of_ln_x(min_x+delta_x) - integral_of_ln_x(min_x)) / delta_x;
			double avg2 = (integral_of_ln_x(1-min_x) - integral_of_ln_x(1-(min_x+delta_x))) / delta_x;
			return -(avg1 + avg2);
		}
		double raw_test_edge_distribution( unsigned long categories, const double *prob_table, const Uint64 *counts ) {
			//  this is NOT actually used anywhere yet, though it's based upon code used in Test_calibration
			double cum_prob1 = 0;
			double cum_prob2 = 0;
			double sum = 0;
			for (unsigned long i = 0; i < categories; i++) {
				unsigned long ri = categories - 1 - i;
				if (cum_prob1 < 0.5) {
					if (counts[i]) sum += distribution_helper(cum_prob1, prob_table[i]) * counts[i];
					cum_prob1 += prob_table[i];
				}
				if (cum_prob2 < 0.5) {
					if (counts[ri]) sum += distribution_helper(cum_prob2, prob_table[ri]) * counts[ri];
					cum_prob2 += prob_table[ri];
				}
				else if (cum_prob1 >= 0.5) return sum;
			}
		}
		double test_edge_distribution( unsigned long categories, const double *prob_table, const Uint64 *counts ) {
			//  this is NOT actually used anywhere yet, though it's based upon code used in Test_calibration
			double raw = raw_test_edge_distribution(categories, prob_table, counts);
			return raw;
		}*/
		double test_uniformity( const SampleSet &ss ) {
			long size = ss.size();
			if (!size) return 0;
			double size_f = size;
			double size_a = size + 1.0;
			double size_inv = 1.0 / size_a;
			double prior = 0;
			double sum_log = 0;
			//double sum_exp = 0;
			double longest = 0;
			const double epsilon = std::pow(0.5, 50);
			for (int j = 0; j <= size; j++) {
				double current = j < size ? ss.get_result_by_index(j) : 1.0;
				long double delta = current - prior;
				delta += epsilon;
				double L = std::log(delta);
				sum_log += L;
				//sum_exp += std::exp(delta*size_inv);
				if (longest < delta) {
					longest = delta;
				}
				prior = current;
			}
			sum_log += std::log(size_a) * size_a;

			//sum_exp = sum_exp - size_a - 1 / size_a - std::pow(size_a, -3) + std::pow(size_a, -4) - std::pow(size_a, -5) * 2  + std::pow(size_a, -6) * 2 ;
			//sum_exp *= std::sqrt(size_a) * (size_a+1) * (size_a+1) * (size_a+1);
			//sum_exp /= 1 + std::pow(size_a * 0.00047515, 5);

			sum_log = -sum_log;
			sum_log -= size_a * 0.57721566490153286 - 0.5;
			sum_log /= std::sqrt(size_f);
			sum_log += std::pow(2*size_f+1, -1.442695) * 0.2;
			sum_log *= 1.245;

			longest -= math_harmonic_series(size_a) / size_a;
			longest *= size_a+2;

			//double rv = sum_log;
			//double rv = (sum_exp + sum_log) / 1.625;//pretty good for fewer samples, but tends to freak out eventually
			double rv = longest * 0.6 + sum_log * 0.6;//seems good, mean rapidly converges to zero, stddev stays between 0.9 and 1.1 as far as I can see
			//
			// sum_log only:
			//    N      percentiles
			//    1  .01%:-0.65  0.1%:-0.65  1%:-0.65  10%:-0.64  90%: 1.01  99%: 3.27  99.9%: 5.57  99.99%: 7.95  dev:0.844 mean:-0.040 med:-0.367
			//    2  .01%:-0.87  0.1%:-0.87  1%:-0.86  10%:-0.78  90%: 1.08  99%: 2.92  99.9%: 4.71  99.99%: 6.46  dev:0.830 mean:-0.019 med:-0.269
			//    4  .01%:-1.18  0.1%:-1.17  1%:-1.10  10%:-0.87  90%: 1.10  99%: 2.64  99.9%: 4.07  99.99%: 5.29  dev:0.820 mean:-0.008 med:-0.190
			//    8  .01%:-1.56  0.1%:-1.48  1%:-1.31  10%:-0.93  90%: 1.09  99%: 2.44  99.9%: 3.61  99.99%: 4.70  dev:0.813 mean:-0.003 med:-0.131
			//   16  .01%:-1.92  0.1%:-1.75  1%:-1.47  10%:-0.96  90%: 1.08  99%: 2.26  99.9%: 3.28  99.99%: 4.17  dev:0.808 mean:-0.000 med:-0.091
			//   32  .01%:-2.20  0.1%:-1.96  1%:-1.59  10%:-0.98  90%: 1.06  99%: 2.15  99.9%: 3.04  99.99%: 3.82  dev:0.805 mean:-0.002 med:-0.066
			//   64  .01%:-2.42  0.1%:-2.11  1%:-1.67  10%:-1.00  90%: 1.06  99%: 2.07  99.9%: 2.87  99.99%: 3.59  dev:0.804 mean:+0.000 med:-0.044
			//  128  .01%:-2.59  0.1%:-2.21  1%:-1.73  10%:-1.01  90%: 1.05  99%: 2.01  99.9%: 2.77  99.99%: 3.41  dev:0.804 mean:+0.001 med:-0.031
			//  256  .01%:-2.69  0.1%:-2.29  1%:-1.77  10%:-1.01  90%: 1.04  99%: 1.97  99.9%: 2.68  99.99%: 3.29  dev:0.804 mean:+0.001 med:-0.021
			//  512  .01%:-2.78  0.1%:-2.34  1%:-1.80  10%:-1.02  90%: 1.04  99%: 1.94  99.9%: 2.61  99.99%: 3.18  dev:0.803 mean:+0.001 med:-0.016
			// 1024  .01%:-2.88  0.1%:-2.38  1%:-1.82  10%:-1.02  90%: 1.04  99%: 1.92  99.9%: 2.59  99.99%: 3.12  dev:0.803 mean:+0.001 med:-0.011
			// 2048  .01%:-2.90  0.1%:-2.41  1%:-1.83  10%:-1.03  90%: 1.04  99%: 1.90  99.9%: 2.54  99.99%: 3.07  dev:0.803 mean:-0.001 med:-0.009
			// 4096  .01%:-2.90  0.1%:-2.44  1%:-1.85  10%:-1.03  90%: 1.03  99%: 1.89  99.9%: 2.52  99.99%: 3.08  dev:0.803 mean:-0.000 med:-0.006
			// 8192  .01%:-2.91  0.1%:-2.44  1%:-1.85  10%:-1.03  90%: 1.03  99%: 1.89  99.9%: 2.52  99.99%: 3.05  dev:0.802 mean:-0.001 med:-0.005
			//16384  .01%:-2.93  0.1%:-2.46  1%:-1.86  10%:-1.02  90%: 1.03  99%: 1.89  99.9%: 2.51  99.99%: 3.04  dev:0.803 mean:+0.002 med:-0.001

			return rv;
		}
		double test_table_uniformity( unsigned long categories, const double *prob_table, const Uint64 *counts ) {
			long double prob_sum = 0;
			long double total = 0;
			for (unsigned long i = 0; i < categories; i++) total += counts[i];
			if (!total) return 0;
			long double sum_exp = 0, sum_log = 0;
			double adjusted_total = total + 1.0;
			double prior = 0;
			//double DoF = 0;
			for (unsigned long i = 0; i < categories; i++) {
				double cur_prob = prob_table[i];
				if (counts[i]) {
					double c = counts[i];
					//DoF += c; //c > 2 ? 2 : 1;
					double delta = cur_prob / (c + 1);
					//sum += _uniformity_transform(prob_sum + delta * 0.5, adjusted_total);
					if (c > 1) {
						//sum += _uniformity_transform(delta, adjusted_total) * (c-1);
					}
					prior = prob_sum + cur_prob - delta * 0.5;
				}
				prob_sum += cur_prob;
			}
			//sum = sum - adjusted_total * 1.578 + 0.3;
			//return sum / std::sqrt(total);
			return 0;
		}



		void SampleSet::_normalize() {
			std::sort(rs.begin(), rs.end());
		//	std::sort<double*>(&rs[0], &rs[rs.size()]);
			_count_duplicates();
		}
		int SampleSet::get_num_elements_less_than ( double other_result ) const {
			int s = rs.size();
			if (!s) return 0;
			if (rs[s-1] < other_result) return s;
			if (rs[0] >= other_result) return 0;
			int low = 0, high = s-1;
			while (low + 1 < high) {
				int mid = (low + high) >> 1;
				if (rs[mid] < other_result) {
					low = mid;
				}
				else high = mid;
			}
			return low + 1;
		//	for (i = 0; i < s && rs[i] < other_result; i++) ;
		//	return i;
		}
		int SampleSet::get_num_elements_greater_than ( double other_result ) const {
			//could use more optimization, but who cares?
			int s = rs.size();
			if (!s) return 0;
			int i;
			if (rs[0] > other_result) return s;
			for (i = 0; i < s && rs[s-1-i] > other_result; i++) ;
			return i;
		}
		void SampleSet::get_num_elements_less_and_greater ( double other_result, int &num_less, int &num_greater ) const {
			//could use more optimization, but who cares?
			num_less = get_num_elements_less_than(other_result);
			num_greater = get_num_elements_greater_than(other_result);
		}
		void SampleSet::_count_duplicates() {
			duplicates = 0;
			if (!rs.size()) return;
			double o = rs[0];
			for (unsigned int i = 1; i < rs.size(); i++) {
				double n = rs[i];
				if (n == o) duplicates++;
				o = n;
			}
		}
		double SampleSet::_get_index ( double other_result ) const {
			double &r = other_result;
			int s = rs.size();
			if (!s) return 0;
			int lower, higher;
			get_num_elements_less_and_greater(r, lower, higher);
			if (lower+higher < s) {//exact match and possibly duplicates
				return lower + (s-(lower+higher)-1)/2.0;
			}
			else {//normal case
				if (higher == 0) return s-1;
				if (lower == 0) return 0;
				double a = rs[lower-1], b = rs[lower];
				return lower-1 + (r-a)/(b-a);
			}
		}
		double SampleSet::get_percentile ( double other_result ) const {
			double &r = other_result;
			int s = rs.size();
			if (!s) return 0;
			int lower, higher;
			get_num_elements_less_and_greater(r, lower, higher);
			double percentile;
			if (lower+higher < s) {//exact match and/or duplicates?
				percentile = (lower + (s-(lower+higher))/2.0) / s;
			}
			else {//normal case
		//		return (lower + 0.5) / (s+1);
				if (higher == 0) return 1 - 0.5 / (s+1);
				if (lower == 0) return 0.5 / (s+1);
				double a = rs[lower-1], b = rs[lower];
				percentile = (lower-1 + (r-a)/(b-a)) / (s+1);
			}
			return percentile;
		}
		double SampleSet::get_result_by_percentile(double percentile) const {
			//lowest distinct percentile is (0.5 / s), highest is ((s-.5) / s)
			//this allows interpolation while preserving equal weighting for all samples
			std::size_t s = rs.size();
			if (!s) issue_error();
			double di = s * percentile - 0.5;
			if (di <= 0) return rs[0];
			if (di >= s-1) return rs[s-1];
			int ii = int(di);
			double fi = di - ii;
			return rs[ii] + (rs[ii+1] - rs[ii]) * fi;
		}

		void BitMatrix::init(int w_, int h_) {
			w = w_;
			h = h_;
			ww = (w_ + WORD_BITS_MASK) >> WORD_BITS_L2;
			data.resize(ww*h, 0);
			//for (int i = 0; i < ww*h; i++) data[i] = 0;
		}
		void BitMatrix::raw_import(int offset, Word *input, int length) {
			for (int i = 0; i < length; i++) data[offset+i] = input[i];
		}
		void BitMatrix::import_partial_row(int x, int y, Word *input, int bits, int bit_offset, bool zeroed) {
			//49 seconds
			//added zeroed
			//46 seconds
			//added shifts
			Word *dest = &data[y*ww+(x>>WORD_BITS_L2)];
			if (false) {
			/*	//clear partial words at begining & end of region
				int end = x + bits;
				Word start_mask = (Word(1) << (x & WORD_BITS_MASK)) - 1;
				Word end_mask = ~(~Word(0) >> (((end-1) & WORD_BITS_MASK) ^ WORD_BITS_MASK));
				int end_word = end >> WORD_BITS;
				if (!zeroed) {
					clear_rectangle(x, x+bits, y, y+1);
					//dest[0] &= start_mask;
					//dest[end_word] &= end_mask;
				}
				int start_word = x >> WORD_BITS_L2;
				int end_word = (end-1) >> WORD_BITS_L2;
				int out_offset = x & WORD_BITS_MASK;
				input -= start_word;
				if (bit_offset == out_offset) {
					if (start_word == end_word) data[start_word] |= input[start_word] & ~(start_mask & end_mask);
					else {
						data[start_word] |= input[start_word] & ~start_mask;
						for (int xw = start_word+1; xw < end_word; xw++) {
							data[xw] = input[xw];
						}
						data[end_word] |= input[end_word] & ~end_mask;
					}
				}
				else {
					int bit_delta = (bit_offset - out_offset) & WORD_BITS_MASK;
					int iww = 
					for (int iw = 1; iw < ; xw++) {
					;
					if (start_word == end_word) {
					}
					else {
						data[start_word] |= (input[xw] >> bit_offset) & ~start_mask;
						for (int xw = start_word+1; xw < end_word; xw++) {
							data[xw] = (input[xw] >> bit_delta) | (input[xw+1] << (WORD_BITS-bit_delta));
						}
					}
				}*/
			}
			//else if (!bit_offset && 
			else {
				if (!zeroed) clear_rectangle(x, x+bits, y, y+1);
				for (int i = 0; i < bits; i++) {
					int a = i+bit_offset;
					bool value = (input[a >> WORD_BITS_L2] >> (a & WORD_BITS_MASK)) & 1;
					int b = i+(x & WORD_BITS_MASK);
					dest[b >> WORD_BITS_L2] |= Word(value) << (b & WORD_BITS_MASK);
				}
			}
		}
		bool BitMatrix::read_position(int x, int y) const {
			int index = (x >> WORD_BITS_L2)+y*ww;
			Word w = data[index];
			Word w2 = w >> (x & WORD_BITS_MASK);
			bool bit = w2 & 1;
			return bit;
			//return (data[(x >> WORD_BITS_L2)+y*ww] >> (x & WORD_BITS_MASK)) & 1;
		}
		void BitMatrix::xor_rows(int destination, int source) {
			int base1 = ww * destination;
			int base2 = ww * source;
			for (int i = 0; i < ww; i++) data[base1+i] ^= data[base2+i];
		}
		void BitMatrix::xor_rows_skip_start(int destination, int source, int skip) {
			int base1 = ww * destination;
			int base2 = ww * source;
			for (int i = skip; i < ww; i++) data[base1+i] ^= data[base2+i];
		}
		void BitMatrix::clear_rectangle(int min_x, int max_x, int min_y, int max_y) {
			int start_word = min_x >> WORD_BITS_L2;
			int end_word = (max_x-1) >> WORD_BITS_L2;
			Word start_mask = (Word(1) << (min_x & WORD_BITS_MASK)) - 1;
			Word end_mask = ~(~Word(0) >> (((max_x-1) & WORD_BITS_MASK) ^ WORD_BITS_MASK));
			if (start_word == end_word) {
				Word mask = start_mask | end_mask;
				for (int y = min_y; y < max_y; y++) data[y*ww+start_word] &= mask;
				return;
			}
			for (int y = min_y; y < max_y; y++) data[y*ww+start_word] &= start_mask;
			for (int x = start_word + 1; x < end_word; x++) {
				for (int y = min_y; y < max_y; y++) data[y*ww+start_word] = 0;
			}
			for (int y = min_y; y < max_y; y++) data[y*ww+end_word] &= end_mask;
		}
		int BitMatrix::normalize_and_rank() {
			int ranks_found = 0;
			for (int x = 0; x < w; x++) {
				if (read_position(x, ranks_found)) {//row already in correct position
					for (int y = ranks_found+1; y < h; y++) if (read_position(x,y)) xor_rows(y, ranks_found);
					ranks_found++;
				}
				else {
					int y = ranks_found+1;
					while (y < h && !read_position(x, y)) y++;
					if (y != h) {
						xor_rows(ranks_found, y);
						xor_rows(y, ranks_found);
						for (++y; y < h; y++) if (read_position(x,y)) xor_rows(y, ranks_found);
						ranks_found++;
					}
					else ;//skipped rank
				}
			}
			return ranks_found;
		}
		int BitMatrix::large_normalize_and_rank() {
			int ranks_found = 0;

			//trying to minimize sweeps through memory... but not minimize actual math
			//(minimizing math costs too much)
			enum {STEP=24};//rows at once
			//--: 306, 4: 263, 8: 249, 16: 246, 64: 239, 256: 237
			//with "shortened" added it's now... 1: 241, 4: 205, 8: 199, 16: 197, 24: 188, 256: 183
			int last_ranks_found = 0;
			std::vector<int> rank_x;//tells us the first bit of each deferred rank in the range [last_ranks_found,ranks_found)
			int virtual_h = STEP;//rows up to (but not including) virtual_h have already been partially normalized up to ranks_found
			if (virtual_h > h) virtual_h = h;

			//Word *base = &data[0];
			for (int x = 0; x < w; x++) {
				int shortened = last_ranks_found >> WORD_BITS_L2;
				if (ranks_found >= virtual_h) {//time for a sweep
					//because too many unused ranks built up
					for (int y = virtual_h; y < h; y++) {
						for (int rank = last_ranks_found; rank < ranks_found; rank++) {
							int x2 = rank_x[rank-last_ranks_found];
							//if (read_position(x2,y)) xor_rows(y, rank);
							//if (read_position(x2,y)) for (int i = shortened; i < ww; i++) base[y*ww+i] ^= base[rank*ww+i];
							if (read_position(x2,y)) xor_rows_skip_start(y, rank, shortened);
						}
					}
					last_ranks_found = ranks_found;
					rank_x.clear();
					virtual_h = x + STEP;
					if (virtual_h > h) virtual_h = h;
				}
				if (read_position(x, ranks_found)) {//row already in correct position
					for (int y = ranks_found+1; y < virtual_h; y++) if (read_position(x,y)) xor_rows_skip_start(y, ranks_found, shortened);
					rank_x.push_back(x);
					ranks_found++;
				}
				else {
					int y = ranks_found+1;
					while (y < virtual_h && !read_position(x, y)) y++;
					if (y == virtual_h) {
						if (last_ranks_found != ranks_found) {//time for a sweep
							//because we need to search past the end of the stuff that is usable atm
							for (int y = virtual_h; y < h; y++) {
								for (int rank = last_ranks_found; rank < ranks_found; rank++) {
									int x2 = rank_x[rank-last_ranks_found];
									if (read_position(x2,y)) xor_rows_skip_start(y, rank, shortened);
								}
							}
							last_ranks_found = ranks_found;
							rank_x.clear();
							virtual_h = x + STEP;
							if (virtual_h > h) virtual_h = h;
						}
						while (y < h && !read_position(x, y)) y++;//it's now safe to search past virtual_h, because the buffer is clear
					}
					if (y != h) {
						int rfww = ranks_found * ww;
						xor_rows_skip_start(ranks_found, y, shortened);
						//for (int i = shortened; i < ww; i++) base[rfww+i] ^= base[y*ww+i];
						xor_rows_skip_start(y, ranks_found, shortened);
						//for (int i = shortened; i < ww; i++) base[y*ww+i] ^= base[rfww+i];
						for (++y; y < virtual_h; y++) if (read_position(x,y)) xor_rows_skip_start(y, ranks_found, shortened);
						//for (++y; y < virtual_h; y++) if (read_position(x,y)) for (int i = shortened; i < ww; i++) base[y*ww+i] ^= base[ranks_found*ww+i];
						rank_x.push_back(x);
						ranks_found++;
					}
					else ;//skipped rank
				}
			}
			return ranks_found;
		}


	}//Tests
}//PractRand


