#ifndef RNG_from_name_h
#define RNG_from_name_h

namespace Special_RNGs {
	template<typename Word>
	class _stdin_reader {
		static void read_failed() {
			std::fprintf(stderr, "error reading standard input\n");
			std::exit(0);
		}
		enum { BUFF_SIZE = 8192 };
		Word *pos, *end;
		bool ended;
		Word buffer[BUFF_SIZE];
		void refill() {
			std::size_t n = std::fread(&buffer[0], sizeof(Word), BUFF_SIZE, stdin);
			if (n < BUFF_SIZE) ended = true;
			pos = &buffer[0];
			end = &buffer[n];
		}
	public:
		_stdin_reader() : ended(false) { refill(); }
		Word read() { Word rv = *(pos++); if (pos == end) refill(); return rv; }
	};
	class RNG_stdin : public PractRand::RNGs::vRNG8 {
		_stdin_reader<PractRand::Uint8> source;
		virtual PractRand::Uint8 raw8() {
			return source.read();
		}
		virtual std::string get_name() const {return "RNG_stdin";}
		virtual int get_native_output_size() const {return -1;}
		virtual void walk_state(PractRand::StateWalkingObject *) {}
	};
	class RNG_stdin8 : public PractRand::RNGs::vRNG8 {
		_stdin_reader<PractRand::Uint8> source;
		virtual PractRand::Uint8 raw8() { return source.read(); }
		virtual std::string get_name() const {return "RNG_stdin8";}
		virtual void walk_state(PractRand::StateWalkingObject *) {}
	};
	class RNG_stdin16 : public PractRand::RNGs::vRNG16 {
		_stdin_reader<PractRand::Uint16> source;
		virtual PractRand::Uint16 raw16() { return source.read(); }
		virtual std::string get_name() const {return "RNG_stdin16";}
		virtual void walk_state(PractRand::StateWalkingObject *) {}
	};
	class RNG_stdin32 : public PractRand::RNGs::vRNG32 {
		_stdin_reader<PractRand::Uint32> source;
		virtual PractRand::Uint32 raw32() { return source.read(); }
		virtual std::string get_name() const {return "RNG_stdin32";}
		virtual void walk_state(PractRand::StateWalkingObject *) {}
	};
	class RNG_stdin64 : public PractRand::RNGs::vRNG64 {
		_stdin_reader<PractRand::Uint64> source;
		virtual PractRand::Uint64 raw64() { return source.read(); }
		virtual std::string get_name() const {return "RNG_stdin64";}
		virtual void walk_state(PractRand::StateWalkingObject *) {}
	};
}

namespace RNG_Factories {
	std::map<std::string, PractRand::RNGs::vRNG *(*)(std::vector<std::string> &params)> RNG_factory_index;
/*	struct RNG_Description {
		const char *name;
		const char *full_name;
		PractRand::RNGs::vRNG *(*factory)(std::vector<std::string> &);
		const char *description;
		const char *param_description;
		int quality;
		int quality_subscores;//decimal digits for each quality subscore - empirical, cycle lengths, statespace, analysis
		int speed;
		int word;
		int size;
		Uint64 flags;
	};*/

	bool parse_argument_list(const std::string &raw, std::string &name, std::vector<std::string> &parameters) {
		// "BDS(BDS(chacha(8),4),8)" -> "SS", "BDS(chacha(8),4)", "8"
		parameters.clear();
		const char *base = raw.c_str();
		const char *tmp = strpbrk(base, "(,)");
		if (!tmp) {
			name = raw;
			return true;
		}
		if (*tmp != '(') return false;
		if (tmp == base) return false;//zero length name not allowed
		std::size_t checkpoint_pos = tmp-base+1;
		name = raw.substr(0, checkpoint_pos-1);
		int level = 1;
		while (level) {
			tmp = strpbrk(tmp+1, "(,)");
			if (!tmp) return false;
			else if (*tmp == '(') level++;
			else if (*tmp == ')' || *tmp == ',') {
				if (level == 1) {
					std::size_t new_checkpoint = tmp-base+1;
					if (new_checkpoint == checkpoint_pos + 1) return false;//parameter = empty string
					parameters.push_back(raw.substr(checkpoint_pos,new_checkpoint-checkpoint_pos-1));
					checkpoint_pos = new_checkpoint;
				}
				if (*tmp == ')') level--;
			}
		}
		return checkpoint_pos == raw.size();
	}
	PractRand::RNGs::vRNG *create_rng(const std::string &raw) {
		std::string rng_name;
		std::vector<std::string> parameters;
		if (!parse_argument_list(raw, rng_name, parameters)) return NULL;
		std::map<std::string, PractRand::RNGs::vRNG *(*)(std::vector<std::string> &params)>::iterator it = RNG_factory_index.find(rng_name);
		if (it == RNG_factory_index.end()) return NULL;
		return it->second(parameters);
	}

	template<class RNG>
	PractRand::RNGs::vRNG *_generic_recommended_RNG_factory(std::vector<std::string> &params) {
		if (params.size() != 0) return NULL;
		return new RNG(PractRand::SEED_NONE);
	}
	template<class RNG>
	PractRand::RNGs::vRNG *_generic_notrecommended_RNG_factory(std::vector<std::string> &params) {
		if (params.size() != 0) return NULL;
		return new RNG();
	}
	PractRand::RNGs::vRNG *lcg_factory(std::vector<std::string> &params) {
		if (params.size() != 2) {params.push_back("wrong number of parameters to lcg - should be lcg(out_bits,total_bits)");return NULL;}
		int out_bits = atoi(params[0].c_str());
		int total_bits = atoi(params[1].c_str());
		if (out_bits!=8 && out_bits != 16 && out_bits != 32) {params.push_back("lcg out_bits must be 8, 16, or 32 bits");return NULL;}
		if (total_bits < out_bits || total_bits > 128) {params.push_back("lcg total_bits invalid: must be out_bits <= total_bits <= 128");return NULL;}
		if (out_bits == 8) {
			if (total_bits <= 64)
				return  new PractRand::RNGs::Polymorphic::NotRecommended::lcg8of64_varqual( total_bits - out_bits);
			else return new PractRand::RNGs::Polymorphic::NotRecommended::lcg8of128_varqual(total_bits - out_bits);
		}
		else if (out_bits == 16) {
			if (total_bits <= 64)
				return  new PractRand::RNGs::Polymorphic::NotRecommended::lcg16of64_varqual( total_bits - out_bits);
			else return new PractRand::RNGs::Polymorphic::NotRecommended::lcg16of128_varqual(total_bits - out_bits);
		}
		else /*if (out_bits == 32)*/ {
			if (total_bits <= 64)
				return  new PractRand::RNGs::Polymorphic::NotRecommended::lcg32of64_varqual( total_bits - out_bits);
			else return new PractRand::RNGs::Polymorphic::NotRecommended::lcg32of128_varqual(total_bits - out_bits);
		}
	}
	PractRand::RNGs::vRNG *xlcg_factory(std::vector<std::string> &params) {
		if (params.size() != 2) {params.push_back("wrong number of parameters to xlcg - should be xlcg(out_bits,total_bits)");return NULL;}
		int out_bits = atoi(params[0].c_str());
		int total_bits = atoi(params[1].c_str());
		if (out_bits!=8 && out_bits != 16 && out_bits != 32) {params.push_back("xlcg out_bits must be 8, 16, or 32 bits");return NULL;}
		if (total_bits < out_bits || total_bits > 64) {params.push_back("xlcg total_bits invalid: must be out_bits <= total_bits <= 64");return NULL;}
		if (out_bits == 8) return new PractRand::RNGs::Polymorphic::NotRecommended::xlcg8of64_varqual(total_bits - out_bits);
		else if (out_bits == 16) return new PractRand::RNGs::Polymorphic::NotRecommended::xlcg16of64_varqual(total_bits - out_bits);
		else /*if (out_bits == 32)*/ return new PractRand::RNGs::Polymorphic::NotRecommended::xlcg32of64_varqual(total_bits - out_bits);
	}
	PractRand::RNGs::vRNG *clcg_factory(std::vector<std::string> &params) {
		if (params.size() != 2) {params.push_back("wrong number of parameters to clcg - should be clcg(out_bits,total_bits)");return NULL;}
		int out_bits = atoi(params[0].c_str());
		int total_bits = atoi(params[1].c_str());
		if (out_bits!=8 && out_bits != 16 && out_bits != 32) {params.push_back("clcg out_bits must be 8, 16, or 32 bits");return NULL;}
		if (total_bits < out_bits || total_bits > 96) {params.push_back("clcg total_bits invalid: must be out_bits+32 <= total_bits <= 96");return NULL;}
		if (out_bits == 8) return new PractRand::RNGs::Polymorphic::NotRecommended::clcg8of96_varqual(total_bits - out_bits - 32);
		else if (out_bits == 16) return new PractRand::RNGs::Polymorphic::NotRecommended::clcg16of96_varqual(total_bits - out_bits - 32);
		else /*if (out_bits == 32)*/ return new PractRand::RNGs::Polymorphic::NotRecommended::clcg32of96_varqual(total_bits - out_bits - 32);
	}
	PractRand::RNGs::vRNG *cxlcg_factory(std::vector<std::string> &params) {
		if (params.size() != 2) {params.push_back("wrong number of parameters to clcg - should be clcg(out_bits,total_bits)");return NULL;}
		int out_bits = atoi(params[0].c_str());
		int total_bits = atoi(params[1].c_str());
		if (out_bits!=8 && out_bits != 16 && out_bits != 32) {params.push_back("cxlcg out_bits must be 8, 16, or 32 bits");return NULL;}
		if (total_bits < out_bits || total_bits > 96) {params.push_back("cxlcg total_bits invalid: must be out_bits+32 <= total_bits <= 96");return NULL;}
		if (out_bits == 8) return new PractRand::RNGs::Polymorphic::NotRecommended::cxlcg8of96_varqual(total_bits - out_bits - 32);
		else if (out_bits == 16) return new PractRand::RNGs::Polymorphic::NotRecommended::cxlcg16of96_varqual(total_bits - out_bits - 32);
		else /*if (out_bits == 32)*/ return new PractRand::RNGs::Polymorphic::NotRecommended::cxlcg32of96_varqual(total_bits - out_bits - 32);
	}
	PractRand::RNGs::vRNG *chacha_factory(std::vector<std::string> &params) {
		if (params.size() > 1) {params.push_back("too many parameters for chacha");return NULL;}
		if (params.size() == 0) return new PractRand::RNGs::Polymorphic::chacha(PractRand::SEED_NONE);
		std::string &param = params[0];
		int value = 0;
		for (unsigned long i = 0; i < param.size(); i++) {
			if (param[i] < '0' || param[i] > '9') {params.push_back("parameter must be an integer between 1 and 255, inclusive");return NULL;}
			value = value * 10 + (param[i] - '0');
			if (value > 255) {params.push_back("number of rounds too high for chacha");return NULL;}
		}
		if (value < 1) {params.push_back("number of rounds too low for chacha");return NULL;}
		PractRand::RNGs::Polymorphic::chacha *rng = new PractRand::RNGs::Polymorphic::chacha(PractRand::SEED_NONE);
		rng->set_rounds(value);
		return rng;
	}
	PractRand::RNGs::vRNG *salsa_factory(std::vector<std::string> &params) {
		if (params.size() > 1) {params.push_back("too many parameters for salsa");return NULL;}
		if (params.size() == 0) return new PractRand::RNGs::Polymorphic::salsa(PractRand::SEED_NONE);
		std::string &param = params[0];
		int value = 0;
		for (unsigned long i = 0; i < param.size(); i++) {
			if (param[i] < '0' || param[i] > '9') {params.push_back("parameter must be an integer between 1 and 255, inclusive");return NULL;}
			value = value * 10 + (param[i] - '0');
			if (value > 255) {params.push_back("number of rounds too high for salsa");return NULL;}
		}
		if (value < 1) {params.push_back("number of rounds too low for salsa");return NULL;}
		PractRand::RNGs::Polymorphic::salsa *rng = new PractRand::RNGs::Polymorphic::salsa(PractRand::SEED_NONE);
		rng->set_rounds(value);
		return rng;
	}
	PractRand::RNGs::vRNG *SelfShrink_factory(std::vector<std::string> &params) {
		//SShrink(BaseRNG)
		if (params.size() != 1) return NULL;
		PractRand::RNGs::vRNG *rng = create_rng(params[0]);
		if (!rng) return NULL;
		return PractRand::RNGs::Polymorphic::NotRecommended::apply_SelfShrinkTransform(rng);
	}
	PractRand::RNGs::vRNG *BDS_factory(std::vector<std::string> &params) {
		//BDS(BaseRNG,log2_of_table_size)
		if (params.size() < 2 || params.size() > 3) return NULL;
		int L2 = std::atoi(params[1].c_str());
		if (L2 < 1 || L2 > 16) return NULL;
		int shift = -1;
		if (params.size() == 3) shift = std::atoi(params[2].c_str());
		PractRand::RNGs::vRNG *rng = create_rng(params[0]);
		if (!rng) return NULL;
		return PractRand::RNGs::Polymorphic::NotRecommended::apply_BaysDurhamShuffle(rng, L2, shift);
	}
	template<class RNG>
	PractRand::RNGs::vRNG *_generic_single_parameter_transform_RNG_factory(std::vector<std::string> &params) {
		if (params.size() != 1) return NULL;
		PractRand::RNGs::vRNG *rng = create_rng(params[0]);
		if (!rng) return NULL;
		return new RNG(rng);
	}
	template<class RNG>
	PractRand::RNGs::vRNG *_generic_single_parameter_RNG_factory(std::vector<std::string> &params) {
		if (params.size() != 1) return NULL;
		return new RNG(atoi(params[0].c_str()));
	}
	template<class RNG>
	PractRand::RNGs::vRNG *_generic_two_parameter_RNG_factory(std::vector<std::string> &params) {
		if (params.size() != 2) return NULL;
		return new RNG(atoi(params[0].c_str()),atoi(params[1].c_str()));
	}
	void register_recommended_RNGs() {
#define REGISTER_RNG_0(RNG) RNG_Factories::RNG_factory_index[ #RNG ] = _generic_recommended_RNG_factory<PractRand::RNGs::Polymorphic:: RNG>;
//#define REGISTER_RNG_0(RNG,DESC) register_rng( #RNG , _generic_recommended_RNG_factory<PractRand::RNGs::Polymorphic:: RNG>, DESC);
	//{ name, full_name, factory, description, param_description, quality, speed, word, size, flags };
//		RNG_Description rngs[] = {
//			{
//		};
		REGISTER_RNG_0( xsm32 )
		REGISTER_RNG_0( xsm64 )
		REGISTER_RNG_0( hc256 )
		REGISTER_RNG_0( isaac32x256 )
		REGISTER_RNG_0( isaac64x256 )
		REGISTER_RNG_0( efiix8x384 )
		REGISTER_RNG_0( efiix16x384 )
		REGISTER_RNG_0( efiix32x384 )
		REGISTER_RNG_0( efiix64x384 )
		REGISTER_RNG_0( mt19937 )
		REGISTER_RNG_0( jsf32 )
		REGISTER_RNG_0( jsf64 )
		REGISTER_RNG_0( sfc16 )
		REGISTER_RNG_0( sfc32 )
		REGISTER_RNG_0( sfc64 )
		REGISTER_RNG_0( arbee )
		REGISTER_RNG_0( trivium )
		REGISTER_RNG_0( sha2_based_pool )
		RNG_Factories::RNG_factory_index[ "chacha" ] = chacha_factory;
		RNG_Factories::RNG_factory_index[ "salsa" ] = salsa_factory;
#undef REGISTER_RNG_0
	}
	void register_input_RNGs() {
#define REGISTER_RNG_0(RNG) RNG_Factories::RNG_factory_index[ #RNG ] = _generic_notrecommended_RNG_factory<  Special_RNGs:: RNG_ ## RNG>;
		REGISTER_RNG_0(stdin)
		REGISTER_RNG_0(stdin8)
		REGISTER_RNG_0(stdin16)
		REGISTER_RNG_0(stdin32)
		REGISTER_RNG_0(stdin64)
#undef REGISTER_RNG_0
	}
	void register_nonrecommended_RNGs() {
#define REGISTER_RNG_0(RNG) RNG_Factories::RNG_factory_index[ #RNG ] = _generic_notrecommended_RNG_factory<  PractRand::RNGs::Polymorphic::NotRecommended:: RNG>;
#define REGISTER_RNG_1(RNG) RNG_Factories::RNG_factory_index[ #RNG ] = _generic_single_parameter_RNG_factory<PractRand::RNGs::Polymorphic::NotRecommended:: RNG>;
#define REGISTER_RNG_2(RNG) RNG_Factories::RNG_factory_index[ #RNG ] = _generic_two_parameter_RNG_factory<PractRand::RNGs::Polymorphic::NotRecommended:: RNG>;
		// include/PractRand/RNGs/other/simple.h
		REGISTER_RNG_0(xsalta16x3)
		REGISTER_RNG_0(xsaltb16x3)
		REGISTER_RNG_0(xsaltc16x3)
		REGISTER_RNG_0(xorshift32)
		REGISTER_RNG_0(xorshift64)
		REGISTER_RNG_0(xorshift32of128)
		REGISTER_RNG_0(xorshift64of128)
		REGISTER_RNG_0(xorshift16of32)
		REGISTER_RNG_0(xorshift32of64)
		REGISTER_RNG_0(xorshift32x4)
		REGISTER_RNG_0(xorwow32of96)
		REGISTER_RNG_0(xorwow32x6)
		REGISTER_RNG_0(sapparot)
		REGISTER_RNG_0(sap16of48)
		REGISTER_RNG_0(sap32of96)
		REGISTER_RNG_0(flea32x1)
		REGISTER_RNG_0(jsf16)
		REGISTER_RNG_0(sfc_v1_16)
		REGISTER_RNG_0(sfc_v1_32)
		REGISTER_RNG_0(sfc_v2_16)
		REGISTER_RNG_0(sfc_v2_32)
		REGISTER_RNG_0(sfc_v3_16)
		REGISTER_RNG_0(sfc_v3_32)
		REGISTER_RNG_0(simpleA)
		REGISTER_RNG_0(simpleB)
		REGISTER_RNG_0(simpleC)
		REGISTER_RNG_0(simpleD)
		REGISTER_RNG_0(simpleE)
		REGISTER_RNG_0(simpleF)
		REGISTER_RNG_0(simpleG)
		REGISTER_RNG_0(trivium_weakenedA)
		REGISTER_RNG_0(trivium_weakenedB)
		REGISTER_RNG_0(mo_Lesr32)
		REGISTER_RNG_0(mo_ResrRers32)
		REGISTER_RNG_0(mo_Rers32of64)
		REGISTER_RNG_0(mo_Resr32of64)
		REGISTER_RNG_0(mo_Resdra32of64)

		// include/PractRand/RNGs/other/mult.h
		RNG_Factories::RNG_factory_index["lcg"] = lcg_factory;
		RNG_Factories::RNG_factory_index["xlcg"] = xlcg_factory;
		RNG_Factories::RNG_factory_index["clcg"] = clcg_factory;
		RNG_Factories::RNG_factory_index["cxlcg"] = cxlcg_factory;
		REGISTER_RNG_0(lcg16of32_extended)
		REGISTER_RNG_0(lcg32_extended)
		REGISTER_RNG_0(garthy16)
		REGISTER_RNG_0(garthy32)
		REGISTER_RNG_0(binarymult16)
		REGISTER_RNG_0(binarymult32)
		REGISTER_RNG_0(rxmult16)
		REGISTER_RNG_0(multish2x64)
		REGISTER_RNG_0(multish3x32)
		REGISTER_RNG_0(multish4x16)
		REGISTER_RNG_0(old_mwlac16)
		REGISTER_RNG_0(mo_Cmfr32)
		REGISTER_RNG_0(mo_Cmr32)
		//REGISTER_RNG_1(clcg8of96_varqual)
		//REGISTER_RNG_1(clcg16of96_varqual)
		//REGISTER_RNG_1(clcg32of96_varqual)
		//REGISTER_RNG_1(cxlcg8of96_varqual)
		//REGISTER_RNG_1(cxlcg16of96_varqual)
		//REGISTER_RNG_1(cxlcg32of96_varqual)
		//REGISTER_RNG_1(lcg8of64_varqual)
		//REGISTER_RNG_1(lcg16of64_varqual)
		//REGISTER_RNG_1(lcg32of64_varqual)
		//REGISTER_RNG_1(lcg8of128_varqual)
		//REGISTER_RNG_1(lcg16of128_varqual)
		//REGISTER_RNG_1(lcg32of128_varqual)
		//REGISTER_RNG_1(xlcg8of64_varqual)
		//REGISTER_RNG_1(xlcg16of64_varqual)
		//REGISTER_RNG_1(xlcg32of64_varqual)

		// include/PractRand/RNGs/other/fibonacci.h
		REGISTER_RNG_0(lfsr_medium)
		REGISTER_RNG_0(mm32)
		REGISTER_RNG_0(mm16of32)
		REGISTER_RNG_0(mm32_awc)
		REGISTER_RNG_0(mwc4691)
		REGISTER_RNG_0(cbuf_accum)
		REGISTER_RNG_0(dual_cbuf)
		REGISTER_RNG_0(dual_cbuf_accum)
		REGISTER_RNG_0(ranrot32)
		REGISTER_RNG_0(fibmul16of32)
		REGISTER_RNG_0(fibmul32of64)
		REGISTER_RNG_0(ranrot32)
		REGISTER_RNG_0(ranrot3tap32)
		REGISTER_RNG_0(mt19937_unhashed)

		// include/PractRand/RNGs/other/indirection.h
		REGISTER_RNG_0(rc4)
		REGISTER_RNG_0(rc4_weakened)
		REGISTER_RNG_1(ibaa8)
		REGISTER_RNG_1(ibaa16)
		REGISTER_RNG_1(ibaa32)
		REGISTER_RNG_1(isaac32_varqual)
		REGISTER_RNG_1(isaac16_varqual)
		REGISTER_RNG_2(efiix4_varqual)
		REGISTER_RNG_2(efiix8_varqual)

		// include/PractRand/RNGs/other/transform.h
		RNG_Factories::RNG_factory_index["BDS"] = BDS_factory;
		RNG_Factories::RNG_factory_index["SShrink"] = SelfShrink_factory;
		RNG_Factories::RNG_factory_index["AsUnknown"] = _generic_single_parameter_transform_RNG_factory<PractRand::RNGs::Polymorphic::NotRecommended::ReinterpretAsUnknown>;
		RNG_Factories::RNG_factory_index["As8" ] = _generic_single_parameter_transform_RNG_factory<PractRand::RNGs::Polymorphic::NotRecommended::ReinterpretAs8>;
		RNG_Factories::RNG_factory_index["As16"] = _generic_single_parameter_transform_RNG_factory<PractRand::RNGs::Polymorphic::NotRecommended::ReinterpretAs16>;
		RNG_Factories::RNG_factory_index["As32"] = _generic_single_parameter_transform_RNG_factory<PractRand::RNGs::Polymorphic::NotRecommended::ReinterpretAs32>;
		RNG_Factories::RNG_factory_index["As64"] = _generic_single_parameter_transform_RNG_factory<PractRand::RNGs::Polymorphic::NotRecommended::ReinterpretAs64>;
#undef REGISTER_RNG_0
#undef REGISTER_RNG_1
#undef REGISTER_RNG_2
	}
}

#endif//RNG_from_name_h