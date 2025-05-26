

/*************************************************************

bible_sat

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


#include "support.h"
#include "dimacs.h"
#include "brain.h"

//============================================================
// code for support

void 
print_ex(top_exception& ex1){
	std::cerr << "got top_exception.\n" << ex1.ex_stk << "\n";
	abort_func(0);
}

void
global_data::print_op_cnf(){
	global_data& slv = *this;
	
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
			os << "TEST_SIMPLIFY" << std::endl;
			break;
		default:
			break;
	}
}

void
global_data::do_cnf_file(debug_info& dbg_inf){
	global_data& slv = *this;
	
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
				test_simplify_cnf();
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
global_data::test_open_out(std::ofstream& os){
	global_data& slv = *this;
	
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
global_data::test_cnf_join(){
	global_data& slv = *this;
	
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
global_data::test_cnf_as_ttnf(bool smpfy_it){
	global_data& slv = *this;

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

	/*
	brain brn;
	brn.br_pt_inst = &inst_info;
	brn.load_instance(num_neu, num_var, inst_neus);

	netix& hd_net = brn.get_head_net();

	DBG_PRT(21, os << "BEFORE simplify " << hd_net);

	if(smpfy_it){
		bool just_forced_nuds = true;
		hd_net.simplify_it(just_forced_nuds);
		BRAIN_CK(! brn.has_br_forced());

		if(brn.has_br_cannots()){
			hd_net.set_result(k_no_satisf);
		}
		if(! brn.has_br_to_choose()){
			hd_net.set_result(k_yes_satisf);
		}
	}

	DBG_PRT(22, os << "AFTER simplify " << hd_net);
	*/

	std::ofstream out_stm;
	std::ostream& os = test_open_out(out_stm);

	print_dimacs_of(os, inst_neus, num_neu, num_var);

	slv.count_instance(inst_info);
}

void
global_data::test_cnf_shuffle(){
	global_data& slv = *this;

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
global_data::test_simplify_cnf(){
	global_data& slv = *this;
	
	debug_info dbg_info;
	brain the_brn;
	the_brn.br_dbg_info = &dbg_info;
	the_brn.br_slv = this;
	dbg_info.dbg_brn = &the_brn;
	
	instance_info& inst_info = slv.get_curr_inst();
	the_brn.br_pt_inst = &inst_info;

	long num_ccls = 0;
	long num_vars = 0;
	long num_lits = 0;
	row_long_t after_ccls;

	the_brn.load_it();
	the_brn.simplify_it(num_ccls, num_vars, num_lits, after_ccls);

	if(inst_info.ist_result == k_unknown_satisf){
		SUPPORT_CK_0(! after_ccls.is_empty());
		SUPPORT_CK_0(num_ccls > 0);
		SUPPORT_CK_0(num_vars > 0);

		std::ofstream out_stm;
		std::ostream& os = test_open_out(out_stm);

		print_dimacs_of(os, after_ccls, num_ccls, num_vars);
	}

	slv.count_instance(inst_info);
}

void
global_data::call_solve_instance(debug_info& dbg_inf){
	global_data& slv = *this;

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
global_data::do_instance(debug_info& dbg_inf)
{
	global_data& slv = *this;
	
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




