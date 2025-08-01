

/*************************************************************

satex

solver.cpp  
(C 2025) QUIROGA BELTRAN, Jose Luis. Bogot� - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com

solver class and classes that assist the system.

--------------------------------------------------------------*/

#include <limits.h>
#include <cstring>

#include "solver.h"
#include "config.h"
#include "sha2.h"

std::string	satisf_val_nams[k_last_satisf_val];

void	init_glb_nams(){
	init_nams(satisf_val_nams, k_first_satisf_val, k_last_satisf_val);
	satisf_val_nams[k_unknown_satisf] = "unknown";
	satisf_val_nams[k_yes_satisf] = "yes_satisf";
	satisf_val_nams[k_no_satisf] = "no_satisf";
	satisf_val_nams[k_timeout] = "timeout";
	satisf_val_nams[k_memout] = "memout";
	satisf_val_nams[k_error] = "error";
}

satisf_val	
result_str_to_val(t_string& res_str){
	satisf_val val = k_unknown_satisf;
	if(res_str == satisf_val_nams[k_yes_satisf]){
		val = k_yes_satisf;
	} else if(res_str == satisf_val_nams[k_no_satisf]){
		val = k_no_satisf;
	} else if(res_str == satisf_val_nams[k_timeout]){
		val = k_timeout;
	} else if(res_str == satisf_val_nams[k_memout]){
		val = k_memout;
	} else if(res_str == satisf_val_nams[k_error]){
		val = k_error;
	}
	return val;
}

void	glb_set_memout(){
	throw memory_exception(mex_memout);
}

void
read_file(t_string f_nam, row<char>& f_data){
	const char* ff_nn = f_nam.c_str();
	std::ifstream istm;
	istm.open(ff_nn, std::ios::binary);
	if(! istm.good() || ! istm.is_open()){
		throw file_exception(flx_cannot_open, f_nam);
	}

	// get size of file:
	istm.seekg (0, std::ios::end);
	long file_sz = istm.tellg();
	istm.seekg (0, std::ios::beg);

	if(file_sz < 0){
		throw file_exception(flx_cannot_calc_size, f_nam);
	}

	DBG_CK(sizeof(char) == 1);

	char* file_content = tpl_malloc<char>(file_sz + 1); // leave room for END_OF_SEC
	istm.read(file_content, file_sz);
	long num_read = istm.gcount();
	if(num_read != file_sz){
		tpl_free<char>(file_content, file_sz + 1);

		throw file_exception(flx_cannot_fit_in_mem, f_nam);
	}
	file_content[file_sz] = END_OF_SEC;

	s_row<char> tmp_rr;
	tmp_rr.init_data(file_content, file_sz + 1);

	f_data.clear();
	tmp_rr.move_to(f_data);

	DBG_CK(f_data.last() == END_OF_SEC);
	f_data.pop(); // read as writed but leave room for END_OF_SEC
}

void
sha_bytes_of_arr(uchar_t* to_sha, long to_sha_sz, row<uchar_t>& the_sha){
	the_sha.clear();
	the_sha.fill(0, NUM_BYTES_SHA2);
	uchar_t* sha_arr = (uchar_t*)(the_sha.get_c_array());

	uchar_t* ck_arr1 = to_sha;
	MARK_USED(ck_arr1);

	sha2(to_sha, to_sha_sz, sha_arr, 0);
	TOOLS_CK(ck_arr1 == to_sha);
	TOOLS_CK((uchar_t*)(the_sha.get_c_array()) == sha_arr);
}

t_string 
sha_txt_of_arr(uchar_t* to_sha, long to_sha_sz){
	row<uchar_t>	the_sha;
	sha_bytes_of_arr(to_sha, to_sha_sz, the_sha);
	t_string sha_txt = the_sha.as_hex_str();
	return sha_txt;
}

long
find_first_digit(std::string& the_str, bool dig = true){
	unsigned long aa = 0;
	if(dig){
		for(	aa = 0; 
			((aa < the_str.size()) && ! isdigit(the_str[aa])); 
			aa++)
		{}
	} else {
		for(	aa = 0; 
			((aa < the_str.size()) && isdigit(the_str[aa])); 
			aa++)
		{}
	}
	return aa;
}

long
get_free_mem_kb(){
	//std::ostream& os = std::cout;

	long free_mem_kb = 0;
	
	std::string str_mem_free ("MemFree:");
	long str_mem_free_ln = str_mem_free.size();
	
	std::string str_ln;

	std::ifstream proc_meminfo;
	proc_meminfo.open("/proc/meminfo");
	if(proc_meminfo.good() && proc_meminfo.is_open()){
		while(! proc_meminfo.eof()){
			std::getline(proc_meminfo, str_ln);
			
			//os << "Lei:<<" << str_ln << ">>" << std::endl;
			
			if(str_ln.compare(0, str_mem_free_ln, str_mem_free) == 0){
				long pos = find_first_digit(str_ln);
				std::string str_val = str_ln.substr(pos);
				pos = find_first_digit(str_val, false);
				str_val = str_val.substr(0, pos);
				
				free_mem_kb = atol(str_val.c_str());
				
				//os << "FOUND IT = >" << str_val << "<" << std::endl;
				break;
			}
		}
		proc_meminfo.close();
	}
	return free_mem_kb;
}

//============================================================
// solver

void
solver::init_solver(){
	sl_dbg_info = NULL;
	
	silent = false;

	using_mem_ctrl = false;

	prt_help = false;
	prt_version = false;
	prt_headers = false;

	help_str =
		"<file_name[.lst]> [-h | -v] [-p <root_path>] [-m <max_kb_to_use>] \n"
		"\n"
		"-h : print help.\n"
		"-v : print version.\n"
		"-p : set the root path to <root_path>.\n"
		"-m : set max number of RAM Kbytes to use while running.\n"
		"\n"
		;

	version_str =
		"v0.1\n"
		"(c) 2025. QUIROGA BELTRAN, Jose Luis. c.c. 79523732. Bogota - Colombia.\n"
		;

	just_read = false;

	MEM_CTRL(using_mem_ctrl = true);

	MEM_STATS.set_memout_func = glb_set_memout;		

	out_os = &(std::cout);
	long ii = 0;
	long num_out_levs = OUT_NUM_LEVS;
	out_lev.set_cap(num_out_levs);
	for(ii = 0; ii < num_out_levs; ii++){
		out_lev.inc_sz() = false;
	}

	op_cnf_id = fo_invalid;
	op_cnf_f_nam = "";
	op_cnf_num = 1;
	op_ck_satisf = false;
	op_dbg_no_learning = false;

	dbg_file_name = "";

	dbg_num_laps = 0;

	reset_err_msg();

	error_cod = -1;

	input_file_nm = "";
	output_file_nm = "";
	
	input_log_name = "";

	batch_log_on = true;
	batch_name = "";
	batch_log_name = "";
	batch_end_log_name = "";
	batch_end_msg_name = "";
	batch_answer_name = "";
	batch_num_files = 0;
	batch_consec = 0;

	batch_num_unknown_satisf = 0;
	batch_num_yes_satisf = 0;
	batch_num_no_satisf = 0;
	batch_num_timeout = 0;
	batch_num_memout = 0;
	batch_num_error = 0;

	batch_stat_choices.vs_nam = "CHOICES";
	batch_stat_laps.vs_nam = "LAPS";
	batch_stat_added_fires.vs_nam = "ADDED_FIRES";
	batch_stat_load_tm.vs_nam = "LOAD SEGs";
	batch_stat_solve_tm.vs_nam = "SOLVE SEGs";
	batch_stat_mem_used.vs_nam = "BYTES USED";

	batch_start_time = 0.0;
	/*
	batch_max_mem_used = 0;
	batch_max_mem_used_perc = 0.0;
	*/
	batch_end_time = 0.0;

	init_glb_nams();
	//init_brn_nams();

	set_active_out_levs();
}

void	
solver::set_active_out_levs(){
	//solver& slv = *this;

	//search PRT_OUT(val
	// max lev == 9

	out_lev[0] = true;	// batch data
	//out_lev[1] = true;	// file data

}

std::ostream&
solver::print_mem_used(std::ostream& os){
	solver& slv = *this;
	if(slv.using_mem_ctrl){
		os << slv.batch_stat_mem_used;
		/*
		long used_mem = (long)mem_percent_used();
		os << "MEM=" << MEM_STATS.num_bytes_in_use << " bytes of " 
			<< MEM_STATS.num_bytes_available << " (" << used_mem << "%)";
		*/
	}
	return os;
}

std::ostream&
solver::print_totals(std::ostream& os, double curr_tm){
	solver& slv = *this;

	os << std::endl;
	if(in_valid_inst()){
		instance_info& inst_info = get_curr_inst();
		os << "CURR_LAPS=" << inst_info.ist_num_laps << " ";
	}
	os << "YES_SAT=" << batch_num_yes_satisf << " ";
	os << "NO_SAT=" << batch_num_no_satisf << " ";
	os << "TMOUT=" << batch_num_timeout << " ";

	os << "MMOUT=" << batch_num_memout << " ";
	os << "ERR=" << batch_num_error << " ";
	os << "UNKNOWN_SAT=" << batch_num_unknown_satisf << " ";

	if(curr_tm != 0.0){
		double full_tm = curr_tm - batch_start_time;
		os << std::fixed;
		os.precision(2);
		os << "SECS=" << full_tm << " ";
	}

	os << std::endl;
	os << batch_stat_solve_tm;
	if(slv.using_mem_ctrl){
		os << batch_stat_mem_used;
	}

	DBG_COMMAND(33, 
		os << batch_stat_laps;
	);

	os << std::endl;
	return os;
}

std::ostream&
avg_stat::print_avg_stat(std::ostream& os){
	double avg_perc = 0.0;
	if(vs_max_val > 0.0){
		avg_perc = ((avg / vs_max_val) * 100.0);
	}

	os << std::fixed;
	os.precision(2);

	os << vs_nam;
	os << " tot=" << vs_tot_val;
	os << " max=" << vs_max_val;
	os << " avg=" << avg;
	os << " %avg=" << avg_perc;
	os << std::endl;
	os.flush();
	return os;
}

std::ostream&
solver::print_final_totals(std::ostream& os){
	solver& slv = *this;
	
	os << std::endl;
	//os << std::fixed;
	//os.precision(2);

	os << slv.batch_stat_choices;
	os << slv.batch_stat_laps;
	if(slv.op_ck_satisf || slv.doing_dbg()){
		os << slv.batch_stat_added_fires;
	}
	os << slv.batch_stat_load_tm;
	os << slv.batch_stat_solve_tm;
	os << slv.batch_stat_mem_used;

	/*
	os << "AVERAGE SOLVE TIME = " <<
		slv.batch_avg_solve_time.avg << std::endl;
	os << "TOTAL SOLVE TIME = " << 
		slv.batch_total_solve_time << std::endl;
	*/

	double tot_tm = slv.batch_end_time - slv.batch_start_time;
	os << "TOTAL TIME = " << tot_tm << std::endl;
	return os;
}

void			
solver::print_batch_consec(){
	solver& slv = *this;
	
	DBG_COMMAND(34, return);

	double perc = 0.0;
	if(batch_num_files > 0){
		perc = (batch_consec * 100.0) / batch_num_files;
	}

	PRT_OUT(0, 
	os << CARRIAGE_RETURN;
	//os.width(15);
	//os.fill(' ');
	os << std::fixed;
	os.precision(2);
	os << batch_consec << " of " << batch_num_files << " (" << perc << " %)";
	os.flush();
	);
}

void
solver::print_final_assig(){
	solver& slv = *this;
	
	if(slv.batch_answer_name.size() == 0){
		return;
	}

	instance_info& inst_info = get_curr_inst();
	std::string f_nam = inst_info.get_f_nam();

	const char* f_nm = slv.batch_answer_name.c_str();
	std::ofstream log_stm;
	log_stm.open(f_nm, std::ios::app);
	if(log_stm.good() && log_stm.is_open()){
		log_stm << "c ====================" << std::endl;
		log_stm << "c " << f_nam << std::endl;
		final_chosen_ids.print_row_data(log_stm, false, " ", -1, -1, false, 20);
		log_stm << " 0" << std::endl;
		final_assig.print_row_data(log_stm, false, " ", -1, -1, false, 20);
		log_stm << " 0" << std::endl;
	}
	log_stm.close();
}

void
solver::count_instance(instance_info& inst_info){
	solver& slv = *this;
	
	double end_time = run_time();
	double full_tm = end_time - inst_info.ist_solve_time;

	batch_stat_laps.add_val(inst_info.ist_num_laps);
	batch_stat_solve_tm.add_val(full_tm);

	/*
	batch_avg_solve_time.add_val(full_tm);
	batch_total_solve_time += full_tm;

	if(full_tm > batch_max_solve_time){
		batch_max_solve_time = full_tm;
	}
	*/

	if(using_mem_ctrl){
		long tot_byt = MEM_STATS.num_bytes_available;
		MARK_USED(tot_byt);
		long by_in_use = MEM_STATS.num_bytes_in_use;

		batch_stat_mem_used.add_val(by_in_use);

		/*
		if(by_in_use > batch_max_mem_used){
			batch_max_mem_used = by_in_use;
		}
		batch_avg_mem_used.add_val(by_in_use);

		double perc_mem_used = mem_percent_used();
		if(perc_mem_used > batch_max_mem_used_perc){
			batch_max_mem_used_perc = perc_mem_used;
		}
		batch_avg_mem_used_perc.add_val(perc_mem_used);
		*/
	}

	DBG_PRT(4, os << "FINAL_CHOSEN=" << final_chosen_ids << std::endl
		<< "FINAL_ASSIG=" << final_assig << std::endl 
		<< std::endl);

	if(out_os != NULL_PT){
		PRT_OUT(1, print_stats(*out_os));
	}

	PRT_OUT(1, os << "FINISHING" << std::endl);

	inst_info.ist_solve_time = full_tm;

	satisf_val inst_res = inst_info.ist_result;

	if(inst_res == k_unknown_satisf){
		batch_num_unknown_satisf++;
	} else if(inst_res == k_yes_satisf){
		PRT_OUT(1, print_final_assig());
		final_assig.clear(true, true);	// So global memcheck says its OK.
		batch_num_yes_satisf++;
	} else if(inst_res == k_no_satisf){
		batch_num_no_satisf++;
	} else if(inst_res == k_timeout){
		batch_num_timeout++;
	} else if(inst_res == k_memout){
		batch_num_memout++;
	} else {
		BRAIN_CK(inst_res == k_error);
		batch_num_error++;
	}

	PRT_OUT(1, os << inst_info << std::endl);

	print_batch_consec();

	//PRT_OUT(0, os << " laps=" << inst_info.ist_num_laps; os.flush(););
}

//============================================================
// timer

bool	
timer::check_period(tmr_func_t tmr_fn, void* pm_fn){
	if(! tmr_has_timeout && ! tmr_is_periodic){
		return false;
	}

	tmr_current_cycle++;
	
	bool is_over = false;
	bool is_period = (tmr_current_cycle >= tmr_cycles);

	if(is_period || tmr_force_check){
		tmr_force_check = false;
		double tmr_current_secs = run_time();
		if(tmr_first_cycle){
			tmr_first_cycle = false;
			SUPPORT_CK_0(tmr_start_secs == 0.0);
			tmr_start_secs = tmr_current_secs;
			tmr_last_secs = tmr_current_secs;
		}
		SUPPORT_CK_0(tmr_current_secs >= tmr_start_secs);

		tmr_current_cycle = 0;
		double real_period = (tmr_current_secs - tmr_last_secs);
		tmr_last_secs = tmr_current_secs;

		if(real_period < tmr_desired_secs){
			tmr_cycles *= 2;
		} else if(tmr_cycles > 2.0){
			tmr_cycles /= 2;
		}
		SUPPORT_CK_0(tmr_cycles >= 1.0);

		double elpsd_tm = elapsed_time(tmr_current_secs);
		if(! tmr_reporting && 
			(elpsd_tm > tmr_desired_secs))
		{
			tmr_reporting = true;
		}

		if(tmr_has_timeout){
			is_over = (elpsd_tm > tmr_desired_timeout);
		}

		bool report_prd = tmr_reporting && ! is_over && 
			tmr_is_periodic && 
			(	! tmr_force_check || 
				(real_period >= tmr_desired_secs));

		if(report_prd && (tmr_fn != NULL_PT)){
			(*tmr_fn)(pm_fn, tmr_current_secs);
		}
	}
	return is_over;
}

//============================================================
// stats

std::ostream&
solver::print_mini_stats(std::ostream& os){
	instance_info& inst_info = get_curr_inst();
	std::string f_nam = inst_info.get_f_nam();

	os << CARRIAGE_RETURN;
	os << "'" << f_nam << "'";
	dbg_print_cond_func(get_dbg_info(), true, false, "NO_NAME", 0, "true", INVALID_DBG_LV);
	return os; 
}

std::ostream&
solver::print_stats(std::ostream& os, double current_secs){
	DBG_COMMAND(35, return os);

	DBG_COMMAND(36,
		print_mini_stats(os);
		return os;
	);

	if(current_secs == 0.0){
		current_secs = run_time();
	}

	instance_info& inst_info = get_curr_inst();
	std::string f_nam = inst_info.get_f_nam();

	os << std::endl;
	os << "file_name: '" << f_nam << "'" << std::endl;
	
	dbg_print_cond_func(get_dbg_info(), true, false, "NO_NAME", 0, "true", INVALID_DBG_LV);
	print_totals(os, current_secs);
	os << std::endl << std::endl;

	os.flush();

	return os;
}

//============================================================
// global functions

bool	dbg_print_cond_func(debug_info* dbg_info, bool prm, bool is_ck, const std::string fnam, int lnum,
		const std::string prm_str, long dbg_lv)
{
	bool aux_cy = false;
	bool& bad_cy = (dbg_info != NULL)?(dbg_info->dbg_bad_cycle1):(aux_cy);
	if(bad_cy){
		throw top_exception(prx_bad_cicle_1, "Recursive call to dbg_print_cond_func !!!!");
	}
	bad_cy = true;
	
	solver* slv = NULL;
	if(dbg_info != NULL){ slv = dbg_info->dbg_slv; }

	bool resp = true;
	if(prm){
		std::ostream& os = t_dbg_os;
		if(dbg_lv != INVALID_DBG_LV){
			if(dbg_lv == DBG_ALL_LVS){
				os << "ckALL.";
			} else {
				os << "ck" << dbg_lv << ".";
			}
		}
		if(slv != NULL){ 
			if(slv->batch_num_files > 1){
				os << "#" << slv->batch_consec << ".";
			}

			long the_lap = slv->get_curr_lap();
			if(the_lap > 0){
				if(is_ck){ os << "LAP="; }
				os << the_lap << ".";
			}
		}

		if(is_ck){
			os << "ASSERT '" << prm_str << "' FAILED (";
			if(slv != NULL){ os << slv->get_curr_f_nam(); }
			os << ")";
			os << " in " << fnam << " at " << lnum;
			os << std::endl;
		}
		os.flush();
		resp = (! is_ck);
	}

	bad_cy = false;
	return resp;
}

void	
solver::log_message(const std::ostringstream& msg_log){
	solver& slv = *this;

	if(! slv.batch_log_on){
		return;
	}

	if(slv.batch_log_name.size() == 0){
		return;
	}

	const char* log_nm = slv.batch_log_name.c_str();
	std::ofstream log_stm;
	log_stm.open(log_nm, std::ios::app);
	if(log_stm.good() && log_stm.is_open()){
		log_stm << msg_log.str() << std::endl; 
		log_stm.close();
	}
}

void	
solver::log_batch_info(){
	solver& slv = *this;
	
	if(! slv.batch_log_on){
		return;
	}

	if(slv.batch_end_log_name.size() == 0){
		return;
	}

	std::ostringstream msg_log;

	const char* log_nm = slv.batch_end_log_name.c_str();
	std::ofstream log_stm;
	log_stm.open(log_nm, std::ios::app);
	if(! log_stm.good() || ! log_stm.is_open()){
		slv.reset_err_msg();
		slv.error_stm << "Could not open file " << log_nm << ".";
		msg_log << slv.error_stm.str();
		std::cerr << msg_log.str() << std::endl;
		log_message(msg_log);
		return;
	}

	slv.batch_instances.print_row_data(log_stm, false, "\n");
	log_stm << std::endl; 
	log_stm.close();

	if(slv.batch_end_msg_name.size() == 0){
		return;
	}

	const char* msg_nm = slv.batch_end_msg_name.c_str();
	std::ofstream msg_stm;
	msg_stm.open(msg_nm, std::ios::app);
	if(! msg_stm.good() || ! msg_stm.is_open()){
		slv.reset_err_msg();
		slv.error_stm << "Could not open file " << msg_nm << ".";
		msg_log << slv.error_stm.str();
		std::cerr << msg_log.str() << std::endl;
		log_message(msg_log);
		return;
	}

	slv.print_totals(msg_stm);
	slv.print_final_totals(msg_stm);

}

void	chomp_string(std::string& s1){
	long ii = s1.length() - 1;
	//while((ii > 0) && ! isalnum(s1[ii])){
	while((ii > 0) && ! isprint(s1[ii])){
		ii--;
	}
	s1.resize(ii + 1);
}

void	
solver::read_batch_file(t_string& fnam, row<instance_info>& names){
	solver& slv = *this;
	
	PRT_OUT(0, os << "Loading batch file '" 
		<< fnam << std::endl); 

	std::ifstream in_stm;
	in_stm.open(fnam.c_str());

	if(! in_stm.good() || ! in_stm.is_open()){
		slv.reset_err_msg();
		slv.error_stm << "Could not open file " << fnam << ".";

		throw file_exception(flx_cannot_open, fnam);
	}

	names.set_cap(1000);
	names.clear();
	bool adding = true;

	std::string my_str;

	while(in_stm.good()){
		std::getline(in_stm, my_str);
		chomp_string(my_str);

		if(my_str == "/*"){
			adding = false;
		}
		if(adding && ! my_str.empty() && (my_str[0] != '#')){
			instance_info& ist = names.inc_sz();
			ist.ist_file_path = my_str;
			if(names.size() % 1000 == 0){
				PRT_OUT(0, os << CARRIAGE_RETURN << names.size());
				//PRT_OUT(0, os << ".");
			}
		}
		if((! adding) && (my_str == "*/")){
			adding = true;
		}
	} 

	PRT_OUT(0, os << CARRIAGE_RETURN << names.size());

	if(in_stm.bad()){
		std::cerr << in_stm.rdstate() << std::endl;
	}

	in_stm.close();
	PRT_OUT(0, 
		os << std::endl
		<< "Read " << names.size() << " file names." << std::endl
		<< "Processing:" << std::endl
	);
}

void	print_periodic_totals(void* pm, double curr_tm){
	solver* pt_slv = (solver*)pm;
	if(pt_slv == NULL){
		throw solver_exception(slx_no_solver, "Called timer func without solver");
	}
	solver& slv = *pt_slv;
	
	MARK_USED(pm);
	PRT_OUT(0, slv.print_totals(os, curr_tm));
}

void	get_enter(std::ostream& os, char* msg){
	os << "PRESS ENTER to continue. " << msg << std::endl;
	getchar();
}

t_string
solver::remove_log(const char* sufix){
	std::ostringstream log_ss;
	log_ss << input_file_nm << "_";
	log_ss << sufix;
	
	t_string lgnam = log_ss.str();
	remove(lgnam.c_str());
	return lgnam;
}

void	
solver::do_all_instances(debug_info& dbg_inf){
	batch_start_time = run_time();
	batch_prt_totals_timer.init_timer(PRINT_TOTALS_PERIOD);

	batch_log_name = remove_log("error.log");
	batch_end_log_name = remove_log("results.log");
	batch_end_msg_name = remove_log("stats.log");
	batch_answer_name = remove_log("assigs.log");

	row<instance_info>& all_insts = batch_instances;
	SUPPORT_CK_0(all_insts.is_empty());
	if(batch_name.size() > 0){
		read_batch_file(batch_name, all_insts);
	} else if(input_log_name.size() > 0){
		read_log_file(input_log_name, all_insts);
	} else {
		instance_info& ist = all_insts.inc_sz();
		ist.ist_file_path = input_file_nm;
	} 

	batch_num_files = all_insts.size();

	print_op_cnf();

	for(long ii = 0; ii < batch_num_files; ii++){
		batch_consec = ii + 1;
		std::string inst_nam = all_insts[ii].ist_file_path;
		//file_name = all_insts[ii].ist_file_path;

		std::ifstream i_ff;
		if(inst_nam.size() > 0){
			batch_prt_totals_timer.
				check_period(print_periodic_totals, this);

			MEM_CTRL(mem_size mem_in_u = MEM_STATS.num_bytes_in_use;)
			MEM_CTRL(MARK_USED(mem_in_u));

			do_cnf_file(dbg_inf);

			SUPPORT_CK_0(mem_in_u == MEM_STATS.num_bytes_in_use);
		}
	}

	print_op_cnf();

	batch_end_time = run_time();

	log_batch_info();
	//PRT_OUT(0, log_batch_info());

	all_insts.clear(true, true);

}

void
solver::set_input_kind(){
	if(input_file_nm.size() == 0){
		return;
	}

	bool is_batch = false;
	bool is_log = false;
	int f_ln = input_file_nm.size();
	if(f_ln > 4){
		const char* f_nm = input_file_nm.c_str();
		const char* f_ext = (f_nm + f_ln - 4);
		if(strcmp(f_ext, ".lst") == 0){
			is_batch = true;
		}
		if(strcmp(f_ext, ".log") == 0){
			is_log = true;
		}
	}

	if(is_batch){
		SUPPORT_CK(batch_name.size() == 0);
		batch_name = input_file_nm;
	}
	if(is_log){
		input_log_name = input_file_nm;
		op_cnf_id = fo_ck_assig;
	}
}

bool
solver::get_args(int argc, char** argv)
{
	std::ostream& os = std::cout;
	MARK_USED(os);

	for(long ii = 1; ii < argc; ii++){
		std::string the_arg = argv[ii];
		if(the_arg == "-h"){
			prt_help = true;
		} else if(the_arg == "-v"){
			prt_version = true;
		} else if(the_arg == "-t"){
			prt_headers = true;
		} else if(the_arg == "-s"){
			silent = true;
		} else if(the_arg == "-j"){
			just_read = true;
		} else if(the_arg == "-ck"){
			op_ck_satisf = true;
		} else if(the_arg == "-no_learning"){
			op_dbg_no_learning = true;
		} else if((the_arg == "-o") && ((ii + 1) < argc)){ 
			int kk_idx = ii + 1;
			ii++;

			output_file_nm = argv[kk_idx];

		} else if((the_arg == "-join_cnfs") && ((ii + 1) < argc)){
			op_cnf_id = fo_join;

			int kk_idx = ii + 1;
			ii++;

			op_cnf_f_nam = argv[kk_idx];

		} else if(the_arg == "-as_3cnf"){
			op_cnf_id = fo_as_3cnf;
		} else if(the_arg == "-as_ttnf"){
			op_cnf_id = fo_as_ttnf;
		} else if(the_arg == "-shuffle_cnf"){
			op_cnf_id = fo_shuffle;
		} else if(the_arg == "-tq"){
			op_cnf_id = fo_simplify;

		} else if((the_arg == "-laps") && ((ii + 1) < argc)){

			int kk_idx = ii + 1;
			ii++;

			dbg_num_laps = atol(argv[kk_idx]);

		} else if((the_arg == "-m") && ((ii + 1) < argc)){
			int kk_idx = ii + 1;
			ii++;

			long max_mem = atol(argv[kk_idx]);
			max_mem = (max_mem * NUM_BYTES_IN_KBYTE);
			if(max_mem > 0){
				MEM_STATS.num_bytes_available = max_mem;
			}

		} else if(input_file_nm.size() == 0){
			input_file_nm = argv[ii];
		}
	}

	if(prt_help){
		os << argv[0] << " " << help_str << std::endl;
		return false;
	}
	if(prt_version){
		os << argv[0] << " " << version_str << std::endl;
		return false;
	}
	if(prt_headers){
		instance_info::print_headers(os);
		os << std::endl;
		return false;
	}
	if(input_file_nm.size() == 0){
		os << argv[0] << " " << help_str << std::endl;
		os << argv[0] << " " << version_str << std::endl;
		return false;
	}

	set_input_kind();

	return true;
}

int	sat3_main(int argc, char** argv){
	//solver& slv = GLB;
	solver slv;

	DBG(std::cout << "FULL_DEBUG is defined" << std::endl);

	debug_info dbg_inf;
	MARK_USED(dbg_inf);
	debug_info* pt_dbg_inf = &dbg_inf;
	MARK_USED(pt_dbg_inf);
	
	slv.sl_dbg_info = &dbg_inf;
	dbg_inf.dbg_slv = &slv;

	//SUPPORT_CK_1(pt_dbg_inf, ((std::cout << "doing CKs" << std::endl) && true));
	//SUPPORT_CK_1(pt_dbg_inf, ((std::cout << "doing CK_1s" << std::endl) && true));
	//SUPPORT_CK_1(pt_dbg_inf, ((std::cout << "doing CK_2s" << std::endl) && true));
	
	SUPPORT_CK_1(pt_dbg_inf, (sizeof(t_1byte) == 1));
	SUPPORT_CK_1(pt_dbg_inf, (sizeof(long) == sizeof(void*)));

	int resp = 0;

	SUPPORT_CK_1(pt_dbg_inf, (dbg_inf.dbg_start_dbg_entries.get_cap() == 0));
	SUPPORT_CK_1(pt_dbg_inf, (dbg_inf.dbg_stop_dbg_entries.get_cap() == 0));

	MEM_CTRL(mem_size mem_in_u = MEM_STATS.num_bytes_in_use;)
	MEM_CTRL(MARK_USED(mem_in_u));

	MEM_STATS.num_bytes_available = (get_free_mem_kb() * NUM_BYTES_IN_KBYTE);

	bool args_ok = slv.get_args(argc, argv);

	double start_tm = 0.0;
	MARK_USED(start_tm);
	start_tm = run_time();

	DBG(dbg_inf.init_dbg_conf());

	if(args_ok){
		PRT_OUT(1, os << ".STARTING AT " << run_time() << std::endl);

		double tot_mem = (double)(MEM_STATS.num_bytes_available);
		slv.batch_stat_mem_used.vs_max_val = tot_mem;

		PRT_OUT(0, os << "Starting with "
			<< MEM_STATS.num_bytes_available << " bytes available" 
			<< std::endl); 
		DBG(PRT_OUT(1, os << "DEBUG_BRAIN activated" 
			<< std::endl));
		slv.do_all_instances(dbg_inf);

		PRT_OUT(1, os << ".ENDING AT " << run_time() << std::endl);
	}

	dbg_inf.dbg_start_dbg_entries.clear(true, true);
	dbg_inf.dbg_stop_dbg_entries.clear(true, true);

	SUPPORT_CK_1(pt_dbg_inf, (mem_in_u == MEM_STATS.num_bytes_in_use));

	double end_tm = 0.0;
	MARK_USED(end_tm);
	if(args_ok){
		//end_tm = run_time();
		PRT_OUT(0, 
			slv.print_totals(os);
			slv.print_final_totals(os);
		);

		DO_FINAL_GETCHAR;
	}

	//SUPPORT_CK_1(pt_dbg_inf, ((std::cout << "doing CKs" << std::endl) && true));
	//SUPPORT_CK_1(pt_dbg_inf, ((std::cout << "doing CK_1s" << std::endl) && true));
	//SUPPORT_CK_1(pt_dbg_inf, ((std::cout << "doing CK_2s" << std::endl) && true));
	DBG(std::cout << "FULL_DEBUG is defined" << std::endl);

	return resp;
}


int	main(int argc, char** argv){
	//int rr = tests_main_(argc, argv);
	int rr = 0;
	try{
		rr = sat3_main(argc, argv);
	} catch (...) {
		std::cerr << "INTERNAL ERROR !!! (sat3_main)" << "\n";
		std::cerr << STACK_STR << "\n";
		abort_func(0);
	}
	return rr;
}

/*

==============================================================
method pointers

DECL example
char (the_class::*the_method_pt)(int param1);

ASSIG example
the_method_pt = &the_class::a_method_of_the_class;

CALL Example
char cc = (an_object_of_the_class.*the_method_pt)(3);


Note that "::*", ".*" and "->*" are actually 
separate C++ operators.  

	reason, reasoning, reasoned, reasoner

		inductive, induce, induced, inducing, inducible

		deductive, deduce, deduced, deducing, deducible

		learn, learned, learning, learnable, learner

		specify, specified, specifying, specifiable, 
		specifier, specifies, specification

	study, studying, studied, studiable, studies

		analytic, analyze, analysed, analysing, analyzable
		analyser, analysis

		synthetic, synthesize, synthesized, synthesizing, 
		synthesizer, synthesis

		memorize, memorized, memorizing, memorizable, memorizer

		simplify, simplified, simplifying, simplificative, 
		simplifier, simplifies, simplification


	specify induce deduce simplify learn analyze synthesize

*/

void	
solver::read_log_file(t_string fnam, row<instance_info>& names){
	std::ifstream flog(fnam.c_str());
	if(! flog.is_open()){
		std::cerr << "Could not open file " << fnam << "\n";
		throw file_exception(flx_cannot_open, fnam);
		return;
	}
	
	names.set_cap(1000);
	names.clear();
	
	t_string lin;
	while(std::getline(flog, lin)){
		if(lin.size() == 0){
			continue;
		}
		t_istr_stream lis(lin);
		t_string tok;
		
		instance_info& ist = names.inc_sz();
		int num_tok = 0;
		while(std::getline(lis, tok, '|')){
			//std::cout << "TOK= " << tok << "|";
			if(num_tok == 0){
				ist.ist_file_path = tok;
			}
			if(num_tok == 1){
				ist.ist_ck_sha = tok;
			}
			if(num_tok == 2){
				ist.ist_ck_result = result_str_to_val(tok);
			}
			if(num_tok == 8){
				tok += "\n";
				parse_assig(tok, ist.ist_ck_assig);
			}
			num_tok++;
		}
		std::cout << ist << "\n";
	}
	flog.close();
}

void 
parse_assig(t_string& assig, row_long_t& lits){
	const char* pt_in = assig.c_str();
	
	long num_ln = 0;

	if(*pt_in != '\n'){
		skip_whitespace(pt_in, num_ln);
		while(*pt_in != '\n'){
			long val = parse_int(pt_in, num_ln); 
			if(val == 0){ break; }
			
			skip_whitespace(pt_in, num_ln);	
			lits.push(val);
		}
	} else {
		skip_line(pt_in, num_ln);
	}	
}

