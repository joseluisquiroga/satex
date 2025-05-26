

/*************************************************************

bible_sat

config.h
(C 2025) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com

Declaration of functions to read and parse config files.

--------------------------------------------------------------*/

#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <fstream>
#include <sstream>

#include "tools.h"

// '\0'
#define END_OF_SEC	0

#define OUT_NUM_LEVS 10

#define DBG_NUM_LEVS 200


enum config_exception_code { 
	k_config_01_exception = k_last_tool_exception,
	k_config_02_exception,
	k_config_03_exception,
	k_config_04_exception,
	k_config_05_exception,
	k_config_06_exception,
	k_config_07_exception,
	k_config_08_exception,
	k_config_09_exception,
	k_last_config_exception
};


#define CONFIG_DBG(prm)	DBG(prm)
#define CONFIG_CK(prm) 	DBG_CK(prm)

//=================================================================
// parser funcs

t_string get_parse_err_msg(const char* hd_msg, long num_line, char ch_err, const char* msg);

void skip_whitespace(const char*& pt_in, long& line);
void skip_line(const char*& pt_in, long& line);
integer parse_int(const char*& pt_in, long line);


//=================================================================
// debug_entry

class debug_entry {
	public:
	long		dbg_round;
	long		dbg_id;

	debug_entry(){
		dbg_round = -1;
		dbg_id = -1;
	}

	~debug_entry(){
	}

	std::ostream& 	print_debug_entry(std::ostream& os);

};

inline
comparison	cmp_dbg_entries(debug_entry const & e1, debug_entry const & e2){
	return cmp_long(e1.dbg_round, e2.dbg_round);
}

//=================================================================
// debug_info

class brain;
class global_data;

class debug_info {
	public:
	
	global_data*	dbg_slv;
	brain*			dbg_brn;
	
	bool			dbg_skip_print_info;	
		
	row<debug_entry>	dbg_start_dbg_entries;
	row<debug_entry>	dbg_stop_dbg_entries;
	long			dbg_current_start_entry;
	long			dbg_current_stop_entry;
	row<bool>		dbg_lev;
	bool 		dbg_bad_cycle1;

	bool			dbg_ic_active;
	long			dbg_ic_max_seq;
	long			dbg_ic_seq;
	bool			dbg_ic_after;
	bool			dbg_ic_gen_jpg;
	
	debug_info(){
		init_debug_info();
	}

	~debug_info(){
	}
	
	void init_debug_info(){
		dbg_slv = NULL;
		dbg_brn = NULL;
		
		dbg_skip_print_info = false;
		
		dbg_start_dbg_entries.clear();
		dbg_stop_dbg_entries.clear();
		dbg_current_start_entry = 0;
		dbg_current_stop_entry = 0;
		dbg_lev.fill(false, DBG_NUM_LEVS);
		dbg_bad_cycle1 = false;

		dbg_ic_active = false;
		dbg_ic_max_seq = -1;
		dbg_ic_seq = 0;
		dbg_ic_after = false;
		dbg_ic_gen_jpg = false;
	}

	debug_info*	get_dbg_info(){
		return this;
	}	

	void	init_dbg_conf();
		
	void	dbg_lv_on(long lv_idx){
		CONFIG_CK(dbg_lev.is_valid_idx(lv_idx));
		dbg_lev[lv_idx] = true;
	}

	void	dbg_lv_off(long lv_idx){
		CONFIG_CK(dbg_lev.is_valid_idx(lv_idx));
		dbg_lev[lv_idx] = false;
	}
	
	t_ostream&	print_debug_info(t_ostream& os, bool from_pt = false){
		os << " dbg_start_dbg_entries=\n";
		dbg_start_dbg_entries.print_row_data(os, true, "\n");
		os << " dbg_stop_dbg_entries=\n";
		dbg_stop_dbg_entries.print_row_data(os, true, "\n");
		os << " num_lvs=" << dbg_lev.size();
		return os;
	}

	void dbg_update_config_entries();
};

//=================================================================
// config_reader

class config_reader {
	public:
		
	row<long> dbg_config_line;

	config_reader(){
	}

	~config_reader(){}

	void 	parse_debug_line(row<long>& dbg_line, std::string& str_ln);
	void	add_config_line(debug_info& dbg_inf, std::string& str_ln);
	void	read_config(debug_info& dbg_inf, const char* f_name);
};

bool dbg_lev_ok(debug_info* dbg_info, long lev);


#endif		// CONFIG_H

