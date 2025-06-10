

/*************************************************************

satex

central.cpp  
(C 2025) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com

funcs that implement top level funcs.

--------------------------------------------------------------*/

#include <execinfo.h>	// trace stack 
#include <cxxabi.h>	// trace stack 
#include <cstring>


#include "solver.h"
#include "dimacs.h"
#include "brain.h"

//============================================================
// code for solver

void 
print_ex(top_exception& ex1){
	std::cerr << "got top_exception.\n" << ex1.ex_stk << "\n";
	abort_func(0);
}

void
solver::print_op_cnf(){
	solver& slv = *this;
	
	std::ostream& os = std::cout;
	switch(slv.op_cnf_id){
		case fo_join:
			os << "TEST_JOIN" << std::endl;
			break;
		case fo_shuffle:
			os << "TEST_SHUFFLE" << std::endl;
			break;
		case fo_as_3cnf:
			os << "TEST_AS_3CNF" << std::endl;
			break;
		case fo_as_ttnf:
			os << "TEST_AS_TTNF" << std::endl;
			break;
		case fo_simplify:
			os << "TEST_TIER_QUEUE" << std::endl;
			break;
		default:
			break;
	}
}

void
solver::do_cnf_file(debug_info& dbg_inf){
	solver& slv = *this;
	
	//std::ostream& os = std::cout;
	try{
		switch(slv.op_cnf_id){
			case fo_join:
				test_cnf_join();
				break;
			case fo_as_3cnf:
				test_cnf_as_ttnf(false);
				break;
			case fo_as_ttnf:
				test_cnf_as_ttnf(true);
				break;
			case fo_shuffle:
				test_cnf_shuffle();
				break;
			case fo_simplify:
				test_tier_queue();
				break;
			default:
				do_instance(dbg_inf);
				break;
		}
	} catch (top_exception& ex1){
		ex1.prt_ex(std::cout);
	}
	catch (...) {
		std::cerr << "INTERNAL ERROR !!! (do_cnf_file)" << "\n";
		std::cerr << STACK_STR << "\n";
		abort_func(0);
	}
}

std::ostream&
solver::test_open_out(std::ofstream& os){
	solver& slv = *this;
	
	if(slv.output_file_nm == ""){
		return std::cout;
	}

	//instance_info& inst_info = slv.get_curr_inst();
	//std::string f_nam = inst_info.get_f_nam();
	//std::string of_nam = f_nam + "_out_" + suf;
	std::string of_nam = slv.output_file_nm;

	const char* out_nm = of_nam.c_str();
	remove(out_nm);

	os.open(out_nm, std::ios::app);
	if(! os.good() || ! os.is_open()){
		slv.reset_err_msg();
		slv.error_stm << "Could not open file " << of_nam << ". Using cout.";
		std::cerr << slv.error_stm.str() << std::endl;
		return std::cout;
	}
	return os;
}

void
solver::test_cnf_join(){
	solver& slv = *this;
	
	instance_info& inst_info = slv.get_curr_inst();

	inst_info.ist_num_laps++;
	//DBG(slv.dbg_update_config_entries());

	row<long> inst_neus1;
	row<long> inst_neus2;

	std::string f_nam1 = inst_info.get_f_nam();
	std::string f_nam2 = slv.op_cnf_f_nam;

	dimacs_loader	the_loader1;
	the_loader1.parse_file(f_nam1, inst_neus1);

	dimacs_loader	the_loader2;
	the_loader2.parse_file(f_nam2, inst_neus2);

	join_cnf_lits(inst_neus1, the_loader1.ld_num_vars, inst_neus2);

	long num_neu = the_loader1.ld_num_ccls + the_loader2.ld_num_ccls;
	long num_var = the_loader1.ld_num_vars + the_loader2.ld_num_vars;

	inst_info.ist_solve_time = run_time();

	/*
	canon_cnf joined_cnf;
	joined_cnf.fill_with(inst_neus1, num_neu, num_var);

	joined_cnf.print_canon_cnf(os);
	*/
	std::ofstream out_stm;
	std::ostream& os = test_open_out(out_stm);

	print_dimacs_of(os, inst_neus1, num_neu, num_var);

	inst_info.ist_result = k_yes_satisf;

	slv.count_instance(inst_info);
}

// test as ttnf

void
solver::test_cnf_as_ttnf(bool smpfy_it){
	solver& slv = *this;

	instance_info& inst_info = slv.get_curr_inst();

	inst_info.ist_num_laps++;
	//DBG(slv.dbg_update_config_entries());

	row<long> inst_neus;

	std::string f_nam = inst_info.get_f_nam();

	dimacs_loader	the_loader;
	the_loader.ld_as_3cnf = true;
	the_loader.parse_file(f_nam, inst_neus);

	long num_neu = the_loader.ld_num_ccls;
	long num_var = the_loader.ld_num_vars;

	inst_info.ist_solve_time = run_time();

	std::ofstream out_stm;
	std::ostream& os = test_open_out(out_stm);

	print_dimacs_of(os, inst_neus, num_neu, num_var);

	slv.count_instance(inst_info);
}

void
solver::test_cnf_shuffle(){
	solver& slv = *this;

	instance_info& inst_info = slv.get_curr_inst();

	inst_info.ist_num_laps++;
	//DBG(slv.dbg_update_config_entries());

	row<long> inst_neus;

	std::string f_nam = inst_info.get_f_nam();

	dimacs_loader	the_loader;
	the_loader.parse_file(f_nam, inst_neus);

	long num_neu = the_loader.ld_num_ccls;
	long num_var = the_loader.ld_num_vars;

	inst_info.ist_solve_time = run_time();

	long rnd_base = gen_random_num(0, 99999);
	tak_mak rnd_gen(rnd_base);

	row<long> shuff_neus;
	row<long> tmp_map;

	shuffle_full_cnf(rnd_gen, num_var, tmp_map, inst_neus, shuff_neus);

	std::ofstream out_stm;
	std::ostream& os = test_open_out(out_stm);

	print_dimacs_of(os, shuff_neus, num_neu, num_var);

	slv.count_instance(inst_info);
}

void
solver::call_solve_instance(debug_info& dbg_inf){
	solver& slv = *this;

	brain the_brn;
	the_brn.br_dbg_info = &dbg_inf;
	the_brn.br_slv = this;
	dbg_inf.dbg_brn = &the_brn;

	instance_info& inst_info = slv.get_curr_inst();
	the_brn.br_pt_inst = &inst_info;

	the_brn.load_it();
	the_brn.solve_it();

	slv.count_instance(inst_info);
}

void
solver::do_instance(debug_info& dbg_inf)
{
	solver& slv = *this;
	
	instance_info& the_ans = slv.get_curr_inst();
	MARK_USED(the_ans);

	/*DBG_PRT(0, os << "STARTING. batch_count=" 
		<< slv.batch_consec << " of " 
		<< slv.batch_num_files);*/

	std::string f_nam = the_ans.get_f_nam();
	SUPPORT_CK_0(f_nam.size() > 0);
	//DBG_PRT(0, os << "FILE=" << f_nam << std::endl);

	call_solve_instance(dbg_inf);

	/*DBG_PRT(0, os << "FINISHING. batch_count="
		<< slv.batch_consec << " of " 
		<< slv.batch_num_files << std::endl;
		the_ans.print_headers(os);
		os << std::endl << the_ans;
	);*/
}

void
tq_print_options(){
	std::cout << "p (push) k (pick) \n";	
}

typedef tier_queue<prop_signal> tq_psig_t;

quanton*
tq_get_quanton(brain& brn){
	std::string the_ln;
	std::cout << "quanton id ?\n";
	long max = brn.br_positons.last_idx();
	std::cout << "min=" << -max << " max=" << max << "\n";
	std::getline(std::cin, the_ln);
	const char* str1 = the_ln.c_str();
	long qid = parse_int(str1, 0);
	quanton* qua = brn.get_quanton(qid);
	return qua;
}

long
tq_get_tier(tq_psig_t& tq){
	std::string the_ln;
	std::cout << "tier ?\n";
	long max = tq.last_tier();
	std::cout << " last_tier=" << max << "\n";
	std::getline(std::cin, the_ln);
	const char* str1 = the_ln.c_str();
	long ti = parse_int(str1, 0);
	return ti;
}

void
solver::test_tier_queue(){
	//solver& slv = *this;
	brain brn;
	
	brn.init_loading(17, 5);
	
	tq_psig_t tq1(get_ptier);
	
	prop_signal snw;
	while(true){
		std::string the_ln;
		tq_print_options();
		std::getline(std::cin, the_ln);
		if(the_ln == ""){
			break;
		}
		if(the_ln == "p"){
			quanton* qua = tq_get_quanton(brn);
			long ti = tq_get_tier(tq1);
			snw.init_prop_signal(qua, NULL_PT, ti);
			tq1.push(snw);			
		}
		else if(the_ln == "k"){
			prop_signal o1 = tq1.pick();
			std::cout << "sig=" << o1 << "\n";
		}
		std::cout << "TIERS=\n" << tq1 << "\n";
	}
};


