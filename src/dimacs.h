

/*************************************************************

yoshu

dimacs.h
(C 2010) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

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

#include "config.h"
//include "sha2.h"
//include "kry.h"
#include "tak_mak.h"

enum dimacs_exception_code { 
	k_dimacs_01_exception = k_last_config_exception,
	k_last_dimacs_exception
};


#define DIMACS_DBG(prm)	DBG(prm)
//define DIMACS_CK(prm) 	DBG_CK(prm)
#define DIMACS_H_CK(prm) 	DBG_CK(prm)

#define MAX_DIMACS_HEADER_SZ 4096

typedef std::string::size_type str_pos_t;
typedef unsigned char		uchar_t;
typedef std::set<std::string> string_set_t;

extern std::string	k_dimacs_header_str;

//=================================================================
// dimacs_exception

typedef enum {
	dix_bad_eq_op,
	dix_bad_creat,
	dix_no_cnf_decl_1,
	dix_no_cnf_decl_2,
	dix_no_cnf_decl_3,
	dix_bad_num_cls,
	dix_bad_format,
	dix_zero_vars,
	dix_zero_cls,
	dix_bad_lit,
	dix_cls_too_long
} di_ex_cod_t;

class dimacs_exception : public top_exception{
public:
	char val;
	long line;
	long pt_pos;
	
	long num_decl_cls;
	long num_decl_vars;
	long num_decl_lits;
	long num_read_cls;
	long num_read_lits;
	long bad_lit;
	
	dimacs_exception(long the_id, char vv = 0, long ll = -1, long pp = -1) : 
			top_exception(the_id)
	{
		val = vv;
		line = ll;
		pt_pos = pp;

		num_decl_cls = 0;
		num_decl_lits = 0;
		num_decl_vars = 0;
		num_read_cls = 0;
		num_read_lits = 0;
		bad_lit = 0;
	}
};

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
// nid_bits

class nid_bits {
public:
	long		nb_set_count;
	bit_row		nb_pos;
	bit_row		nb_neg;

	nid_bits(long sz = 0){
		init_nid_bits(sz);
	}

	~nid_bits(){
	}

	void	clear(){
		nb_set_count = 0;
		nb_pos.clear();
		nb_neg.clear();
	}

	void	init_nid_bits(long sz = 0){
		nb_set_count = 0;
		nb_pos.fill(false, sz, false);
		nb_neg.fill(false, sz, false);
	}

	void	append_nid_bits(long n_sz){
		nb_pos.fill(false, n_sz, true);
		nb_neg.fill(false, n_sz, true);
	}

	void	inc_nid_bits(){
		DIMACS_H_CK(nb_set_count == 0);
		DIMACS_H_CK(nb_pos.size() == nb_neg.size());

		long n_sz = nb_pos.size() + 1;
		nb_pos.fill(false, n_sz, true);
		nb_neg.fill(false, n_sz, true);
	}

	long	size(){
		DIMACS_H_CK(nb_pos.size() == nb_neg.size());
		return nb_pos.size();
	}

	bool	is_true(long nid){
		DIMACS_H_CK(nb_pos.is_valid_idx(abs_long(nid)));
		DIMACS_H_CK(nb_neg.is_valid_idx(abs_long(nid)));

		bool resp = false;
		if(nid > 0){
			resp = nb_pos[nid];
		} else {
			resp = nb_neg[-nid];
		}
		return resp;
	}

	void	reset(long nid){
		DIMACS_H_CK(is_true(nid));
		DIMACS_H_CK(! is_true(-nid));
		if(nid > 0){
			nb_pos[nid] = false;
		} else {
			nb_neg[-nid] = false;
		}
		nb_set_count--;
	}

	void	set(long nid){
		DIMACS_H_CK(! is_true(nid));
		DIMACS_H_CK(! is_true(-nid));
		if(nid > 0){
			nb_pos[nid] = true;
		} else {
			nb_neg[-nid] = true;
		}
		nb_set_count++;
	}

	bool	any_true(long nid){
		return (is_true(nid) || is_true(-nid));
	}
};

inline
void
reset_all(nid_bits& bts, row<long>& rr_all, long first_ii = 0, long last_ii = -1){
	long the_sz = rr_all.size();
	if((last_ii < 0) || (last_ii > the_sz)){
		last_ii = the_sz;
	}
	if((first_ii < 0) || (first_ii > last_ii)){
		first_ii = 0;
	}
	for(long aa = first_ii; aa < last_ii; aa++){
		long nid = rr_all[aa];
		bts.reset(nid);
	}
}

inline
void
set_all(nid_bits& bts, row<long>& rr_all, long first_ii = 0, long last_ii = -1){
	long the_sz = rr_all.size();
	if((last_ii < 0) || (last_ii > the_sz)){
		last_ii = the_sz;
	}
	if((first_ii < 0) || (first_ii > last_ii)){
		first_ii = 0;
	}
	for(long aa = first_ii; aa < last_ii; aa++){
		long nid = rr_all[aa];
		bts.set(nid);
	}
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

class dimacs_loader {
public:
	typedef std::istream::char_type ld_char_t;
	typedef std::istream::pos_type ld_pos_t;

	std::string		ld_file_name;

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

	// ---------------------------------------

	nid_bits		ld_dots;

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

	void	init_parse();
	void	init_dimacs_loader();

	void 	read_problem_decl(const char*& pt_in, long& num_var, long& num_ccl, long& line);
	void 	skip_cnf_decl(const char*& pt_in, long line);
	
	void	load_file();
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


