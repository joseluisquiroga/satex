

/*************************************************************

satex

dimacs.h
(C 2025) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com

Declaration of functions to read and parse dimacs files.

--------------------------------------------------------------*/

#ifndef DIMACS_H
#define DIMACS_H

/*
#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif

#define _XOPEN_SOURCE 500	// used in <ftw.h> to define behaviour (see features.h)

#include <ftw.h>
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <set>

#include "config.h"
//include "sha2.h"
//include "kry.h"
#include "tak_mak.h"

#define DIMACS_DBG(prm)	DBG(prm)
//define DIMACS_CK(prm) 	DBG_CK(prm)
#define DIMACS_H_CK(prm) 	DBG_CK(prm)

#define MAX_DIMACS_HEADER_SZ 4096

typedef std::string::size_type str_pos_t;
typedef unsigned char		uchar_t;
typedef std::set<std::string> string_set_t;

extern std::string	k_dimacs_header_str;

//=================================================================
// funtion declarations

//void read_problem_decl(const char*& pt_in, long& num_var, long& num_ccl, long& line);
void print_dimacs_of(std::ostream& os, row<long>& all_lits, long num_cla, long num_var);

inline
long
get_var(long lit){
	return abs_long(lit);
}


//=================================================================
// dimacs_loader

enum added_var_t {
	av_orlit,
	av_andlit,
	av_last
};

enum fix_kind {
	fx_lit_already_in_clause,
	fx_clause_has_both_lit,
	fx_empty_clause,
	fx_clause_has_one_lit,
	k_last_fix
};

#define RANDOM_INIT	false
#define FIRST_LIT_IDX	1

#define EMPTY_CNF_COMMENT	"c the cnf is empty\n"
#define TOT_LITS_DECL	"c tot_lits="

typedef std::set<long>	long_set_t;

class dimacs_loader {
public:
	typedef std::istream::char_type ld_char_t;
	typedef std::istream::pos_type ld_pos_t;
	
	debug_info*			ld_dbg_info;

	std::string		ld_file_name;
	std::string		ld_file_sha;

	bool			ld_as_3cnf;

	long			ld_max_in_ccl_sz;

	long			ld_num_line;
	row<ld_char_t>		ld_content;
	const char*		ld_cursor;

	long			ld_decl_ccls;
	long			ld_decl_vars;
	long			ld_decl_lits;

	long			ld_parsed_ccls;
	long			ld_parsed_lits;
	long			ld_parsed_vars;
	
	long_set_t		ld_all_vars;

	// ---------------------------------------

	//nid_bits		ld_dots;

	row<integer>		ld_fixes;

	row_long_t 		ld_lits1;
	row_long_t 		ld_lits2;

	row_row_long_t 		ld_rr_lits1;
	row_row_long_t 		ld_rr_lits2;

	long			ld_last_or_lit;

	long			ld_nud_added_ccls;
	long			ld_nud_added_vars;
	long			ld_nud_added_lits;

	long			ld_num_ccls;
	long			ld_num_vars;
	long			ld_tot_lits;

	dimacs_loader(){
		init_dimacs_loader();
	}

	~dimacs_loader(){
		init_dimacs_loader();
	}

	debug_info*	get_dbg_info(){
		return ld_dbg_info;
	}	
	
	void	init_parse();
	void	init_dimacs_loader();

	void 	skip_dimacs_whitespace(const char*& pt_in, long& line);
	bool 	follows_int(const char*& pt_in);
	
	void 	read_problem_decl(const char*& pt_in, long& num_var, long& num_ccl, long& line);
	void 	skip_cnf_decl(const char*& pt_in, long line);
	
	void	load_file(t_string& f_nam);
	void	parse_header();
	bool	parse_clause(row<integer>& lits);

	void	verif_num_ccls(std::string& f_nam, long num_decl_ccls, long num_read_ccls);
	void	verif_tot_lits(std::string& f_nam, long tot_decl_lits, long tot_read_lits);

	bool	fix_lits(row_long_t& lits, bool& add_it);

	bool	fixappend_lits(row_long_t& lits, row_long_t& prepared);

	void	add_lits(row_long_t& lits, row_long_t& prepared, bool is_orig);
	void	append_rr_lits(row_row_long_t& rr_lits, row<long>& prepared);

	void	three_lits(row_long_t& lits, row_long_t& prepared);

	void	lits_opps(row<long>& r_lits);

	void	calc_f_lit_equal_or(long d_nio, row_long_t& or_lits,
			row_row_long_t& rr_nios);

	void	calc_f_lit_equal_and(long d_nio, row_long_t& and_lits,
			row_row_long_t& rr_nios);

	void	parse_all_ccls(row<long>& inst_ccls);
	void	parse_file(std::string& f_nam, row<long>& inst_ccls);
	void	finish_parse(row<long>& inst_ccls);

	long	get_cursor_pos();
};

//=================================================================
// map funcs

void		shuffle_lit_mapping(tak_mak& rnd_gen, row<integer>& to_shuff);
void		init_lit_mapping(tak_mak& rnd_gen, row<integer>& the_map, long num_var);
integer		map_literal(row<integer>& the_map, integer lit);
integer		shift_literal(long in_lit, long the_shift);
void		map_cnf_lits(row<integer>& the_map, row<long>& in_ccls, row<long>& out_ccls);
void		shuffle_cnf_lits(tak_mak& rnd_gen, long num_var, row<integer>& the_map,
				row<long>& in_ccls, row<long>& out_ccls);

void		shuffle_ccl_mapping(tak_mak& rnd_gen, row<integer>& to_shuff);
void		init_ccl_mapping(tak_mak& rnd_gen, row<integer>& the_map, long num_neu);
void		rl_to_rrl(row<long>& in_lits, row_row_long_t& out_ccls);
void		rrl_to_rl(row_row_long_t& in_ccls, row<long>& out_lits);
void		shuffle_cnf_ccls(tak_mak& rnd_gen, row<integer>& the_map,
				row<long>& in_lits, row<long>& out_lits);

void		shuffle_full_cnf(tak_mak& rnd_gen, long num_var, row<integer>& the_map,
				row<long>& in_ccls, row<long>& out_ccls);

void		shift_cnf_lits(long the_shift, row<long>& in_out_lits);
void		join_cnf_lits(row<long>& in_out_lits1, long num_var1, row<long>& in_out_lits2);


#endif		// DIMACS_H


