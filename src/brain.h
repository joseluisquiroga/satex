

/*************************************************************

bible_sat

brain.h
(C 2025) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com

Declarations of classes and that implement the neural network.

--------------------------------------------------------------*/

#ifndef BRAIN_H
#define BRAIN_H


//=================================================================================================
// defines

#define BRAIN_DBG(prm) 		DBG(prm)
#define DBG_SOURCES(cod)	DBG(cod)

//=================================================================================================
// MAIN CLASSES

#define ROOT_LEVEL 0
#define MIN_TRAIL_SZ 1

enum action_t {
	ac_invalid = -100,
	ac_recoil = -1,
	ac_stop = 0,
	ac_go_on = 1
};

enum charge_t {
	cg_negative = -1,
	cg_neutral = 0,
	cg_positive = 1
};

#define k_invalid_order		0
#define k_left_order		1
#define k_right_order		2
#define k_random_order		3

#define INVALID_RECOIL 0
#define INVALID_LEVEL -1

#define MAX_RECOIL LONG_MAX

//=================================================================================================
// extern vars

#define DBG_DIR "./dbg_ic_output/"
#define DBG_PREF "brain"
#define DBG_SUFI ".myl"


//=================================================================================================
// decl

class ticket;
class quanton;
class neuron;
class reason;
class brain;

//=================================================================
// printing declarations

#define DECLARE_PRINT_FUNCS(obj_t) \
std::ostream& operator << (std::ostream& os, obj_t& obj1); \
std::ostream& operator << (std::ostream& os, obj_t* obj1); \
\

// end_of_define

#define DEFINE_PRINT_FUNCS(obj_t) \
inline \
std::ostream& operator << (std::ostream& os, obj_t& obj1){ \
	obj1.print_##obj_t(os); \
	os.flush(); \
	return os; \
} \
\
inline \
std::ostream& operator << (std::ostream& os, obj_t* obj1){ \
	if(obj1 == NULL_PT){ \
		os << "NULL_" << #obj_t; \
	} else { \
		obj1->print_##obj_t(os, true); \
	} \
	os.flush(); \
	return os; \
} \
\

// end_of_define


DECLARE_PRINT_FUNCS(ticket)
DECLARE_PRINT_FUNCS(quanton)
DECLARE_PRINT_FUNCS(neuron)
DECLARE_PRINT_FUNCS(reason)

//=================================================================================================
// includes

#include <limits.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "tools.h"
#include "support.h"

//=================================================================
// comparison declarations

class ticket;
class quanton;
class neuron;
class brain;

//=================================================================
// comparison declarations

bool 		cmp_ticket_eq(ticket& x, ticket& y);
bool 		cmp_ticket_lt(ticket& x, ticket& y);
comparison	cmp_long_id(quanton* const & qua1, quanton* const & qua2);
comparison	cmp_choice_idx_lt(quanton* const & qua1, quanton* const & qua2);
comparison	cmp_choice_idx_gt(quanton* const & qua1, quanton* const & qua2);
comparison	cmp_qlevel(quanton* const & qua1, quanton* const & qua2);

//=================================================================
// funcs declarations

charge_t 	negate_trinary(charge_t val);

void	set_dots_of(row<quanton*>& quans);
void	reset_dots_of(row<quanton*>& quans);
//void	negate_quantons(row<quanton*>& qua_row);
bool	compute_reasons(row<reason>& rsns);
void	append_reasons(row<reason>& dest, row<reason>& src);
void	get_ids_of(row<quanton*>& quans, row_long_t& the_ids);


//=================================================================================================
// ticket

class ticket {
	public:
	long		tk_recoil;	// recoil at update_tk time
	long		tk_level;	// level at update_tk time
	//long		tk_trail_sz;	// trail_sz at update_tk time

	// methods

	ticket(){
		init_ticket();
	}

	void	init_ticket(){
		tk_recoil = INVALID_RECOIL;
		tk_level = INVALID_LEVEL;
		//tk_trail_sz = INVALID_IDX;
	}

	debug_info*	get_dbg_info(){
		return NULL;
	}
	
	/*
	bool	is_valid(){
		return ((tk_recoil != INVALID_RECOIL) && 
				(tk_level != INVALID_LEVEL) &&
				(tk_trail_sz != INVALID_IDX) 
				);
	}*/

	bool	is_valid(){
		return ((tk_recoil != INVALID_RECOIL) && (tk_level != INVALID_LEVEL));
	}

	//bool	is_active(brain* brn);
	void	update_ticket(brain* brn);

	/*
	long		trail_idx(){
		return (tk_trail_sz - 1);
	}*/

	std::ostream&	print_ticket(std::ostream& os, bool from_pt = false){
		os << "rc:" << tk_recoil;
		os << " lv:" << tk_level;
		//os << " tz:" << tk_trail_sz << " ";
		os.flush();
		return os;


	}
};

inline
bool
cmp_ticket_eq(ticket& x, ticket& y){
	if(x.tk_recoil != y.tk_recoil){ return false; }
	if(x.tk_level != y.tk_level){ return false; }
	return true;
}

/*
inline
bool
cmp_ticket_lt(ticket& x, ticket& y){
	if(x.tk_recoil > y.tk_recoil){ return false; }
	if(x.tk_recoil < y.tk_recoil){ return true; }
	bool resp = (x.tk_trail_sz < y.tk_trail_sz);
	BRAIN_CK_0(!resp || (x.tk_level <= y.tk_level));
	return resp;
}
*/

//=================================================================================================
// quanton


class quanton {
	public:
		
	BRAIN_DBG(debug_info*		qu_dbg_info;)

	static 
	quanton			ROOT_QUANTON;

	long			qu_dbg_ic_trail_idx;	// idx in trail
	
	// id attributes
	long			qu_id;		// my long number id
	long			qu_index;	// idx in brain's 'positons' or 'negatons'
	charge_t		qu_spin;	// positive if positon or negative if negaton
	quanton*		qu_inverse;	// my inverse quanton

	// state attributes
	// symetric means that it must be set for positons and negatons at the same time

	charge_t		qu_dot;		// symetric. to mark quantons during learning and neuron creation

	charge_t		qu_charge;	// symetric. current charge
	ticket			qu_charge_tk;	// symetric. 

	neuron*			qu_source;	// source of signal when charged

	// tunneling attributes
	row<neuron*>		qu_tunnels;	// tunnelled neurons.

	// choice attributes
	long			qu_choice_idx;	// idx in brain's 'choices'

	// auxiliar attributes for analysis
	
	DBG_SOURCES(row<neuron*>	qu_dbg_ic_all_sources;)	// for printing the ic

	// methods

	quanton(){
		init_quanton(NULL, cg_neutral, INVALID_IDX, NULL);
	}

	~quanton(){

		BRAIN_CK_0(qu_source == NULL);

		BRAIN_CK_0(qu_tunnels.size() == 0);

		qu_tunnels.clear(false, true);

		DBG_SOURCES(qu_dbg_ic_all_sources.clear(false, true);)

		init_quanton(NULL, cg_neutral, 0, NULL);
	}

	void	qua_tunnel_signals(brain* brn, row<neuron*>& cnflicts);

	debug_info*	get_dbg_info(){
		debug_info* dbg_info = NULL;
		BRAIN_DBG(dbg_info = qu_dbg_info);
		return dbg_info;
	}

	void	init_quanton(debug_info* dbg_inf, charge_t spn, long ii, quanton* inv){
		BRAIN_DBG(qu_dbg_info = dbg_inf);

		qu_dbg_ic_trail_idx = INVALID_IDX;

		qu_id = (spn == cg_positive)?(ii + 1):(-(ii + 1));
		qu_index = ii;
		qu_spin = spn;
		qu_inverse = inv;

		qu_choice_idx = INVALID_IDX;

		qu_dot = cg_neutral;

		qu_charge = cg_neutral;
		qu_charge_tk.init_ticket();
		qu_source = NULL;
	}

	quanton*	get_positon(){
		if(qu_spin == cg_negative){
			BRAIN_CK_0(qu_inverse->qu_spin == cg_positive);
			return qu_inverse;
		}
		BRAIN_CK_0(qu_spin == cg_positive);
		return this;
	}

	long		abs_id(){
		long resp = get_positon()->qu_id;
		BRAIN_CK_0(resp > 0);
		return resp;
	}

	bool	ck_all_tunnels();
	void	tunnel_swapop(long idx_pop);

	bool	has_dot(){	
		return (qu_dot != cg_neutral);	
	}

	void	set_dot(){
		qu_dot = cg_positive;
		qu_inverse->qu_dot = cg_negative;
	}

	void	reset_dot(){


		qu_dot = cg_neutral;
		qu_inverse->qu_dot = cg_neutral;
	}

	long		qrecoil(){ return qu_charge_tk.tk_recoil; }
	long		qlevel(){ return qu_charge_tk.tk_level; }

	bool		ck_charge(brain* brn);

	void		set_charge(brain* brn, neuron* src, charge_t cha);
	charge_t	get_charge(){ return qu_charge; }

	void		set_source(brain* brn, neuron* neu);
	neuron*		get_source(){
		if(qu_source == NULL){
			return qu_inverse->qu_source;
		}
		return qu_source;
	}


	std::ostream&		print_quanton(std::ostream& os, bool from_pt = false);

	std::ostream& 		print_ic_label(std::ostream& os){
		os << "\"";
		if(qu_charge == cg_negative){ os << "K"; }
		if(qu_spin == cg_negative){ os << "["; }
		os << abs_id();
		if(qu_spin == cg_negative){ os << "]"; }
		os << "<";
		os << qlevel();
		os << "_" << qu_dbg_ic_trail_idx;
		DBG_SOURCES(os << "_" << qu_dbg_ic_all_sources.size();)
		os << ">";
		os << "\"";
		return os;
	}
};

//=================================================================================================
// neuron

class neuron {
	public:

	BRAIN_DBG(debug_info*		ne_dbg_info;)

	static
	row<quanton*>		ne_dbg_fibres;		// tmp row for quas (usually) sorted by id

	long			ne_index;
	bool			ne_original;

	long			ne_fibres_sz;
	quanton**		ne_fibres;		// used in forward propagation of negative signls
	long			ne_fibre_0_idx;		// this == fibres[0]->tunnels[fibre_0_idx]
	long			ne_fibre_1_idx;		// this == fibres[1]->tunnels[fibre_1_idx]

	long			ne_edge;		// last (with max_level) of negated tunneled quantons 
	ticket			ne_edge_tk;

	bool			ne_is_conflict;		// to push conflcts only once into conflct queue

	long			ne_dbg_num_fires;

	// methods

	neuron(){
		init_neuron();
	}

	~neuron(){
		//BRAIN_CK(ck_no_source_of_any());

		row<quanton*> empty;
		BRAIN_CK_0(empty.size() == 0);
		quanton* forced_qua = update_fibres(empty, false);
		MARK_USED(forced_qua);
		BRAIN_CK_0(forced_qua == NULL);

		BRAIN_CK_0(ne_fibres == NULL);
		BRAIN_CK_0(size() == 0);

		init_neuron();
	}

	void	init_neuron(){
		BRAIN_DBG(ne_dbg_info = NULL);
		
		ne_original = false;

		ne_fibres = NULL;
		ne_fibres_sz = 0;
		ne_fibre_0_idx = INVALID_IDX;
		ne_fibre_1_idx = INVALID_IDX;

		ne_edge = INVALID_IDX;
		ne_edge_tk.init_ticket();

		ne_is_conflict = false;

		ne_dbg_num_fires = 0;
	}

	debug_info*	get_dbg_info(){
		debug_info* dbg_info = NULL;
		BRAIN_DBG(dbg_info = ne_dbg_info);
		return dbg_info;
	}

	long	size(){ return ne_fibres_sz; }

	bool	ck_tunnels(){
		BRAIN_CK_0(	(ne_fibre_0_idx == INVALID_IDX) || 
				((ne_fibres[0]->qu_tunnels)[ne_fibre_0_idx] == this));
		BRAIN_CK_0(	(ne_fibre_1_idx == INVALID_IDX) ||	
				((ne_fibres[1]->qu_tunnels)[ne_fibre_1_idx] == this));
		return true;
	}

	quanton* forced_quanton(){
		return ne_fibres[0];
	}

	void	swap_fibres_0_1(){
		BRAIN_CK(ck_tunnels());
		quanton* fb_0 = ne_fibres[0];
		ne_fibres[0] = ne_fibres[1];
		ne_fibres[1] = fb_0;

		long fb_0_idx = ne_fibre_0_idx;
		ne_fibre_0_idx = ne_fibre_1_idx;
		ne_fibre_1_idx = fb_0_idx;
		BRAIN_CK(ck_tunnels());
	}

	void	swap_fibres_1(long idx){
		BRAIN_CK(ck_tunnels());
		quanton* fb_1 = ne_fibres[1];
		long	idx_1 = ne_fibre_1_idx;

		ne_fibres[1] = ne_fibres[idx];
		ne_fibres[idx] = fb_1;

		ne_fibres[1]->qu_tunnels.push(this);
		ne_fibre_1_idx = ne_fibres[1]->qu_tunnels.size() - 1;

		fb_1->tunnel_swapop(idx_1);
		BRAIN_CK(ck_tunnels());
	}

	action_t	neu_tunnel_signals(brain* brn, quanton* qua);

	quanton*	update_fibres(row<quanton*>& synps, bool orig);
	quanton*	get_prt_fibres(row<quanton*>& tmp_fibres, bool sort_them = true);	// sorted by id 

	bool		ck_all_neg(brain* brn, long from);

	bool		ck_no_source_of_any(){
		for(long ii = 0; ii < ne_fibres_sz; ii++){
			quanton* qua = ne_fibres[ii];
			MARK_USED(qua);
			BRAIN_CK_0(qua->get_source() != this);
			DBG_SOURCES(
				row<neuron*>& dbg_all_src = qua->qu_dbg_ic_all_sources;
				for(long ii = 0; ii < dbg_all_src.size(); ii++){
					BRAIN_CK_0(dbg_all_src[ii] != this);
				}
			)
		}
		return true;
	}

	bool	save_clause(row_long_t& save_ccls, long& num_lits);

	bool	neu_compute_binary(){
		for(long ii = 0; ii < size(); ii++){
			charge_t chg = ne_fibres[ii]->get_charge();
			if(chg == cg_positive){
				return true;
			}
		}
		return false;
	}

	//for IS_SAT_CK
	bool	neu_compute_dots(){
		for(long ii = 0; ii < size(); ii++){
			charge_t chg = ne_fibres[ii]->qu_dot;
			if(chg == cg_positive){
				return true;
			}
		}
		return false;
	}

	std::ostream& 	print_neu_base(std::ostream& os, bool detail, bool prt_src, bool sort_fib);
	std::ostream&	print_neuron(std::ostream& os, bool from_pt = false);
};

//=================================================================================================
// reason

class reason {
	public:
	row<quanton*>		ra_motives;
	long			ra_target_level;

	neuron*			ra_confl;	// only info attrib
	neuron*			ra_neuron;	// only info attrib
	quanton*		ra_forced;	// only info attrib

	reason(){
		init_reason();
	}

	~reason(){
		init_reason();
		ra_motives.clear(false, true);
	}

	void	init_reason(long m_sz = 0, neuron* cfl = NULL, long target_lev = INVALID_LEVEL){
		if(m_sz > 0){


			ra_motives.set_cap(m_sz);
			ra_motives.clear();
		}

		ra_target_level = target_lev;
		ra_confl = cfl;
		ra_neuron = NULL;
		ra_forced = NULL_PT;
	}

	debug_info*	get_dbg_info(){
		debug_info* dbg_info = NULL;
		BRAIN_DBG(if(! ra_motives.is_empty()){ dbg_info = ra_motives[0]->get_dbg_info(); });
		return dbg_info;
	}
	
	void	fill_reason(row<quanton*>& tmp_mots, 
			neuron* cfl = NULL, long target_lev = INVALID_LEVEL)
	{
		init_reason(tmp_mots.size(), cfl, target_lev);

		for(long ii = 0; ii < tmp_mots.size(); ii++){
			quanton* mot = tmp_mots[ii];
			BRAIN_CK(mot != NULL);
			mot->reset_dot();
			ra_motives.push(mot);
		}
	}

	long		size(){
		return ra_motives.size();
	}

	bool	rsn_compute_dots(){
		for(long ii = 0; ii < ra_motives.size(); ii++){
			trinary chg = ra_motives[ii]->qu_dot;
			if(chg == cg_positive){
				return true;
			}
		}
		return false;
	}

	std::ostream&	print_reason(std::ostream& os, bool from_pt = false){
		MARK_USED(from_pt);
		os << "{";
		os << ra_motives;
		os << " lv:" << ra_target_level;
		if(ra_neuron !=  NULL){
			os << " neu:";
			os << ra_neuron;
		}
		if(ra_forced !=  NULL){
			os << " qua:";
			os << ra_forced;
		}
		if(ra_confl !=  NULL){
			os << " cfl:";
			os << ra_confl;
		}
		os << "}";
		os.flush();
		return os;
	}

	void	rsn_copy_to(reason& rsn){
		rsn.init_reason(size(), ra_confl, ra_target_level);
		rsn.ra_neuron = ra_neuron;
		ra_motives.copy_to(rsn.ra_motives);
	}
};

//=================================================================================================
// brain

void		due_periodic_prt(void* pm, double curr_secs);

class brain {
	public:

	debug_info*		br_dbg_info;
	instance_info*	br_pt_inst;
	timer			br_prt_timer;

	double 			br_start_load_tm;

	// debug attributes

	// temporal attributes
	row<quanton*>		br_tmp_fixing_quantons;
	row<quanton*>		br_tmp_motives;
	row<quanton*>		br_tmp_edge;

	row<reason>		br_reasons;
	long			br_excited_level;

	// config attributes
	std::string		br_file_name;
	std::string		br_file_name_in_ic;

	ticket			br_current_ticket;

	//row<quanton*>		satisfying;		// charges after finishing
	k_row<quanton>		br_positons;	// all quantons with positive charge
	k_row<quanton>		br_negatons;	// all quantons with negative charge
	row<quanton*>		br_trail;		// in time of charging order

	charge_t		br_choice_spin;
	long			br_choice_order;

	long			br_choices_lim;	// last known choice idx
	row<quanton*>		br_choices;	// to find non charged quantons quickly
	row<quanton*>		br_chosen;	// the in 'root' level + chosen ones

	k_row<neuron>		br_neurons;	// all neurons
	queue<quanton*>		br_signals;	// forward propagated signals

	// final message

	std::ostringstream	br_final_msg;

	// methods

	brain(){
		DBG_CK(GLB.pt_brain == NULL_PT);
		GLB.pt_brain = this;

		init_brain();
	}

	debug_info*	get_dbg_info(){
		return br_dbg_info;
	}	

	void	init_brain(){
		br_dbg_info = NULL;
		
		br_pt_inst = NULL_PT;
		br_prt_timer.init_timer(PRINT_PERIOD, SOLVING_TIMEOUT);

		br_start_load_tm = 0.0;

		// debug attributes

		// temp attributes
		br_excited_level = 0;

		// config attributes
		br_file_name = "<file_name>";
		br_file_name_in_ic = "<file_name_in_ic>";

		// state attributes
		br_choice_spin = cg_neutral;
		br_choice_order = k_invalid_order;
		br_choices_lim = INVALID_IDX;
	}

	~brain(){
		DBG_CK(GLB.pt_brain == this);
		GLB.pt_brain = NULL_PT;

		clear_brain();
	}

	void	clear_brain(){
		if(level() != ROOT_LEVEL){
			recoil_to_level(ROOT_LEVEL, br_reasons);
		}

		// reset neurons
		br_neurons.clear(true);
		neuron::ne_dbg_fibres.clear(false, true);

		// temporal attributes
		br_tmp_fixing_quantons.clear(false, true);
		br_tmp_motives.clear(false, true);
		br_tmp_edge.clear(false, true);

		br_reasons.clear(true, true);

		// state attributes
		//satisfying.clear(true, true);
		br_positons.clear(true, true);	
		br_negatons.clear(true, true);	
		br_trail.clear(false, true);
		br_choices.clear(false, true);
		br_chosen.clear(false, true);

		br_neurons.clear(false, true);	// already reseted
		br_signals.clear(false, true);
	}

	// core methods

	void		brn_tunnel_signals(row<neuron*>& tunn_cflcs);
	action_t	send_signal(quanton* qua, neuron* src);
	quanton*	choose_quanton();

	void		find_middle_reason_of(neuron* confl, reason& rsn, row<quanton*>& tmp_mots);
	void		init_excited_level();
	void		update_excited_level(reason& rsn);
	void		find_reasons(row<neuron*>& confls, row<reason>&	resns);
	quanton*	add_reason(reason& rsn);
	void		learn_reasons(row<reason>& resns);
	void		pulsate(row<neuron*>& cnflcts);
	void		build_greater_than_predicate(long lim_idx, row<reason>& ord_resns, row<quanton*>& ceros_tmp);

	// aux methods

	void	set_file_name_in_ic(std::string f_nam = "");
	void	config_brain(std::string f_nam = "");
	void	init_loading(long num_qua, long num_neu);
	bool	init_sat();

	void	set_random_choices();

	neuron*	alloc_neuron(bool orig);
	neuron*	add_neuron(row<quanton*>& quans, quanton*& forced_qua, bool orig);

	quanton*	get_quanton(long q_id){
		quanton* qua = NULL;
		BRAIN_CK_0(q_id != 0);
		if(q_id < 0){ 
			qua = &(br_negatons[-q_id - 1]);
		} else {
			BRAIN_CK_0(q_id > 0);
			qua =  &(br_positons[q_id - 1]);
		}
		BRAIN_CK_0(qua->qu_id == q_id);
		return qua;
	}

	long		recoil(){
		return br_current_ticket.tk_recoil;
	}

	long		last_recoil(){
		return br_current_ticket.tk_recoil - 1;
	}

	long		next_recoil(){
		return br_current_ticket.tk_recoil + 1;
	}

	void	inc_recoil(){
		br_current_ticket.tk_recoil++;

		if(br_current_ticket.tk_recoil == MAX_RECOIL){
			std::cout << "FATAL ERROR. Too many 'recoils'\n";
			abort_func(-1, br_file_name.c_str());
		}
	}

	long		level(){
		return br_current_ticket.tk_level;
	}

	void	inc_level(){
		(br_current_ticket.tk_level)++;
	}

	void	dec_level(){
		br_current_ticket.tk_level--;
	}

	void	recoil_to_level(long target_lev, row<reason>& resns){
		long lev = level();
		BRAIN_CK_0(lev != ROOT_LEVEL);

		DBG_PRT(14, os << "ordered_trail ";
			br_trail.copy_to(br_tmp_edge, 1);
			br_tmp_edge.mix_sort(cmp_choice_idx_lt);
			os << br_tmp_edge;
		);

		DBG_PRT(14, os << "trail " << br_trail);
		DBG_PRT(14, os << "chosen " << br_chosen);

		quanton* qua = NULL;
		long qua_lev = INVALID_LEVEL;
		MARK_USED(qua_lev);
		charge_t qua_chg = cg_neutral;
		MARK_USED(qua_chg);

		bool end_of_recoil = (lev <= target_lev);
		while(br_trail.size() > 0){

			bool end_of_lev = (br_trail.last()->qlevel() != lev);
			if(end_of_lev){
				BRAIN_CK_0((br_trail.last()->qlevel() + 1) == lev);
				BRAIN_CK_0(lev != ROOT_LEVEL);
				dec_level();
				lev = level();

				end_of_recoil = (lev <= target_lev);
				if(end_of_recoil){
					break;
				}
				BRAIN_CK_0(lev != ROOT_LEVEL);
			}

			qua = br_trail.last();
			qua_lev = qua->qlevel();
			qua_chg = qua->get_charge();

			qua->set_charge(this, NULL, cg_neutral);
		}
	}

	// aux func
	bool	is_choice(long q_idx){
		BRAIN_CK_0(q_idx >= 0);
		BRAIN_CK_0(q_idx < br_trail.size());
		quanton* qua1 = br_trail[q_idx];
		if(q_idx == 0){
			BRAIN_CK_0(qua1 == &(quanton::ROOT_QUANTON));
			return false;
		}
		BRAIN_CK_0(q_idx >= 1);
		quanton* qua0 = br_trail[q_idx - 1];
		long lev = qua1->qlevel();
		BRAIN_CK_0(lev != INVALID_LEVEL);
		if(qua0->qlevel() < lev){
			BRAIN_CK_0(qua1->get_source() == NULL);
			BRAIN_CK_0((qua0->qlevel() + 1) == lev);
			return true;
		}
		return false;
	}

	bool	ck_current_ticket();
	bool	ck_motives(row<quanton*>& mots);
	//bool	ck_choices(bool after = false);
	bool	ck_trail();
	void	print_trail();
	//void	print_satifying(const char* log_assig_nm);
	std::ostream& 	print_all_quantons(std::ostream& os, long ln_sz, std::string ln_fd);

	bool	brn_compute_binary();
	bool	brn_compute_dots(bool only_orig);
	bool	brn_compute_dots_of(row<quanton*>& assig, bool only_orig = false);

	void		get_quantons_from_lits(row_long_t& all_lits, long first, long last, row<quanton*>& neu_quas);
	void		add_neuron_from_lits(row_long_t& all_lits, long first, long last);
	void		load_instance(long num_neu, long num_var, row_long_t& load_ccls);
	void		save_instance(long& num_ccls, long& num_vars, long& num_lits, row_long_t& save_ccls);

	void		load_it();
	void		solve_it();
	void		simplify_it(long& num_ccls, long& num_vars, long& num_lits, row_long_t& after_ccls);

	void		set_assig();

	void		check_timeout();
	void		check_sat_assig();

	instance_info&	get_my_inst(){
		BRAIN_CK(br_pt_inst != NULL_PT);
		return *br_pt_inst;
	}

	void		set_result(satisf_val re);
	satisf_val	get_result(){
		instance_info& inst_info = get_my_inst();
		satisf_val the_result = inst_info.ist_result;
		return the_result;
	}

	std::ostream& 	print_brain(std::ostream& os);

	std::ostream& 	dbg_ic_prt_dotty_label(std::ostream& os);
	std::ostream& 	dbg_ic_prt_dotty_file(std::ostream& os, long style = 0);
	void		dbg_ic_print();


};

//=================================================================================================
// INLINES dependant on class declarations  

inline
void	due_periodic_prt(void* pm, double curr_secs){
	MARK_USED(pm);
	if(GLB.out_os != NULL_PT){
		PRT_OUT(0, GLB.print_stats(*GLB.out_os, curr_secs));
	}
}

/*
inline 
bool
ticket::is_active(brain* brn){
	if(!is_valid()){ return false; }
	if(tk_trail_sz > brn->br_trail.size()){ return false; }
	long tk_trl_idx = trail_idx();
	quanton* qua = brn->br_trail[tk_trl_idx];
	bool acve = (qua->qrecoil() == tk_recoil);
	BRAIN_CK(!acve || (qua->qlevel() == tk_level));
	return acve;
}*/

inline 
void
ticket::update_ticket(brain* brn){
	tk_recoil = brn->recoil();
	tk_level = brn->level();
	//tk_trail_sz = brn->br_trail.size();
}

inline 
void			
quanton::tunnel_swapop(long idx_pop){
	long idx_mov = qu_tunnels.size() - 1;
	neuron* neu2 = qu_tunnels.last();
	BRAIN_CK(neu2->ck_tunnels());
	qu_tunnels.swapop(idx_pop);
	if(idx_pop != idx_mov){ 
		BRAIN_CK(qu_tunnels[idx_pop] == neu2);
		if(neu2->ne_fibres[0] == this){
			BRAIN_CK(neu2->ne_fibre_0_idx == idx_mov);
			neu2->ne_fibre_0_idx = idx_pop;
		} else {
			BRAIN_CK(neu2->ne_fibres[1] == this);
			BRAIN_CK(neu2->ne_fibre_1_idx == idx_mov);
			neu2->ne_fibre_1_idx = idx_pop;
		}
		BRAIN_CK(neu2->ck_tunnels());
	}
}

inline 
void			
quanton::set_source(brain* brn, neuron* neu){
	BRAIN_CK((qu_source == NULL) || (qu_inverse->qu_source == NULL));
	BRAIN_CK((qu_inverse->qu_source == NULL) || (neu == NULL));
	qu_source = neu;
	if(neu == NULL){ 
		qu_inverse->qu_source = NULL;
		DBG_SOURCES(
			qu_dbg_ic_all_sources.clear();
			qu_inverse->qu_dbg_ic_all_sources.clear();
		)
		return; 
	}
	BRAIN_CK_0(qu_source != NULL);
	DBG(
		qu_source->ck_all_neg(brn, 1);
	);
}

inline 
void		
quanton::set_charge(brain* brn, neuron* neu, charge_t cha){
	BRAIN_CK(ck_charge(brn));
	BRAIN_CK(this != &ROOT_QUANTON);

	BRAIN_CK((cha == cg_positive) || (cha == cg_neutral));
	BRAIN_CK(get_charge() != cg_negative);
	BRAIN_CK((get_charge() != cg_positive) || (cha == cg_neutral));
	BRAIN_CK((get_charge() != cg_neutral) || (cha == cg_positive));

	if(brn->level() == ROOT_LEVEL){ 
		neu = NULL; 
	}

	qu_charge = cha;
	qu_inverse->qu_charge = negate_trinary(cha);
	set_source(brn, neu);

	if(get_charge() == cg_neutral){
		DBG_PRT(15, os << "unbind " << *this);
		BRAIN_CK(neu == NULL);
		quanton* qua_trl = brn->br_trail.pop();
		MARK_USED(qua_trl);
		BRAIN_CK(qua_trl == this);
		BRAIN_CK(qlevel() != ROOT_LEVEL);

		quanton* lst = brn->br_trail.last();
		if(lst->qlevel() < qlevel()){
			BRAIN_CK(lst->qlevel() + 1 == qlevel());
			quanton* qua_cho = brn->br_chosen.pop();
			MARK_USED(qua_cho);
			BRAIN_CK(qua_cho == this);
		}

		if(qu_choice_idx < brn->br_choices_lim){ 
			BRAIN_CK_0(qu_choice_idx >= 0);
			brn->br_choices_lim = qu_choice_idx; 
		}

		qu_dbg_ic_trail_idx = INVALID_IDX;
		qu_inverse->qu_dbg_ic_trail_idx = INVALID_IDX;

		qu_charge_tk.init_ticket();
		qu_inverse->qu_charge_tk.init_ticket();
		qu_charge_tk.tk_recoil = brn->recoil();
	} else {
		qu_dbg_ic_trail_idx = brn->br_trail.size();
		qu_inverse->qu_dbg_ic_trail_idx = brn->br_trail.size();

		quanton* lst = brn->br_trail.last();
		brn->br_trail.push(this);

		qu_charge_tk.update_ticket(brn);
		qu_inverse->qu_charge_tk.update_ticket(brn);

		if((lst->qlevel() < qlevel()) || (qlevel() == ROOT_LEVEL)){
			BRAIN_CK((lst->qlevel() + 1 == qlevel()) 
					 || (qlevel() == ROOT_LEVEL));
			brn->br_chosen.push(this);
		}

		DBG_PRT(15, os << "BIND " << this; 
			if(neu != NULL){ os << " src: "; os << neu; }
		);
	}

	BRAIN_CK(ck_charge(brn));

}

inline
comparison	cmp_long_id(quanton* const & qua1, quanton* const & qua2){
	return cmp_long(qua1->qu_id, qua2->qu_id);
}


inline
comparison	cmp_choice_idx_lt(quanton* const & qua1, quanton* const & qua2){
	return cmp_long(qua1->qu_choice_idx, qua2->qu_choice_idx);
}

inline
comparison	cmp_choice_idx_gt(quanton* const & qua1, quanton* const & qua2){
	return cmp_long(qua2->qu_choice_idx, qua1->qu_choice_idx);
}

inline
comparison	cmp_qlevel(quanton* const & qua1, quanton* const & qua2){
	long	qlev1 = qua1->qlevel();
	long	qlev2 = qua2->qlevel();
	bool inv1 = (qlev1 == INVALID_LEVEL);
	bool inv2 = (qlev2 == INVALID_LEVEL);
	// sort them in inverse order with inv as the max:
	if(inv1 && inv2){ return 0; }
	if(inv1 && !inv2){ return -1; }
	if(!inv1 && inv2){ return 1; }
	if(qlev1 == qlev2){
		return cmp_long(qua2->get_charge(), qua1->get_charge());
	}
	return cmp_long(qlev2, qlev1);
}

inline
charge_t negate_trinary(charge_t val){
	if(val == cg_negative){
		return cg_positive;
	}
	if(val == cg_positive){
		return cg_negative;
	}
	return cg_neutral;
}

/*
inline
void	negate_quantons(row<quanton*>& qua_row){
	for(long kk = 0; kk < qua_row.size(); kk++){
		qua_row[kk] = qua_row[kk]->qu_inverse;
	}
}
*/

inline
bool	compute_reasons(row<reason>& rsns){
	if(rsns.size() == 0){
		return true;
	}

	bool resp = true;
	long ii;
	for(ii = 0; ii < rsns.size(); ii++){

		resp = rsns[ii].rsn_compute_dots();
		if(!resp){
			break;
		}
	}

	return resp;
}

inline
void	append_reasons(row<reason>& dest, row<reason>& src){
	for(long kk = 0; kk < src.size(); kk++){
		dest.inc_sz();
		src[kk].rsn_copy_to(dest.last());
	}
}

inline
void	get_ids_of(row<quanton*>& quans, row_long_t& the_ids){
	the_ids.clear();
	for(long kk = 0; kk < quans.size(); kk++){
		quanton& qua = *(quans[kk]);
		the_ids.push(qua.qu_id);
	}
}

//=================================================================================================
// printing funcs

DEFINE_PRINT_FUNCS(ticket)
DEFINE_PRINT_FUNCS(quanton)
DEFINE_PRINT_FUNCS(neuron)
DEFINE_PRINT_FUNCS(reason)

//=================================================================================================
// other funcs

std::string	dbg_name(std::string pref, long seq, std::string suf);
void	system_exec(std::ostringstream& strstm);

//=================================================================================================
// central.cpp funcs

std::ostream&	test_open_out(std::ofstream& os);
void		test_cnf_join();
void		test_cnf_as_ttnf(bool smpfy_it);
void		test_cnf_shuffle();
void		test_simplify_cnf();

void		call_solve_instance(debug_info& dbg_inf);
void		do_instance(debug_info& dbg_inf);
void		print_dimacs_of(std::ostream& os, row<long>& all_lits, long num_cla, long num_var);


#endif		// BRAIN_H

