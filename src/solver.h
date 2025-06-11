

/*************************************************************

satex

solve.h
(C 2025) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com

Declaration of solver class and classes that assist the system.

--------------------------------------------------------------*/

#ifndef SUPPORT_H
#define SUPPORT_H

//=================================================================
// configuration defs

//define DO_GETCHAR	
#define DO_GETCHAR			getchar();
#define DO_FINAL_GETCHAR
//define DO_FINAL_GETCHAR		DO_GETCHAR
//define SOLVING_TIMEOUT		10.5	// 0.0 if no timeout
#define SOLVING_TIMEOUT			0.0		// 0.0 if no timeout
#define RECEIVE_TIMEOUT			10.0	// for timed_receive
#define PRINT_PROGRESS			true	// or if (SOLVING_TIMEOUT > 1.0)
#define PRINT_PERIOD			4.0
#define PRINT_TOTALS_PERIOD 		10.0
#define MAX_CONFLICTS			0		// all
#define RESULT_FIELD_SEP			"|"


//=================================================================
// includes

#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <map>
#include <iterator>

#include <gmpxx.h>

#include "tools.h"
#include "config.h"
#include "dimacs.h"

#define GET_RUN_TIME() cpu_time()

//=================================================================
// pre-configuration decl

extern bool	dbg_bad_cycle1;

//=================================================================
// debug defs

#define DBG_ALL_LVS -1

#define DO_PRINTS(prm)		prm
#define CARRIAGE_RETURN		((char)13)

#define	DBG_LV_COND(lev, cond)	\
	( dbg_lev_ok(get_dbg_info(), lev) && (cond) ) \

//--end_of_def


#define	DBG_COND(lv_arr, lev, cond)	\
	(	(	(lev < 0) || \
			((lev >= 0) && lv_arr[lev]) \
		) && \
		(cond) \
	) \

//--end_of_def

#define PRT_OUT(lev, comm) \
	DO_PRINTS( \
		if(! slv.silent && DBG_COND(slv.out_lev, lev, \
				(slv.out_os != NULL_PT))) \
		{ \
			std::ostream& os = *(slv.out_os); \
			comm; \
			os.flush(); \
		} \
	) \
			
//--end_of_def

#define INVALID_DBG_LV 		-123

bool	dbg_print_cond_func(debug_info* dbg_info, bool prm,
		bool is_ck = false,
		const std::string fnam = "NO_NAME",
		int lnum = 0,
		const std::string prm_str = "NO_PRM",
		long dbg_lv = INVALID_DBG_LV);

#define	DBG_PRT_COND(lev, cond, comm)	\
	DBG( \
		dbg_print_cond_func(get_dbg_info(), DBG_LV_COND(lev, cond), \
			false, "NO_NAME", 0, #cond, lev); \
		if(DBG_LV_COND(lev, cond)){ \
			std::ostream& os = t_dbg_os; \
			comm; \
			os << std::endl; \
			os.flush(); \
		} \
	) \

//--end_of_def
	
#define	DBG_PRT(lev, comm)	DBG_PRT_COND(lev, true, comm)

#define	DBG_COMMAND(lev, comm) \
		if(DBG_LV_COND(lev, true)){ \
			std::ostream& os = t_dbg_os; \
			DBG(comm); \
			os.flush(); \
		} \

//--end_of_def

#define SUPPORT_CK_0(prm) DBG_CK(prm)

// end_of_def

#define SUPPORT_CK_1(dbinf, prm) \
	DBG_CK(dbg_print_cond_func(dbinf, (! (prm)), true, __FILE__, __LINE__, #prm)); \

// end_of_def

#define SUPPORT_CK(prm) \
	DBG_CK(dbg_print_cond_func(get_dbg_info(), (! (prm)), true, __FILE__, __LINE__, #prm)); \

// end_of_def

#define BRAIN_CK_0(prm) DBG_CK(prm)
//define BRAIN_CK_0(prm)	SUPPORT_CK(prm)

#define BRAIN_CK(prm) \
	DBG_CK(dbg_print_cond_func(get_dbg_info(), (! (prm)), true, __FILE__, __LINE__, #prm)); \

// end_of_def

#define BRAIN_CK_1(prm)	;
//define BRAIN_CK_1(prm)	BRAIN_CK(prm)

#define BRAIN_CK_2(prm)	;
//define BRAIN_CK_2(prm)	BRAIN_CK(prm)

#define DBG_SLOW(prm)
//define DBG_SLOW(prm)	DBG(prm)

#define init_nams(nams, fst_idx, lst_idx) \
	for(long nams##kk = fst_idx; nams##kk < lst_idx; nams##kk++){ \
		nams[nams##kk] = "invalid_name !!!"; \
	} \

// end_define


//=================================================================
// typedefs and defines

class brain;

typedef	int	location;

#define INVALID_QUANTON_ID	0
#define INVALID_POLARITY	0
#define INVALID_LAYER		-1

//=================================================================
// consecutive

typedef long 	consecutive_t;
//typedef mpz_class 	consecutive_t;

#define INVALID_CONSECUTIVE	-1
#define MAX_CONSECUTIVE		std::numeric_limits<double>::max()


//======================================================================
// solver_exception

typedef enum {
	slx_no_solver,
} sl_ex_cod_t;

class solver_exception : public top_exception {
public:
	
	solver_exception(long the_id = 0, t_string ff = "error_in_solver") : top_exception(the_id)
	{
	}

	virtual t_string name(){ t_string nm = "solver_exception"; return nm; }
};

//=================================================================
// average

class average {
public:
	double	avg;
	integer	sz;		// the number of avg numbers
	integer	max_sz;	// max num of vals

	average(integer maxx = MAX_INTEGER){
		avg = 0;
		sz = 0;
		max_sz = maxx;
	}

	void	add_val(double val){
		if(sz >= max_sz){ return; }
		if(sz == 0){ sz = 1; avg = val; return; }
		avg = (avg * ((double)sz / (sz + 1))) + (val / (sz + 1));
		sz++;
	}

	void	remove_val(double val){
		if(sz == 1){ sz = 0; avg = 0; return; }
		avg = (avg * ((double)sz / (sz - 1))) - (val / (sz - 1));
		sz--;
	}

	std::ostream&	print_average(std::ostream& os){
		os << "avg:" << avg << " sz:" << sz << " ";
		os.flush();
		return os;
	}

};

inline
std::ostream& operator << (std::ostream& os, average& obj){
	return obj.print_average(os);
}

//=================================================================
// avg_stat

class avg_stat : private average {
public:
	double		vs_tot_val;
	double		vs_max_val;
	std::string	vs_nam;

	avg_stat(std::string nam = "avg?", integer maxx = MAX_INTEGER) : average (maxx) {
		vs_tot_val = 0.0;
		vs_max_val = 0.0;
		vs_nam = nam;
	}

	void	add_val(double val){
		vs_tot_val += val;
		average::add_val(val);
		if(val > vs_max_val){ vs_max_val = val; }
	}

	void	remove_val(double val){
		vs_tot_val -= val;
		average::remove_val(val);
	}

	std::ostream&	print_avg_stat(std::ostream& os);

};

inline
std::ostream& operator << (std::ostream& os, avg_stat& obj){
	return obj.print_avg_stat(os);
}


//=================================================================
// timer

inline 
double	run_time(){
	return GET_RUN_TIME();
}


#define MIN_TIMER_PERIOD	0.0
#define MIN_TIMER_TIMEOUT	0.0

typedef void (*tmr_func_t)(void* pm, double curr_secs);

class timer {
public:
	bool			tmr_reporting;
	double			tmr_start_secs;
	double			tmr_last_secs;
	double			tmr_desired_secs;
	double			tmr_desired_timeout;

	double			tmr_cycles;
	double			tmr_current_cycle;

	bool			tmr_force_check;

	bool			tmr_first_cycle;

	bool			tmr_is_periodic;
	bool			tmr_has_timeout;

	timer(double period = 4.0, double tm_out = 0.0){
		init_timer(period, tm_out);
	}

	void	init_timer(double period = 4.0, double tm_out = 0.0){
		tmr_reporting = false;
		tmr_start_secs = 0.0;
		tmr_last_secs = 0.0;
		tmr_desired_secs = period;
		tmr_desired_timeout = tm_out;

		tmr_cycles = 1;
		tmr_current_cycle = 0;

		tmr_force_check = false;

		tmr_first_cycle = true;

		tmr_is_periodic = (tmr_desired_secs > MIN_TIMER_PERIOD);
		tmr_has_timeout = (tmr_desired_timeout > MIN_TIMER_TIMEOUT);
	}

	double	elapsed_time(double current_secs = 0.0){
		if(current_secs == 0.0){
			current_secs = run_time();
		}
		double s_time = (current_secs - tmr_start_secs);
		return s_time;
	}

	double	period_time(double current_secs = 0.0){
		if(current_secs == 0.0){
			current_secs = run_time();
		}
		double p_time = (current_secs - tmr_last_secs);
		return p_time;
	}

	bool	check_period(tmr_func_t fn = NULL_PT, void* pm = NULL_PT);
};

//=================================================================
// instance_info

enum satisf_val {
	k_unknown_satisf,
	k_yes_satisf,
	k_no_satisf,
	k_timeout,
	k_memout,
	k_error
};

extern std::string	satisf_val_nams[];

#define k_first_satisf_val		k_unknown_satisf
#define k_last_satisf_val		(k_error + 1)

class instance_info {
public:
	std::string		ist_file_path;
	std::string		ist_file_sha;
	double			ist_solve_time;
	satisf_val		ist_result;
	long			ist_num_vars;
	long			ist_num_ccls;
	long			ist_num_lits;
	consecutive_t	ist_num_laps;
	std::string		ist_final_assig;
	satisf_val		ist_ck_result;
	std::string		ist_ck_sha;
	row_long_t		ist_ck_assig;

	instance_info(){
		ist_file_path = "Unknown_path";
		ist_file_sha = "Unknown_sha";
		ist_solve_time = 0.0;
		ist_result = k_unknown_satisf;
		ist_num_vars = 0;
		ist_num_ccls = 0;
		ist_num_lits = 0;
		ist_num_laps = 0;
		ist_ck_result = k_unknown_satisf;
		ist_ck_sha = "";
		ist_final_assig = "Invalid_final_assig";
	}

	std::string	get_f_nam(){
		return ist_file_path;
	}

	static
	std::ostream& 	print_headers(std::ostream& os);

	std::ostream& 	print_instance_info(std::ostream& os);

};

inline
std::ostream& operator << (std::ostream& os, instance_info& obj){
	return obj.print_instance_info(os);
}

//=================================================================
// solver

enum cnf_oper_t {
	fo_invalid,
	fo_join,
	fo_as_3cnf,
	fo_as_ttnf,
	fo_shuffle,
	fo_simplify,
	fo_ck_assig,
};

class quanton;
class solver;

//extern solver		GLB;

void	glb_set_memout();
long	get_free_mem_kb();

typedef	void 	(solver::*dbg_info_fn_t)();

class solver {
public:
	debug_info*		sl_dbg_info;
	
	bool			silent;

	bool			using_mem_ctrl;
	bool			prt_help;
	bool			prt_version;
	bool			prt_headers;

	std::string		help_str;
	std::string		version_str;

	bool			just_read;

	row<quanton*>		final_assig;
	row<long>		final_trail_ids;
	row<long>		final_chosen_ids;

	std::ostream*	out_os;
	row<bool>		out_lev;

	cnf_oper_t		op_cnf_id;
	std::string		op_cnf_f_nam;
	long			op_cnf_num;
	bool			op_ck_satisf;
	bool			op_dbg_no_learning;

	std::string		dbg_file_name;
	std::ofstream		dbg_file;

	row<long>		dbg_config_line;

	long			dbg_num_laps;

	std::ostringstream	error_stm;
	long			error_cod;

	std::string		input_file_nm;
	std::string		output_file_nm;

	std::string		input_log_name;
	
	bool			batch_log_on;
	std::string		batch_name;
	std::string		batch_log_name;
	std::string		batch_end_log_name;
	std::string		batch_end_msg_name;
	std::string		batch_answer_name;
	integer			batch_num_files;
	integer			batch_consec;
	integer			batch_num_unknown_satisf;
	integer			batch_num_yes_satisf;
	integer			batch_num_no_satisf;
	integer			batch_num_timeout;
	integer			batch_num_memout;
	integer			batch_num_error;

	avg_stat		batch_stat_choices;
	avg_stat		batch_stat_laps;
	avg_stat		batch_stat_added_fires;
	avg_stat		batch_stat_load_tm;
	avg_stat		batch_stat_solve_tm;
	avg_stat		batch_stat_mem_used;

	double			batch_start_time;
	double			batch_end_time;
	timer			batch_prt_totals_timer;

	row<instance_info>	batch_instances;

	row<std::string>	exception_strings;

	solver(){
		init_solver();
	}

	void 		init_solver();

	~solver(){
	}

	debug_info*	get_dbg_info(){
		return sl_dbg_info;
	}	

	void		reset_global(){
		reset_err_msg();
		error_cod = -1;
	}

	bool		get_args(int argc, char** argv);
	void		set_input_kind();

	instance_info&	get_curr_inst(){
		long batch_idx = batch_consec - 1;
		DBG_CK(batch_instances.is_valid_idx(batch_idx));
		instance_info& the_ans = batch_instances[batch_idx];
		return the_ans;
	}

	bool		doing_dbg(){
		bool in_dbg = false;
		MARK_USED(in_dbg);
		DBG(in_dbg = true);
		return in_dbg;
	}

	bool		in_valid_inst(){
		long batch_idx = batch_consec - 1;
		return (batch_instances.is_valid_idx(batch_idx));
	}

	consecutive_t	get_curr_lap(){
		if(batch_instances.is_empty()){
			return 0;
		}
		instance_info& inst_info = get_curr_inst();
		return inst_info.ist_num_laps;
	}

	satisf_val&	result(){
		instance_info& the_ans = get_curr_inst();
		return the_ans.ist_result;
	}

	void		init_paths();

	bool		is_finishing(){
		return (result() != k_unknown_satisf);
	}

	bool	is_here(location rk){
		MARK_USED(rk);
		return true;
	}

	void	init_exception_strings();
	t_string	remove_log(const char* sufix);

	void	dbg_default_info(){
		std::ostream& os = t_dbg_os;
	
		os << "NO DBG INFO AVAILABLE " << 
		"(define a func for this error code)" << std::endl; 
	}

	std::string	get_curr_f_nam(){
		if(batch_instances.is_empty()){
			return "UKNOWN FILE NAME";
		}
		instance_info& inst_info = get_curr_inst();
		return inst_info.get_f_nam();
	}

	std::ostream&	get_os(){
		if(out_os != NULL_PT){
			return *out_os;
		}
		return std::cout;
	}

	void	set_active_out_levs();

	void	reset_err_msg(){
		error_stm.clear();
		error_stm.str() = "";
		error_stm.flush();
	}

	double	mem_percent_used();

	void	print_final_assig();
	void	count_instance(instance_info& inst_info);

	void	log_message(const std::ostringstream& msg_log);
	void	log_batch_info();
	void 	read_log_file(t_string fnam, row<instance_info>& names);
	void	read_batch_file(t_string& fnam, row<instance_info>& names);
	void	do_all_instances(debug_info& dbg_inf);

	void	print_op_cnf();
	void	do_cnf_file(debug_info& dbg_inf);
	
	std::ostream&	test_open_out(std::ofstream& os);
	void		test_cnf_join();
	void		test_cnf_as_ttnf(bool smpfy_it);
	void		test_cnf_shuffle();
	void		test_tier_queue();
	void		test_simplify_cnf();

	void		call_solve_instance(debug_info& dbg_inf);
	void		do_instance(debug_info& dbg_inf);
	void		check_final_assig(debug_info& dbg_inf);
	
	//int	walk_neuron_tree(std::string& dir_nm);

	std::ostream&	print_mini_stats(std::ostream& os);
	std::ostream& 	print_stats(std::ostream& os, double current_secs = 0.0);

	std::ostream&	print_mem_used(std::ostream& os);
	std::ostream&	print_totals(std::ostream& os, double curr_tm = 0.0);
	std::ostream&	print_final_totals(std::ostream& os);
	void		print_batch_consec();

};

//=================================================================
// FUNCTION

inline
std::ostream& 	
instance_info::print_headers(std::ostream& os){
	std::string sep = RESULT_FIELD_SEP;
	os << "file_path" << sep;
	os << "solve_time" << sep;
	os << "result" << sep;
	os << "#vars" << sep;
	os << "#ccls" << sep;
	os << "#lits" << sep;
	os << "#laps" << sep;
	return os;
}

inline
std::ostream& 	
instance_info::print_instance_info(std::ostream& os){
	std::string sep = RESULT_FIELD_SEP;
	os << ist_file_path << sep;
	os << ist_file_sha << sep;
	os << satisf_val_nams[ist_result] << sep;
	os << ist_solve_time << sep;
	os << ist_num_vars << sep;
	os << ist_num_ccls << sep;
	os << ist_num_lits << sep;
	os << ist_num_laps << sep;
	os << ist_final_assig << sep;
	if(ist_ck_result != k_unknown_satisf){
		os << satisf_val_nams[ist_ck_result] << sep;
	}
	if(ist_ck_sha.size() > 0){
		os << ist_ck_sha << sep;
	}
	if(! ist_ck_assig.is_empty()){
		os << ist_ck_assig << sep;
	}
	return os;
}

inline
std::ostream& 	
debug_entry::print_debug_entry(std::ostream& os){
	os << "dbg(p=" << dbg_round << ", i=" << dbg_id << ")";
	return os;
}

inline
std::ostream& operator << (std::ostream& os, debug_entry& dbg_ety){
	return dbg_ety.print_debug_entry(os);
}

//=================================================================
// global functions

typedef void (*core_func_t)(void);

void	init_glb_nams();
satisf_val	result_str_to_val(t_string& res_str);

void		read_file(t_string f_nam, row<char>& f_data);
void		sha_bytes_of_arr(uchar_t* to_sha, long to_sha_sz, row<uchar_t>& the_sha);
t_string 	sha_txt_of_arr(uchar_t* to_sha, long to_sha_sz);

void 	parse_assig(t_string& assig, row_long_t& lits);

void	chomp_string(std::string& s1);
void	get_enter(std::ostream& os, char* msg);
int	tests_main_(int argc, char** argv);


#endif		// SUPPORT_H

