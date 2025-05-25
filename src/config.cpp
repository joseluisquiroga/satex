

/*************************************************************

bible_sat

config.cpp  
(C 2025) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com

Functions to read and parse config files.

--------------------------------------------------------------*/

#include <sys/stat.h>

#include <cassert>
#include <cstring>

#include "support.h"


t_string get_parse_err_msg(const char* hd_msg, long num_line, char ch_err, const char* msg){
	t_ostr_stream err_msg;

	err_msg << hd_msg;
	if(num_line >= 0){
		err_msg << "line " << num_line << ". ";
	}
	if(ch_err >= 0){
		err_msg << "invalid character '" << ch_err << "'. ";
	}
	if(msg != NULL_PT){
		err_msg << msg;
	}
	return err_msg.str();
}

void skip_whitespace(const char*& pt_in, long& line){
	while(	(! isalnum(*pt_in) || isspace(*pt_in)) && 
			(*pt_in != '-') && (*pt_in != '+') && (*pt_in != 0))
	{ 
		if(*pt_in == '\n'){ 
			line++; 
		}
		pt_in++; 
	}
}

void skip_line(const char*& pt_in, long& line){
	while(*pt_in != 0){
		if(*pt_in == '\n'){ 
			line++; 
			pt_in++; 
			return; 
		}
		pt_in++; 
	}
}

integer parse_int(const char*& pt_in, long line) {
	integer	val = 0;
	bool	neg = false;

	if(*pt_in == '-'){ neg = true; pt_in++; }
	else if(*pt_in == '+'){ pt_in++; }

	if( ! isdigit(*pt_in)){
		throw parse_exception(pax_bad_int, (char)(*pt_in), line);
	}
	while(isdigit(*pt_in)){
		val = val*10 + (*pt_in - '0');
		pt_in++;
	}
	return (neg)?(-val):(val);
}

void
config_reader::parse_debug_line(row<long>& dbg_line, std::string& str_ln){
	const char* pt_in = str_ln.c_str();

	dbg_line.clear();
	
	long num_ln = 0;

	if(isalnum(*pt_in)){
		skip_whitespace(pt_in, num_ln);
		while(isdigit(*pt_in)){
			long val = parse_int(pt_in, num_ln); 
			skip_whitespace(pt_in, num_ln);
	
			dbg_line.push(val);
		}
	} else {
		skip_line(pt_in, num_ln);
	}
}

void	
config_reader::add_config_line(debug_info& dbg_info, std::string& str_ln){
	std::ostream& os = std::cout;
	MARK_USED(os);
	row<long>& dbg_ln = dbg_config_line;
	parse_debug_line(dbg_ln, str_ln);

	//os << "Entendi:<<" << dbg_ln << ">>" << std::endl;

	if(! dbg_ln.is_empty()){
		debug_entry& start_dbg = dbg_info.dbg_start_dbg_entries.inc_sz();

		long debug_id = dbg_ln[0];
		start_dbg.dbg_id = debug_id;

		if(dbg_ln.size() > 1){
			start_dbg.dbg_round = dbg_ln[1];
		}

		if(dbg_ln.size() > 2){
			debug_entry& stop_dbg = dbg_info.dbg_stop_dbg_entries.inc_sz();
			stop_dbg.dbg_id = debug_id;
			stop_dbg.dbg_round = dbg_ln[2];
		}
	}
}

/*void
mini_test(){
	std::ostream& os = std::cout;
	os << "isalnum('/') " << isalnum('/') << std::endl;
}*/

void
config_reader::read_config(debug_info& dbg_info, const char* file_nm){
	std::ostream& os = std::cout;
	CONFIG_CK(file_nm != NULL_PT);

	dbg_info.dbg_start_dbg_entries.clear();
	dbg_info.dbg_stop_dbg_entries.clear();
	dbg_config_line.clear();

	//mini_test();

	std::ifstream in_stm;

	in_stm.open(file_nm, std::ios::binary);
	if(!in_stm.good() || !in_stm.is_open()){
		os << "NO " << file_nm << " FILE FOUND." << std::endl;
		return;
	}

	std::string str_ln;

	while(! in_stm.eof()){
		std::getline(in_stm, str_ln);
		
		//os << "Lei:<<" << str_ln << ">>" << std::endl;
		add_config_line(dbg_info, str_ln);
	}
	in_stm.close();

	dbg_config_line.clear(false, true);

	dbg_info.dbg_start_dbg_entries.mix_sort(cmp_dbg_entries);
	dbg_info.dbg_stop_dbg_entries.mix_sort(cmp_dbg_entries);

	//os << "start_dbgs=" << dbg_info.dbg_start_dbg_entries << std::endl;
	//os << "stop_dbgs=" << dbg_info.dbg_stop_dbg_entries << std::endl;
}

void
debug_info::dbg_update_config_entries(){
	std::ostream& os = std::cout;
	MARK_USED(os);

	if(GLB.get_curr_lap() >= (LONG_MAX - 1)){
		return;
	}

	long curr_round = (long)GLB.get_curr_lap();
	/*
	if(curr_round <= 0){
		return;
	}*/

	long& start_idx = dbg_current_start_entry;
	long& stop_idx = dbg_current_stop_entry;

	row<debug_entry>& start_lst = dbg_start_dbg_entries;
	row<debug_entry>& stop_lst = dbg_stop_dbg_entries;

	while(	(start_idx < start_lst.size()) && 
		(start_lst[start_idx].dbg_round <= curr_round))
	{
		long start_dbg_id = start_lst[start_idx].dbg_id;
		SUPPORT_CK(dbg_lev.is_valid_idx(start_dbg_id));
		dbg_lev[start_dbg_id] = true;
		start_idx++;
	} 

	while(	(stop_idx < stop_lst.size()) && 
		(stop_lst[stop_idx].dbg_round < curr_round))
	{
		long stop_dbg_id = stop_lst[stop_idx].dbg_id;
		SUPPORT_CK(dbg_lev.is_valid_idx(stop_dbg_id));
		dbg_lev[stop_dbg_id] = false;
		stop_idx++;
	} 
}



