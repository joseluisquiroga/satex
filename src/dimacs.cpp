

/*************************************************************

satex

dimacs.cpp  
(C 2010) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com

Functions to read and parse dimacs files.

--------------------------------------------------------------*/


#include <errno.h>
#include <limits.h>
#include <sys/stat.h>

#include <cassert>
#include <cstring>

#include "solver.h"

#define DIMACS_CK_0(prm) 	SUPPORT_CK_0(prm)
#define DIMACS_CK(prm) 	SUPPORT_CK(prm)

// '\0'
#define END_OF_SEC	0


std::string	k_dimacs_header_str =
		"c (C) 2010. QUIROGA BELTRAN, Jose Luis. Bogota - Colombia.\n"
		"c Date of birth: December 28 of 1970.\n"
		"c Place of birth: Bogota - Colombia - Southamerica.\n"
		"c Id (cedula): 79523732 de Bogota.\n";


t_string dimacs_err_msg(long num_line, char ch_err, const char* msg){
	return get_parse_err_msg("DIMACS ERROR. ", num_line, ch_err, msg);
}

void
dimacs_loader::skip_dimacs_whitespace(const char*& pt_in, long& line){
	while((*pt_in != 0) && isspace(*pt_in)){ 
		if(*pt_in == '\n'){ 
			line++; 
		}
		pt_in++; 
	}
}

bool
dimacs_loader::follows_int(const char*& pt_in){
	char cc = *pt_in;
	if(cc == 0){ return false; }
	bool dd = isdigit(cc);
	if(dd){ return true; }
	if((cc != '+') && (cc != '-')){	return false; }
	
	char cc2 = *(pt_in + 1);
	if(cc2 == 0){ return false; }
	bool dd2 = isdigit(cc2);
	return dd2;
}

void
dimacs_loader::load_file(t_string& f_nam){
	ld_file_name = f_nam;
	// loads the full file into ld_content
	read_file(ld_file_name, ld_content);
	
	ld_file_sha = sha_txt_of_arr((uchar_t*)ld_content.get_data(), ld_content.get_data_sz());
	
	ld_content.push(END_OF_SEC); // it already has room for it

	DBG_PRT(9, os << " ld_content="; ld_content.print_row_data(os, true, ""));
}

	/*
	uchar_t* arr_to_sha = (uchar_t*)ld_content.get_c_array();
	long arr_to_sha_sz = ld_content.get_c_array_sz() - 1;
	ld_sha_str = sha_txt_of_arr(arr_to_sha, arr_to_sha_sz);

	DBG_PRT(11, os << "SHA_SZ 1=" << arr_to_sha_sz);
	*/

bool
dimacs_loader::fixappend_lits(row<long>& lits, row<long>& prepared){
	if(lits.size() > ld_max_in_ccl_sz){
		ld_max_in_ccl_sz = lits.size();
	}

	if(ld_as_3cnf){
		three_lits(lits, prepared);
	}

	add_lits(lits, prepared, true);

	return true;
}

void
dimacs_loader::add_lits(row<long>& lits, row<long>& prepared, bool is_orig){
	if(! is_orig){
		ld_nud_added_ccls++;
		ld_nud_added_lits += lits.size();
	}

	lits.append_to(prepared);
	prepared.push(0);

	lits.clear();
}

void
dimacs_loader::append_rr_lits(row_row_long_t& rr_lits, row<long>& prepared){
	while(! rr_lits.is_empty()){
		row<long>& ff = rr_lits.last();

		add_lits(ff, prepared, false);

		rr_lits.dec_sz();
	}
	DIMACS_CK(rr_lits.is_empty());
}

void
dimacs_loader::three_lits(row<long>& lits, row<long>& prepared){
	DIMACS_CK(ld_decl_vars > 0);

	DBG_PRT(8, os << "THREE_LITS " << lits);

	long max_ccl_sz = 3;
	long def_sz = 2;
	MARK_USED(def_sz);

	while(lits.size() > max_ccl_sz){
		long lit_1 = lits.pop();
		long lit_0 = lits.pop();

		DIMACS_CK(ld_lits2.is_empty());
		ld_lits2.push(lit_0);
		ld_lits2.push(lit_1);

		ld_nud_added_vars++;
		long curr_max_var = ld_decl_vars + ld_nud_added_vars;
		DIMACS_CK(curr_max_var > 0);

		lits.push(curr_max_var);

		DIMACS_CK(ld_rr_lits1.is_empty());
		DIMACS_CK(ld_lits2.size() == def_sz);

		calc_f_lit_equal_or(curr_max_var, ld_lits2, ld_rr_lits1);
		DIMACS_CK(ld_lits2.is_empty());
		//ld_lits2.clear();

		append_rr_lits(ld_rr_lits1, prepared);
	}
}

void
dimacs_loader::calc_f_lit_equal_or(long d_lit, row<long>& or_lits,
			row_row_long_t& rr_lits)
{
	DBG_PRT(11, os << "EQ_OR. d_lit=" << d_lit << " or_lits=" << or_lits);

	DIMACS_CK(d_lit > 0);
	DIMACS_CK(! or_lits.is_empty());

	row<long>& f1 = rr_lits.inc_sz();

	or_lits.copy_to(f1);
	f1.push(-d_lit);

	DBG_PRT(41, os << "EQ_OR. ccl=" << f1);

	long ii = 0;
	for(ii = 0; ii < or_lits.size(); ii++){
		row<long>& f2 = rr_lits.inc_sz();
		long o_lit = or_lits[ii];

		f2.push(-o_lit);
		f2.push(d_lit);

		DBG_PRT(41, os << "EQ_OR. ccl(" << ii << ")=" << f2);
	}

	or_lits.clear();
}

void
dimacs_loader::init_parse(){
	ld_max_in_ccl_sz = 0;

	ld_num_line = 0;
	ld_cursor = NULL_PT;

	ld_decl_ccls = INVALID_NATURAL;
	ld_decl_vars = INVALID_NATURAL;
	ld_decl_lits = INVALID_NATURAL;

	ld_parsed_ccls = 0;
	ld_parsed_lits = 0;
	ld_parsed_vars = 0;

	//ld_dots.clear();

	ld_fixes.fill(0, k_last_fix);

	ld_lits1.clear();
	ld_lits2.clear();

	ld_rr_lits1.clear();
	ld_rr_lits2.clear();

	ld_last_or_lit = 0;

	ld_nud_added_ccls = 0;
	ld_nud_added_vars = 0;
	ld_nud_added_lits = 0;

	ld_num_ccls = INVALID_NATURAL;
	ld_num_vars = INVALID_NATURAL;
	ld_tot_lits = INVALID_NATURAL;
	
	ld_all_vars = long_set_t();

	ld_num_line = 1;
	if(! ld_content.is_empty()){
		DIMACS_CK(ld_content.last() == END_OF_SEC);
		ld_cursor = ld_content.get_c_array();
	}
}

void
dimacs_loader::init_dimacs_loader(){
	
	ld_dbg_info = NULL;

	ld_file_name = "";

	ld_as_3cnf = false;

	init_parse();
}

bool
dimacs_loader::parse_clause(row<integer>& lits){
	lits.clear();

	const char*& pt_in = ld_cursor;

	if(pt_in == NULL_PT){
		return false;
	}

	if(*pt_in == END_OF_SEC){
		return false;
	}

	skip_dimacs_whitespace(pt_in, ld_num_line);

	DIMACS_DBG(row<integer> not_mapped);

	integer	parsed_lit;
	while(*pt_in != END_OF_SEC){
		skip_dimacs_whitespace(pt_in, ld_num_line);
		bool isint = follows_int(pt_in);
		if(! isint){
			skip_line(pt_in, ld_num_line);
			continue;
		}
		parsed_lit = parse_int(pt_in, ld_num_line);
		if(parsed_lit == 0){ break; }
		
		ld_all_vars.insert(get_var(parsed_lit));
		
		lits.push(parsed_lit);
	}

	if(lits.is_empty()){
		return false;
	}

	ld_parsed_ccls++;
	ld_parsed_lits += lits.size();
	return true;
}

void
dimacs_loader::parse_all_ccls(row<long>& inst_ccls)
{
	init_parse();
	//parse_header();

	DBG_PRT(11, os << "ld_cursor=" << ld_cursor);

	DIMACS_CK(ld_nud_added_ccls == 0);
	DIMACS_CK(ld_nud_added_vars == 0);
	DIMACS_CK(ld_nud_added_lits == 0);

	//ld_dots.init_nid_bits(ld_decl_vars + 1);

	inst_ccls.clear();

	DIMACS_CK(ld_lits1.is_empty());
	while(parse_clause(ld_lits1)){
		bool go_on = fixappend_lits(ld_lits1, inst_ccls);
		MARK_USED(go_on);
		DIMACS_CK(go_on);
		DIMACS_CK(ld_lits1.is_empty());
	}
}

void
dimacs_loader::parse_file(std::string& f_nam, row<long>& inst_ccls)
{
	//ld_file_name = f_nam;

	load_file(f_nam);

	parse_all_ccls(inst_ccls);

	finish_parse(inst_ccls);
	
	DBG_PRT(45, os << inst_ccls);
}

// shuffle lits

void
shuffle_lit_mapping(tak_mak& rnd_gen, row<integer>& to_shuff){
	row<integer> tmp_bag;

	long sz_0 = to_shuff.size();
	MARK_USED(sz_0);

	row_index from = FIRST_LIT_IDX;
	to_shuff.copy_to(tmp_bag, from);
	to_shuff.clear(true, false, from);

	while(tmp_bag.size() > 0){
		long idx_pop = rnd_gen.gen_rand_int32_ie(0, tmp_bag.size());

		DIMACS_CK_0(idx_pop >= 0);
		DIMACS_CK_0(idx_pop < tmp_bag.size());
		integer var1 = tmp_bag.swap_pop(idx_pop);
		DIMACS_CK_0(var1 > 0);

		long pol = rnd_gen.gen_rand_int32_ie(0, 2);
		if(pol == 0){ var1 = -var1; }

		to_shuff.push(var1);
	}

	DIMACS_CK_0(to_shuff.size() == sz_0);
}

void
init_lit_mapping(tak_mak& rnd_gen, row<integer>& the_map, long num_var){
	//long rnd_base = gen_random_num(0, 99999);
	//tak_mak rnd_gen(rnd_base);

	the_map.set_cap(num_var + 1);
	the_map.clear();
	for(long kk = 0; kk < num_var + 1; kk++){
		the_map.push(kk);
	}

	shuffle_lit_mapping(rnd_gen, the_map);
}

integer
map_literal(row<integer>& the_map, integer lit){
	integer var = get_var(lit);
	DIMACS_H_CK(var > 0);
	DIMACS_H_CK(var < the_map.size());
	integer map_var = the_map[var];
	if(lit < 0){
		return -map_var;
	}
	return map_var;
}

void
map_cnf_lits(row<integer>& the_map, row<long>& in_lits, row<long>& out_lits)
{
	out_lits.clear();
	for(long ii = 0; ii < in_lits.size(); ii++){
		long in_lit = in_lits[ii];
		long out_lit = 0;
		if(in_lit != 0){
			out_lit = map_literal(the_map, in_lit);
		}
		out_lits.push(out_lit);
	}
}

void
shuffle_cnf_lits(tak_mak& rnd_gen, long num_var, row<integer>& the_map,
		row<long>& in_lits, row<long>& out_lits)
{
	init_lit_mapping(rnd_gen, the_map, num_var);

	map_cnf_lits(the_map, in_lits, out_lits);
}

// join ccls

long
shift_literal(long in_lit, long the_shift){
	integer var = get_var(in_lit);
	DIMACS_CK_0(var > 0);
	DIMACS_CK_0(the_shift > 0);
	var += the_shift;
	if(in_lit < 0){
		return -var;
	}
	return var;
}

void
shift_cnf_lits(long the_shift, row<long>& in_out_lits)
{
	for(long ii = 0; ii < in_out_lits.size(); ii++){
		long in_lit = in_out_lits[ii];
		long out_lit = 0;
		if(in_lit != 0){
			out_lit = shift_literal(in_lit, the_shift);
		}
		in_out_lits[ii] = out_lit;
	}

}

void
join_cnf_lits(row<long>& in_out_lits1, long num_var1, row<long>& in_out_lits2)
{
	shift_cnf_lits(num_var1, in_out_lits2);
	in_out_lits2.append_to(in_out_lits1);
}

// shuffle ccls

void
shuffle_ccl_mapping(tak_mak& rnd_gen, row<integer>& to_shuff){
	row<integer> tmp_bag;

	long sz_0 = to_shuff.size();
	MARK_USED(sz_0);

	to_shuff.copy_to(tmp_bag);
	to_shuff.clear();

	while(tmp_bag.size() > 0){
		long idx_pop = rnd_gen.gen_rand_int32_ie(0, tmp_bag.size());

		DIMACS_CK_0(idx_pop >= 0);
		DIMACS_CK_0(idx_pop < tmp_bag.size());
		integer idx1 = tmp_bag.swap_pop(idx_pop);
		DIMACS_CK_0(idx1 >= 0);

		to_shuff.push(idx1);
	}

	DIMACS_CK_0(to_shuff.size() == sz_0);
}

void
init_ccl_mapping(tak_mak& rnd_gen, row<integer>& the_map, long num_ccl){
	the_map.set_cap(num_ccl);
	the_map.clear();
	for(long kk = 0; kk < num_ccl; kk++){
		the_map.push(kk);
	}

	shuffle_ccl_mapping(rnd_gen, the_map);
}

void
rl_to_rrl(row<long>& in_lits, row_row_long_t& out_ccls)
{
	row_long_t tmp_rr;
	out_ccls.clear();
	tmp_rr.clear();
	for(long ii = 0; ii < in_lits.size(); ii++){
		long in_lit = in_lits[ii];
		if(in_lit != 0){
			tmp_rr.push(in_lit);
		} else {
			row_long_t& rr = out_ccls.inc_sz();
			tmp_rr.move_to(rr);
			DIMACS_CK_0(tmp_rr.is_empty());
		}
	}
}

void
rrl_to_rl(row_row_long_t& in_ccls, row<long>& out_lits)
{
	out_lits.clear();
	for(long ii = 0; ii < in_ccls.size(); ii++){
		row_long_t& rr = in_ccls[ii];
		for(long kk = 0; kk < rr.size(); kk++){
			long in_lit = rr[kk];
			out_lits.push(in_lit);
		}
		out_lits.push(0);
	}
}

void
shuffle_cnf_ccls(tak_mak& rnd_gen, row<integer>& the_map,
		row<long>& in_lits, row<long>& out_lits)
{
	row_row_long_t in_ccls;
	rl_to_rrl(in_lits, in_ccls);

	long num_ccls = in_ccls.size();
	init_ccl_mapping(rnd_gen, the_map, num_ccls);

	DIMACS_CK_0(the_map.size() == num_ccls);

	row_row_long_t out_ccls;
	out_ccls.fill_new(num_ccls);

	for(long ii = 0; ii < in_ccls.size(); ii++){
		row_long_t& rr1 = in_ccls[ii];
		long n_idx = the_map[ii];

		DIMACS_CK_0(out_ccls.is_valid_idx(n_idx));
		row_long_t& rr2 = out_ccls[n_idx];

		DIMACS_CK_0(! rr1.is_empty());
		DIMACS_CK_0(rr2.is_empty());

		rr1.move_to(rr2);
	}

	rrl_to_rl(out_ccls, out_lits);
}

void		shuffle_full_cnf(tak_mak& rnd_gen, long num_var, row<integer>& the_map,
				row<long>& in_ccls, row<long>& out_ccls)
{
	row<long> tmp_ccls;
	shuffle_cnf_lits(rnd_gen, num_var, the_map, in_ccls, tmp_ccls);
	shuffle_cnf_ccls(rnd_gen, the_map, tmp_ccls, out_ccls);
}

void
dimacs_loader::lits_opps(row<long>& r_lits){
	for(long jj = 0; jj < r_lits.size(); jj++){
		long lit = r_lits[jj];
		r_lits[jj] = -lit;
	}
}

void
dimacs_loader::calc_f_lit_equal_and(long d_lit, row<long>& and_lits,
			row_row_long_t& rr_lits)
{
	DBG_PRT(12, os << "EQ_AND. d_lit=" << d_lit << " and_lits=" << and_lits);

	SUPPORT_CK(! and_lits.is_empty());
	row<long>& f1 = rr_lits.inc_sz();

	and_lits.copy_to(f1);
	lits_opps(f1);
	f1.push(d_lit);

	DBG_PRT(42, os << "EQ_AND. ccl=" << f1);

	long ii = 0;
	for(ii = 0; ii < and_lits.size(); ii++){
		row<long>& f2 = rr_lits.inc_sz();
		long a_lit = and_lits[ii];

		f2.push(a_lit);
		f2.push(-d_lit);

		DBG_PRT(42, os << "EQ_AND. ccl(" << ii << ")=" << f2);
	}

	and_lits.clear();
}

void
print_dimacs_of(std::ostream& os, row<long>& all_lits, long num_cla, long num_var){

	os << k_dimacs_header_str << std::endl;
	os << "p cnf " << num_var << " " << num_cla << " " << std::endl;

	long first = 0;
	long neus_cou = 0;
	long dens_cou = 0;

	for(long ii = 0; ii < all_lits.size(); ii++){
		long nio_id = all_lits[ii];
		os << nio_id << " ";
		if(nio_id == 0){
			os << std::endl;

			neus_cou++;
			long num_dens = ii - first;
			DIMACS_CK_0(num_dens > 0);
			dens_cou += num_dens;

			first = ii + 1;
		}
	}

	os << std::endl;
	os.flush();
}

void
dimacs_loader::finish_parse(row<long>& inst_ccls)
{
	ld_parsed_vars = ld_all_vars.size();
	
	SUPPORT_CK(ld_as_3cnf || (ld_nud_added_ccls == 0));
	SUPPORT_CK(ld_as_3cnf || (ld_nud_added_vars == 0));
	SUPPORT_CK(ld_as_3cnf || (ld_nud_added_lits == 0));

	ld_num_ccls = ld_parsed_ccls + ld_nud_added_ccls;
	ld_num_vars = ld_parsed_vars + ld_nud_added_vars;
	ld_tot_lits = ld_parsed_lits + ld_nud_added_lits;
	
	DBG_PRT(45, os << "ccls=" << ld_num_ccls);
	DBG_PRT(45, os << "vars=" << ld_num_vars);
	DBG_PRT(45, os << "lits=" << ld_tot_lits);	
}

long
dimacs_loader::get_cursor_pos(){
	const char* pt_base = ld_content.get_c_array();
	const char* pt_in = ld_cursor;
	return (pt_in - pt_base);
}



