

/*************************************************************

bible_sat

brain.cpp  
(C 2010) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com

Classes and that implement the neural network.

--------------------------------------------------------------*/

#include <execinfo.h>	// trace stack 
#include <cxxabi.h>	// trace stack 
#include <cstring>

#include "solver.h"
#include "brain.h"

//============================================================
// random generator

long gen_random_num(long min, long max);

// search CONFIG_DBG_IC to config dbg_ic

//============================================================
// Some code for debug purposes

std::string	dbg_name(std::string pref, long seq, std::string suf){
	std::ostringstream stm1;
	long ancho = 3;
	char ch_cero = '0';
	stm1 << DBG_DIR << pref;
	stm1.width(ancho);
	stm1.fill(ch_cero);
	stm1 << seq << suf;
	std::string nm = stm1.str();
	return nm;
}

//============================================================
// aux funcs

std::string	
trinary_to_str(charge_t obj){
	if(obj == cg_positive){
		return "pos";
	} else if(obj == cg_negative){
		return "neg";
	} else {
		BRAIN_CK_0(obj == cg_neutral);
		return "ntr";
	}
	return "???";
}

// ck funcs

bool
brain::ck_current_ticket(){
	BRAIN_CK_0(br_trail.size() > 0);
	ticket tk = br_trail.last()->qu_charge_tk;
	MARK_USED(tk);
	BRAIN_CK_0((br_current_ticket.tk_level == 0) || (tk.tk_recoil == br_current_ticket.tk_recoil));
	BRAIN_CK_0(tk.tk_level == br_current_ticket.tk_level);
	//BRAIN_CK_0(tk.tk_trail_sz == br_trail.size());
	return true;
}

bool
brain::ck_motives(row<quanton*>& mots){
	for(long ii = 0; ii < mots.size(); ii++){
		quanton* mot = mots[ii];
		MARK_USED(mot);
		BRAIN_CK_0(mot->qlevel() <= level());
		BRAIN_CK_0(mot->get_charge() == cg_negative);
	}
	return true;
}

/*
bool
brain::ck_choices(bool after){
	BRAIN_CK_0(br_choices_lim <= br_choices.size());
	long lim_cho = br_choices_lim;
	if(after){
		lim_cho--;
	}
	long last_lev = INVALID_LEVEL;
	for(long ii = 0; ii < lim_cho; ii++){
		quanton* qua = br_choices[ii];
		ticket& q_tk = qua->qu_charge_tk;
		long q_trl_idx = q_tk.trail_idx();
		bool is_cho = is_choice(q_trl_idx);
		quanton* q_trl = br_trail[q_trl_idx];
		MARK_USED(q_trl);
		long q_lev = qua->qlevel();

		if(is_cho || (last_lev == INVALID_LEVEL)){
			last_lev = q_lev;
		} else {
			BRAIN_CK_0(q_lev <= last_lev);
		}
	
		BRAIN_CK_0(q_trl->get_positon() == qua);
		BRAIN_CK_0(q_tk.is_valid());
		BRAIN_CK_0(qua->qu_choice_idx == ii);
		BRAIN_CK_0(qua->qu_spin == cg_positive);
		BRAIN_CK_0(qua->get_charge() != cg_neutral);
	}
	return true;
}*/

bool	
quanton::ck_charge(brain* brn){
	if((qlevel() == ROOT_LEVEL) && (get_source() != NULL)){
		abort_func(-1, brn->br_file_name.c_str());
	}
	BRAIN_CK_0(	(get_charge() == cg_positive) || 
			(get_charge() == cg_neutral) || 
			(get_charge() == cg_negative) );
	BRAIN_CK_0(	(qu_inverse->get_charge() == cg_positive) || 
			(qu_inverse->get_charge() == cg_neutral) || 
			(qu_inverse->get_charge() == cg_negative) );
	//BRAIN_CK_0((get_charge() * cg_negative) == qu_inverse->get_charge());
	BRAIN_CK_0(negate_trinary(get_charge()) == qu_inverse->get_charge());
	return true;
}

bool
brain::ck_trail(){
	std::ostream& os = t_dbg_os;

	ticket last;
	long num_null_src = 0;
	BRAIN_CK_0(br_trail.size() > 0);
	BRAIN_CK_0(br_trail[0] == &(quanton::ROOT_QUANTON));
	last = br_trail[0]->qu_charge_tk;
	quanton* last_choice = NULL;
	long kk = 0;
	for(long ii = 1; ii < br_trail.size(); ii++){
		quanton* qua = br_trail[ii];

		if((qua->qlevel() == ROOT_LEVEL) && (qua->get_source() != NULL)){
			os << "ck_trail CASE 1" << std::endl;
			print_trail();
			abort_func(-1, br_file_name.c_str());
		}

		if((qua->qu_source == NULL) && (qua->qlevel() != 0)){
			num_null_src++;
		}
		if(qua->get_charge() == cg_neutral){
			os << "ck_trail CASE 2" << std::endl;
			print_trail();
			abort_func(-1, br_file_name.c_str());
		}
		if(qua->has_dot()){
			os << "ck_trail CASE 3" << std::endl;
			print_trail();
			abort_func(-1, br_file_name.c_str());
		}
		qua->ck_charge(this);
		// TK_LE
		
		//if(! (last < qua->qu_charge_tk)){
		/*
		if(! cmp_ticket_lt(last, qua->qu_charge_tk)){
			os << "ck_trail CASE 4" << std::endl;
			print_trail();
			abort_func(-1, br_file_name.c_str());
		}*/
		
		bool cho = is_choice(ii);
		if(cho){
			last_choice = qua;
			if((kk >= br_chosen.size()) || (br_chosen[kk] != qua)){
				std::cout << "qua= " << qua << std::endl;
				if(kk < br_chosen.size()){
					std::cout << "chosen_qua= " << br_chosen[kk] << std::endl;
				}
				print_trail();
				std::cout << "chosen" << std::endl;
				std::cout << br_chosen << std::endl;
			}
			BRAIN_CK_0(kk < br_chosen.size());
			BRAIN_CK_0(br_chosen[kk] == qua);
			kk++;
		}
		if(qua->qlevel() == ROOT_LEVEL){
			BRAIN_CK_0(kk < br_chosen.size());
			BRAIN_CK_0(br_chosen[kk] == qua);
			kk++;
		}

		if(!cho && (qua->get_source() == NULL))
		{
			quanton* cls = qua;
			BRAIN_CK_0(cls->qlevel() == qua->qlevel());
			if((cls != last_choice) && (cls->qlevel() != 0)){
				os << "ck_trail CASE 5" << std::endl;
				print_trail();
				abort_func(-1, br_file_name.c_str());
			}
		}
		last = qua->qu_charge_tk;
	}
	if(num_null_src != level()){
		os << "ck_trail CASE 6" << std::endl;
		print_trail();
		abort_func(-1, br_file_name.c_str());
	}
	return true;
}


//============================================================
// print methods

void
brain::print_trail(){
	std::ostream& os = t_dbg_os;
	/*
	for(long kk = 0; kk < br_trail.size(); kk++){
		br_trail[kk]->print_quanton(os);
		os << " ";
	}
	os << std::endl;*/
	os << br_trail << std::endl;
	os.flush();
}

/*
void
brain::print_satifying(const char* log_assig_nm){
	if(log_assig_nm == NULL){
		return;
	}

	std::ofstream log_stm;
	if((br_file_name != NULL) && (log_assig_nm != NULL)){
		log_stm.open(log_assig_nm, std::ios::app);
	}
	if(log_stm.is_open()){
		log_stm << "c ====================" << std::endl;
		log_stm << "c " << br_file_name << std::endl;
		satisfying.print_row_data(log_stm, false, " ", -1, -1, false, 20);
		log_stm << " 0" << std::endl;
	}
	log_stm.close();
}
*/

std::ostream& 
brain::print_all_quantons(std::ostream& os, long ln_sz, std::string ln_fd){
	//print_trail();
	//std::cout << std::endl << std::endl;
	BRAIN_CK_0(br_choices.size() == br_positons.size());
	long num_null_src = 0;
	for(long ii = 0; ii < br_choices.size(); ii++){
		if((ii > 0) && ((ii % ln_sz) == 0)){
			os << ln_fd;
		}

		quanton* qua = br_choices[ii];
		std::string pre = " ";
		std::string suf = " ";
		charge_t chg = qua->get_charge();
		if(chg == cg_negative){
			pre = "[";
			suf = "]";
		} else if(chg == cg_neutral){
			pre = "";
			suf = "_?";
		}
		if(qua->qlevel() == ROOT_LEVEL){
			os << "r";
		}
		if(	(qua->get_source() == NULL) && 
			(qua->qlevel() != 0) &&
			(chg != cg_neutral)
		){
			num_null_src++;
			os << "*";
		} else {
			os << " ";
		}
		os << pre << qua << suf << " ";
	}
	BRAIN_CK_0(num_null_src == level());
	os.flush();
	return os;
}

//============================================================
// check SAT result

void	set_dots_of(row<quanton*>& quans){
	for(long ii = 0; ii < quans.size(); ii++){
		quanton* qua = quans[ii];
		if(qua != &(quanton::ROOT_QUANTON)){
			BRAIN_CK_0(! qua->has_dot());
			qua->set_dot();
		}
	}
}

void	reset_dots_of(row<quanton*>& quans){
	for(long ii = 0; ii < quans.size(); ii++){
		quanton* qua = quans[ii];
		if(qua != &(quanton::ROOT_QUANTON)){
			BRAIN_CK_0(qua->has_dot());
			qua->reset_dot();
		}
	}
}

bool
brain::brn_compute_binary(){
	long ii;
	for(ii = 0; ii < br_neurons.size(); ii++){
		if(!(br_neurons[ii].neu_compute_binary())){
			return false;
		}
	}
	return true;
}

//for IS_SAT_CK
bool
brain::brn_compute_dots(bool only_orig){
	long num_neu_ck = br_neurons.size();
	for(long ii = 0; ii < num_neu_ck; ii++){
		neuron& neu = br_neurons[ii];
		if(! neu.ne_original){
			slv().batch_stat_added_fires.add_val(neu.ne_dbg_num_fires);
		}
		if(! neu.neu_compute_dots()){
			return false;
		}
	}
	return true;
}

//for IS_SAT_CK
bool
brain::brn_compute_dots_of(row<quanton*>& assig, bool only_orig){
	set_dots_of(assig);

	long ii = 0;
	while((ii < assig.size()) && (assig[ii]->qlevel() == ROOT_LEVEL)){
		if(assig[ii]->qu_dot != cg_positive){
			return false;
		}
		ii++;
	}

	bool resp = brn_compute_dots(only_orig);
	reset_dots_of(assig);
	return resp;
}

//============================================================
// quanton methods

quanton		quanton::ROOT_QUANTON;

std::ostream&
quanton::print_quanton(std::ostream& os, bool from_pt){
	MARK_USED(from_pt);

	if(this == &ROOT_QUANTON){
		os << "ROOT_QUA";
		os.flush();
		return os;
	}

	bool is_neg = (qu_charge == cg_negative);
	bool is_pos = (qu_charge == cg_positive);

	if(from_pt){
		//os << "#";
		if(is_pos){ os << '/'; }
		if(is_neg){ os << '\\'; }
		os << qu_id; 
		if(is_neg){ os << '/'; }
		if(is_pos){ os << '\\'; }
		if(get_source() == NULL_PT){ os << "." << qlevel(); }
		os.flush();
		return os;
	}

	os << "\n";
	os << "LIT " << qu_id << " ";
	os << "i" << qu_index << " ";
	os << "INV " << qu_inverse->qu_id << "\n";
	os << "CHG " << trinary_to_str(qu_charge) << " ";
	os << "chtk{" << qu_charge_tk << "} ";
	os << "sp_" << trinary_to_str(qu_spin) << " ";
	os << "st_" << qu_choice_idx << " ";
	os << "d_" << trinary_to_str(qu_dot) << "\n";
	os << "src ";
	if(get_source() != NULL){
		os << get_source();
	} else {
		os << "NULL";
	}
	os << "\n";
	os << "ne_tnn_" << qu_tunnels << "\n";
	os << "\n";
	os.flush();
	return os;
}

//============================================================
// neuron methods

row<quanton*>	neuron::ne_dbg_fibres;

solver& 
neuron::slv(){
	debug_info* dbg_info = NULL;
	BRAIN_DBG(dbg_info = ne_dbg_info);
	if((dbg_info != NULL) && (dbg_info->dbg_brn != NULL)){
		return dbg_info->dbg_brn->slv();
	}
	throw brain_exception(brx_no_solver_info, "NEURON without not debugging calling slv()");
}

quanton*
neuron::update_fibres(row<quanton*>& synps, bool orig){
	long num_neutral = 0;
	long num_neg_chgs = 0;
	if(ne_fibres != NULL){
		ck_tunnels();
		ne_fibres[0]->tunnel_swapop(ne_fibre_0_idx);
		ne_fibre_0_idx = INVALID_IDX;
		ne_fibres[1]->tunnel_swapop(ne_fibre_1_idx);
		ne_fibre_1_idx = INVALID_IDX;
		tpl_free<quanton*>(ne_fibres, ne_fibres_sz);
		ne_fibres = NULL;
		ne_fibres_sz = 0;
	}
	ne_fibres_sz = synps.size();
	if(ne_fibres_sz > 0){
		BRAIN_CK_0(ne_fibres_sz > 1);

		row<quanton*> tmp_fibs;
		tmp_fibs.set_cap(ne_fibres_sz);
		synps.copy_to(tmp_fibs);
		tmp_fibs.mix_sort(cmp_qlevel);
		DBG_PRT(16, os << "update_syns " << tmp_fibs);

		ne_fibres = tpl_malloc<quanton*>(ne_fibres_sz);

		long no_neg_idx0 = INVALID_IDX;
		long no_neg_idx1 = INVALID_IDX;
		MARK_USED(no_neg_idx1);

		DBG(
			long id0 = INVALID_IDX;
			long id1 = INVALID_IDX;
		);
		for(long ii = 0; ii < ne_fibres_sz; ii++){
			quanton* qua = tmp_fibs[ii];
			//quanton* qua = synps[ii];
			DBG(
				id0 = id1;
				id1 = qua->abs_id();
				BRAIN_CK_0(id0 != id1);
			);

			ne_fibres[ii] = qua;
			charge_t q_chg = qua->get_charge();

			if(q_chg == cg_negative){
				num_neg_chgs++;
			} else {
				no_neg_idx1 = no_neg_idx0;
				no_neg_idx0 = ii;
			}
			if(q_chg == cg_neutral){
				num_neutral++;
			}

		}
		BRAIN_CK_0(num_neg_chgs < ne_fibres_sz);

		tmp_fibs.clear(false, true);

		ne_fibres[0]->qu_tunnels.push(this);
		ne_fibre_0_idx = ne_fibres[0]->qu_tunnels.size() - 1;
		ne_fibres[1]->qu_tunnels.push(this);
		ne_fibre_1_idx = ne_fibres[1]->qu_tunnels.size() - 1;
		ck_tunnels();
	}
	BRAIN_CK_0((ne_fibres_sz > 0) || (num_neutral != 1));
	quanton* forced_qua = NULL;
	if(num_neg_chgs == (ne_fibres_sz - 1)){
		// ABORT CASE
		if(ne_fibres[0]->get_charge() == cg_negative){
			debug_info* dbg_inf = get_dbg_info();
			if((dbg_inf != NULL) && (dbg_inf->dbg_brn != NULL)){
				dbg_inf->dbg_brn->print_trail();
			}
			std::cout << this << std::endl;;
			std::cout << ne_fibres[0] << std::endl;;
			abort_func(0, "ABORT !. ne_fibres[0]->is_neg !!!");
		}
		BRAIN_CK_0(ne_fibres[0]->get_charge() != cg_negative);
		forced_qua = forced_quanton();
	}

	ne_edge_tk.init_ticket();

	return forced_qua;
}

quanton*		
neuron::get_prt_fibres(row<quanton*>& tmp_fibres, bool sort_them){
	quanton* signl_qua = NULL;
	tmp_fibres.clear();
	tmp_fibres.set_cap(ne_fibres_sz);
	for (long ii = 0; ii < ne_fibres_sz; ii++){
		quanton* qua = ne_fibres[ii];
		tmp_fibres.push(qua);
		if(qua->get_source() == this){
			BRAIN_CK_0(signl_qua == NULL);
			signl_qua = qua;
		}
	}
	if(sort_them){
		tmp_fibres.mix_sort(cmp_long_id);
	}
	return signl_qua;
}

bool
quanton::ck_all_tunnels(){
	for(long ii = 0; ii < qu_tunnels.size(); ii++){
		neuron* neu = qu_tunnels[ii];
		neu->ck_tunnels();
	}
	return true;
}

action_t
neuron::neu_tunnel_signals(brain* brn, quanton* qua){
	DBG(if(slv().op_dbg_no_learning && ! ne_original){
		return ac_go_on;
	});

	DBG_PRT(17, os << "tunneling " << qua << " in " << this);
	BRAIN_CK(ne_fibres != NULL);
	BRAIN_CK(ck_tunnels());
	if(ne_fibres[0] == qua){
		swap_fibres_0_1();
		BRAIN_CK_2(ne_fibres[0]->ck_all_tunnels());
		BRAIN_CK_2(ne_fibres[1]->ck_all_tunnels());
	}
	BRAIN_CK(ne_fibres[1] == qua);
	if(ne_fibres[0]->get_charge() == cg_positive){
		BRAIN_CK_2(ne_fibres[0]->ck_all_tunnels());
		BRAIN_CK_2(ne_fibres[1]->ck_all_tunnels());
		return ac_go_on;	// neu_is_satisf
	}

	long old_max = ne_fibres_sz - 1;
	/*
	if(ne_edge_tk.is_active(brn)){
		BRAIN_CK(ne_edge <= old_max);
		BRAIN_CK(ck_all_neg(brn, ne_edge));
		old_max = ne_edge;
	}*/
	DBG(
		long new_max = old_max;
		long max_lev = ne_fibres[new_max]->qlevel();
		long qua_lev = qua->qlevel()
	);

	for(long ii = old_max; ii > 1; ii--){
		charge_t chg = ne_fibres[ii]->get_charge();

		if(chg != cg_negative){
			BRAIN_CK(max_lev <= qua_lev);
			swap_fibres_1(ii);
			ne_edge = ii;
			ne_edge_tk.update_ticket(brn);

			BRAIN_CK(cmp_ticket_eq(ne_edge_tk, brn->br_current_ticket));
			//BRAIN_CK(ne_edge_tk.is_active(brn));
			BRAIN_CK(ne_fibres[ii]->get_charge() == cg_negative);

			BRAIN_CK_2(ne_fibres[0]->ck_all_tunnels());
			BRAIN_CK_2(ne_fibres[1]->ck_all_tunnels());
			BRAIN_CK_2(ne_fibres[ii]->ck_all_tunnels());
			return ac_go_on;	// neu_is_satisf
		}
		DBG(
			if(ne_fibres[ii]->qlevel() > max_lev){
				new_max = ii;
				max_lev = ne_fibres[new_max]->qlevel();
			}
		);
	}

	DBG(ck_all_neg(brn, 1));

	quanton* forced_qua = forced_quanton();	
	action_t sgl_nxt = brn->send_signal(forced_qua, this);
	
	BRAIN_CK_2(ne_fibres[0]->ck_all_tunnels());
	BRAIN_CK_2(ne_fibres[1]->ck_all_tunnels());
	return sgl_nxt;
}

bool
neuron::ck_all_neg(brain* brn, long from){
	bool all_ok = true;
#ifdef FULL_DEBUG
	for(long ii = from; ii < ne_fibres_sz; ii++){
		all_ok = (all_ok && (ne_fibres[ii]->get_charge() == cg_negative) );
	}
#endif
	return all_ok;
}

std::ostream&
neuron::print_neuron(std::ostream& os, bool from_pt){
	return print_neu_base(os, ! from_pt, true, true);
}

std::ostream&	
neuron::print_neu_base(std::ostream& os, bool detail, bool prt_src, bool sort_fib){

	quanton* signl_qua = get_prt_fibres(ne_dbg_fibres, sort_fib);

	if(! detail){
		os << ne_index << " ";
		os << ne_dbg_fibres;

		if(prt_src && (signl_qua != NULL)){
			os << "src_of:";
			if(signl_qua != NULL){
				os << signl_qua;
			} else {
				os << "NULL";
			}
		}

		os.flush();
		return os;
	}

	os << "\n";
	os << "INDEX " << ne_index << " ";
	os << "in_ " << ((ne_original)?("yes"):("no")) << "\n";
	os << "fz: " << ne_fibres_sz << " ";
	os << "fb[ ";
	for(long ii = 0; ii < ne_fibres_sz; ii++){
		os << ne_fibres[ii] << " ";
	}
	os << "] ";
	ck_tunnels();
	os << "f0i_" << ne_fibre_0_idx << " ";
	os << "f1i_" << ne_fibre_1_idx << " ";
	os << "\n";

	os << "eg_" << ne_edge << " ";
	os << "egtk{" << ne_edge_tk << "} ";
	os << "src_of:";
	if(signl_qua != NULL){
		os << signl_qua;
	} else {
		os << "NULL";
	}
	os << "\n";



	os << "syns" << ne_dbg_fibres << "\n";
	os << "\n";

	os.flush();
	return os;
}

//============================================================
// brain methods

void
quanton::qua_tunnel_signals(brain* brn, row<neuron*>& cnflicts){
	bool stop_tunn = false;
	BRAIN_CK_0(get_charge() != cg_neutral);

	BRAIN_CK(get_charge() != cg_neutral);
	BRAIN_CK(qlevel() != INVALID_LEVEL);

	for(long ii = qu_tunnels.size() - 1; (ii >= 0); ii--){
		if(stop_tunn){ break; }
		if(! cnflicts.is_empty()){ break; }

		neuron* neu = qu_tunnels[ii];
		BRAIN_CK(neu != NULL);
		action_t resp = neu->neu_tunnel_signals(brn, this);
		if(resp != ac_go_on){
			DBG_PRT(18, os << "**confict** " << neu);
			if(!(neu->ne_is_conflict)){
				cnflicts.push(neu);
				neu->ne_is_conflict = true;
			}
			if(resp == ac_stop){
				stop_tunn = true;
				break;
			}
		}
	} // end for (ii)
	BRAIN_CK_2(ck_all_tunnels());
}

void
brain::brn_tunnel_signals(row<neuron*>& cnflicts){
	BRAIN_CK(ck_trail());
	DBG(
		ticket tk1;
		tk1.update_ticket(this);
		ck_current_ticket();
		BRAIN_CK_0(cnflicts.size() == 0);
	);

	cnflicts.clear();
	while((br_signals.size() > 0) && cnflicts.is_empty()){
		quanton* inv = br_signals.pick()->qu_inverse;
		inv->qua_tunnel_signals(this, cnflicts);

		DBG_PRT(19, os << "finished tunnelling " << inv);
	} // end while

	br_signals.clear();

	DBG(
		BRAIN_CK_0(br_signals.size() == 0);
		ck_current_ticket();
		ticket tk2;
		tk2.update_ticket(this);
		BRAIN_CK_0(cmp_ticket_eq(tk1, tk2));
	);
}

action_t	 
brain::send_signal(quanton* qua, neuron* neu){
	action_t sg_resp = ac_invalid;

	if(neu != NULL_PT){ neu->ne_dbg_num_fires++; }

	DBG_SOURCES(
		if((qua != NULL) && (neu != NULL)){ 
			qua->qu_dbg_ic_all_sources.push(neu);		
		}
	)
	
	if(qua->get_charge() != cg_neutral){
		if(qua->get_charge() == cg_negative){
			if(level() == ROOT_LEVEL){
				sg_resp = ac_stop;
			} else {
				sg_resp = ac_recoil;
			}
		} else {
			sg_resp = ac_go_on;
		}
	} else {
		qua->set_charge(this, neu, cg_positive);
		br_signals.push(qua);
		sg_resp = ac_go_on;
	}

	BRAIN_CK_0(sg_resp != ac_invalid);
	return sg_resp;
}

quanton*	
brain::choose_quanton(){ 
	quanton* qua = NULL;

	BRAIN_CK(ck_trail());
	//BRAIN_CK(ck_choices());

	for(long ii = br_choices_lim; ii < br_choices.size(); ii++){
		quanton* qua_ii = br_choices[ii];
		if(qua_ii->get_charge() == cg_neutral){
			qua = qua_ii;
			br_choices_lim = ii + 1;
			break;
		}
	}

	BRAIN_CK_0((qua == NULL) || (qua->get_charge() == cg_neutral));
	BRAIN_CK_0(	(qua == NULL) || 
			(qua->qu_spin == cg_positive) || 
			(qua->qu_spin == cg_negative));

	//BRAIN_CK(ck_choices(true));

	if((qua != NULL) && (br_choice_spin == cg_negative)){
		qua = qua->qu_inverse;
	}

	return qua;
}

void
brain::find_middle_reason_of(neuron* confl, reason& rsn, row<quanton*>& tmp_mots){

	long remains = 0;
	long trail_sz = br_trail.size();
	long trl_idx = trail_sz - 1;
	quanton* last_qua = NULL;
	long n_level = 0;

	BRAIN_CK(level() != ROOT_LEVEL);

	tmp_mots.clear();

	BRAIN_CK(confl != NULL);
	BRAIN_CK(confl->ne_fibres != NULL);

	neuron* nxt_src = confl;
	do{
		BRAIN_CK_0(nxt_src != NULL);

		DBG_PRT(20, os << "reasoning_cause_of_conflict " << nxt_src << " remains " << remains);

		BRAIN_CK(nxt_src->ne_fibres != NULL);
		BRAIN_CK(	(last_qua == NULL) || 
					(nxt_src->ne_fibres[0]->get_charge() == cg_positive) );
		BRAIN_CK(	(last_qua == NULL) || nxt_src->neu_compute_binary());
		long ii = (last_qua == NULL)?(0):(1);
		for(; ii < nxt_src->ne_fibres_sz; ii++){
			quanton* qua = nxt_src->ne_fibres[ii];
			BRAIN_CK(qua->get_charge() == cg_negative);

			if(!qua->has_dot()){
				DBG(std::string dbg_msg = "");
				qua->set_dot();
				if(qua->qlevel() == level()){
					DBG(dbg_msg = "dotting ");
					remains++;
				} else {
					DBG(dbg_msg = "motive ");
					tmp_mots.push(qua);
					n_level = max(n_level, qua->qlevel());
				}
				DBG_PRT(20, os << dbg_msg << qua << " remains " << remains);
			}
		}
		do{ 
			BRAIN_CK_0(trl_idx > 0);
			last_qua = br_trail[trl_idx--];
			BRAIN_CK_0(last_qua->qlevel() == level());
			DBG_PRT(20, os << "checking " << last_qua);
		} while(!last_qua->has_dot());

		DBG_PRT(20, os << "dotted found " << last_qua << " remains " << remains);
		nxt_src = last_qua->get_source();
		remains--;
		last_qua->reset_dot();

	} while(remains > 0);

	quanton* ast_qua = last_qua->qu_inverse; 
	tmp_mots.push(ast_qua);  

	BRAIN_CK(ast_qua->qlevel() == level());
	BRAIN_CK(n_level < level());
	BRAIN_CK(ast_qua->get_charge() == cg_negative);
	BRAIN_CK(ck_motives(tmp_mots));

	rsn.fill_reason(tmp_mots, confl, n_level);
	rsn.ra_forced = ast_qua;

	BRAIN_CK(rsn.ra_motives.size() > 0);
	BRAIN_CK(rsn.ra_confl == confl);
	BRAIN_CK(rsn.ra_target_level == n_level);
	BRAIN_CK(rsn.ra_target_level != INVALID_LEVEL);
	BRAIN_CK(rsn.ra_forced != NULL_PT);

	DBG_PRT(20, os << "reason " << rsn);

	BRAIN_CK(trail_sz == br_trail.size());
	BRAIN_CK(tmp_mots.size() > 0);
}

void
brain::find_reasons(row<neuron*>& confls, row<reason>& resns){
	resns.clear(true);
	resns.grow(confls.size() + 1); // leave one space 

	for(;confls.size() > 0;){
		neuron* cnfl = confls.pop();
		BRAIN_CK_0(cnfl->ne_is_conflict);
		cnfl->ne_is_conflict = false;

		resns.inc_sz();
		reason& rsn = resns.last();
		find_middle_reason_of(cnfl, rsn, br_tmp_motives);
		update_excited_level(rsn);

		DBG_PRT(21, os << "conflict" << cnfl << std::endl
			<< " rsn=" << rsn << std::endl
			<< " ex_lv=" << br_excited_level << std::endl;
			print_trail();
		);
	}
	BRAIN_CK_0(confls.size() == 0);
}

quanton*
brain::add_reason(reason& rsn){
	row<quanton*>& learnt = rsn.ra_motives;

	DBG_PRT(22, os << "adding " << rsn);

	BRAIN_CK_0(learnt.size() > 0);
	quanton* forced_qua = NULL;
	if(learnt.size() > 1){
		rsn.ra_neuron = add_neuron(learnt, forced_qua, false);
	} else {
		BRAIN_CK_0(learnt.size() == 1);
		BRAIN_CK_0(level() == ROOT_LEVEL);
		BRAIN_CK_0(rsn.ra_neuron == NULL);
		forced_qua = learnt[0];		
		// NOTE that when (learnt.size() == 1) then
		// 'rsn.ra_target_level' == ROOT_LEVEL
	}
	return forced_qua;
}

void
brain::init_excited_level(){
	br_excited_level = level();
}

void
brain::update_excited_level(reason& rsn){
	if(rsn.size() == 1){
		//stats.dbg_num_mono_rsns++;
		rsn.ra_target_level = ROOT_LEVEL;
		BRAIN_CK_0(rsn.ra_neuron == NULL);
	} 
	BRAIN_CK(rsn.ra_target_level != INVALID_LEVEL);
	if(rsn.ra_target_level < br_excited_level){
		br_excited_level = rsn.ra_target_level;
	}
	//rsn.ra_target_level = INVALID_LEVEL;
}

void
brain::init_loading(long num_qua, long num_neu){
	br_neurons.set_cap(2 * num_neu);
	br_neurons.clear(true); 

	br_choices.clear();

	br_choices.set_cap(num_qua);
	br_choices.fill(NULL, num_qua);

	// reset quantons
	br_positons.set_cap(num_qua);
	br_positons.clear(true);

	br_negatons.set_cap(num_qua);
	br_negatons.clear(true);
	for(long ii = 0; ii < num_qua; ii++){
		br_positons.inc_sz();
		br_negatons.inc_sz();

		quanton* qua_pos = &(br_positons.last());
		quanton* qua_neg = &(br_negatons.last());


		qua_pos->init_quanton(br_dbg_info, cg_positive, ii, qua_neg);
		qua_neg->init_quanton(br_dbg_info, cg_negative, ii, qua_pos);

		long idx = ii;
		if(br_choice_order == k_left_order){
			idx = num_qua - ii - 1;
		}

		BRAIN_CK_0(idx >= 0);
		BRAIN_CK_0(idx < num_qua);
		br_choices[idx] = qua_pos; 
		qua_pos->qu_choice_idx = idx;
		qua_neg->qu_choice_idx = idx;
	}
	BRAIN_CK_0(br_positons.size() == num_qua);
	BRAIN_CK_0(br_negatons.size() == num_qua);
	BRAIN_CK_0(br_choices.size() == num_qua);

	if(br_choice_order == k_random_order){
		set_random_choices();
	}

	br_tmp_fixing_quantons.set_cap(num_qua);
	br_tmp_motives.set_cap(num_qua);
	br_trail.set_cap(num_qua + 1);

	br_choices_lim = 0;

	br_trail.push(&(quanton::ROOT_QUANTON));

	br_current_ticket.tk_level = 0;
	br_current_ticket.tk_recoil = 1;

	quanton::ROOT_QUANTON.qu_charge_tk.update_ticket(this);

	init_excited_level();

}

bool
brain::init_sat(){
	//SYNS_CK

	/*
	stats.num_start_neu = stats.num_neurons;
	stats.num_start_qua = br_positons.size();
	stats.num_start_syn = stats.num_synapses;
	*/

	row<neuron*> ini_cnflicts;
	brn_tunnel_signals(ini_cnflicts);
	if(ini_cnflicts.size() > 0){
		return false;
	}
	BRAIN_CK_0(ini_cnflicts.size() == 0);

	return true;
}

void
brain::learn_reasons(row<reason>& resns){
	DBG_PRT(23, os << "**LEARNING**" << std::endl; print_trail(); );

	long ii = 0;
	for(ii = 0; ii < resns.size(); ii++){
		reason& rsn = resns[ii];
		BRAIN_CK_0(rsn.size() >= 1);
		quanton* forced_qua = rsn.ra_forced;
		BRAIN_CK(forced_qua != NULL_PT);

		quanton* f_qua = add_reason(rsn);
		MARK_USED(f_qua);
		BRAIN_CK(f_qua == forced_qua);
		forced_qua = f_qua;

		if(forced_qua != NULL){
			DBG_PRT(23, os << "added_forced_quanton: " << forced_qua;
				if(rsn.size() == 1){ os << " IS_SINGLETON"; } 
			);
			action_t ok = send_signal(forced_qua, rsn.ra_neuron);
			MARK_USED(ok);
			BRAIN_CK_0(ok == ac_go_on);
		}
	}

	if(dbginf().dbg_ic_active && dbginf().dbg_ic_after){
		dbg_ic_print();
	}

	resns.clear(true);
}

//trinary
void
brain::pulsate(row<neuron*>& cnflcts){
	BRAIN_CK(cnflcts.is_empty());

	brn_tunnel_signals(cnflcts);

	if(cnflcts.size() > 0){
		BRAIN_CK(cnflcts.size() == 1);

		DBG_PRT(24, 
			std::cout << "pulsate. BEFORE conflit treatment. Type ENTER to continue..." << std::endl;
			DO_GETCHAR
		);

		if(level() == ROOT_LEVEL){ 
			set_result(k_no_satisf);
			return;
		}

		DBG_PRT(25, os << cnflcts.size() << " **CONFLICTS**");

		init_excited_level();
		find_reasons(cnflcts, br_reasons);
		BRAIN_CK_0(br_reasons.size() > 0);

		if(dbginf().dbg_ic_active){
			dbg_ic_print();
		}

		BRAIN_CK_0(br_reasons.size() > 0);
		BRAIN_CK_0(br_signals.size() == 0);

		recoil_to_level(br_excited_level, br_reasons);
		inc_recoil();

		instance_info& inst_info = get_my_inst();
		inst_info.ist_num_laps++;
		DBG(
			if(br_dbg_info != NULL){ br_dbg_info->dbg_update_config_entries(); }
			check_timeout();
		);

		learn_reasons(br_reasons);

		BRAIN_CK_0(cnflcts.size() == 0);
		init_excited_level();

	} else {
		BRAIN_CK_0(cnflcts.size() == 0);

		quanton* qua = NULL;
		qua = choose_quanton();
		if(qua == NULL){
			set_result(k_yes_satisf);
			return;
		}

		inc_level();

		DBG_PRT(25, os << "**CHOICE**");


		action_t ok = send_signal(qua, NULL);
		MARK_USED(ok);
		BRAIN_CK_0(ok == ac_go_on);
	}
}

neuron*
brain::alloc_neuron(bool orig){
	neuron* neu = NULL;

	br_neurons.inc_sz();
	neu = &(br_neurons.last());
	neu->ne_index = br_neurons.size() - 1;
	//(stats.num_neurons)++;

	neu->ne_original = orig;

	BRAIN_CK_0(neu != NULL);
	BRAIN_CK_0(neu->ne_index != INVALID_IDX);

	return neu;
}

neuron*
brain::add_neuron(row<quanton*>& quans, quanton*& forced_qua, bool orig){
	neuron*	neu = alloc_neuron(orig);
	BRAIN_DBG(neu->ne_dbg_info = br_dbg_info);

	forced_qua = neu->update_fibres(quans, orig);

	// stats and dbg
	//stats.avg_neu_sz.add_val(neu->size());
	//(stats.num_synapses) += neu->size();

	DBG_PRT(26, os << "adding " << neu);

	// the neuron
	return neu;
}

std::ostream&
brain::print_brain(std::ostream& os){

	os << "reasons:" << br_reasons << "\n";
	os << "\n";
	os << "cho_spin_" << (long)br_choice_spin << "\n";
	os << "cho_ord_" << (long)br_choice_order << "\n";
	os << "\n NEURONS:\n" << br_neurons << "\n";
	os << "\n POSITONS:\n" << br_positons << "\n";
	os << "\n NEGATONS:\n" << br_negatons << "\n";
	os << "\n trail:\n" << br_trail << "\n";
	os << "signals:\n" << br_signals << "\n";
	os << "\n";
	os << "\n";
	os << "choices_lim:" << br_choices_lim << "\n";
	os << "\n choices:"; br_choices.print_row_data(os, true, " ", br_choices_lim, br_choices_lim); os << "\n";
	//os << "\n satisfying:" << satisfying << "\n";
	os << "\n";

	os.flush();
	return os;
}

void
brain::set_file_name_in_ic(std::string f_nam){
	if(f_nam.empty()){
		return;
	}
	std::string nam = br_file_name;
	long nm_sz = nam.size();
	for(long ii = 0; ii < nm_sz; ii++){
		if(nam[ii] == '\\'){
			nam[ii] = '|';
		}
	}
	br_file_name_in_ic = nam;
}

void
brain::config_brain(std::string f_nam){
	if(! f_nam.empty()){
		br_file_name = f_nam;
	}
	set_file_name_in_ic(f_nam);

	if(dbginf().dbg_ic_active){
		std::ostringstream o_str;
		o_str << "rm " << DBG_DIR << "*.dot";
		system_exec(o_str);
	}
	if(dbginf().dbg_ic_gen_jpg){
		std::ostringstream o_str;
		o_str << "rm " << DBG_DIR << "all_dot_to_jpg.bat";
		system_exec(o_str);
	}
}

void system_exec(std::ostringstream& strstm){
	std::string comm = strstm.str();
	const char* comm_cstr = comm.c_str();
	std::cout << comm_cstr << std::endl;
	system(comm_cstr);
}

void
brain::set_random_choices(){
	row<quanton*> tmp_bag;
	br_choices.copy_to(tmp_bag);

	BRAIN_CK_0(br_choices.size() == tmp_bag.size());
	long ii = 0;
	while(tmp_bag.size() > 0){
		long idx_pop = gen_random_num(0, tmp_bag.size());
		BRAIN_CK_0(idx_pop >= 0);
		BRAIN_CK_0(idx_pop < tmp_bag.size());
		quanton* qua = tmp_bag.swap_pop(idx_pop);
		br_choices[ii] = qua;
		qua->qu_choice_idx = ii;
		qua->qu_inverse->qu_choice_idx = ii;
		ii++;
	}
}

void
brain::load_it(){
	instance_info& inst_info = get_my_inst();

	br_start_load_tm = run_time();

	std::string f_nam = inst_info.get_f_nam();

	// dimacs loading

	dimacs_loader	the_loader;
	the_loader.ld_dbg_info = br_dbg_info;
	
	row<long> inst_ccls;
	the_loader.parse_file(f_nam, inst_ccls);

	// brain loading

	long num_neu = the_loader.ld_num_ccls;
	long num_var = the_loader.ld_num_vars;

	load_instance(num_neu, num_var, inst_ccls);
}

void
brain::set_result(satisf_val re){
	instance_info& inst_info = get_my_inst();
	satisf_val& the_result = inst_info.ist_result;

	BRAIN_CK(re != k_unknown_satisf);
	BRAIN_CK((the_result != k_yes_satisf) || (re != k_no_satisf));
	BRAIN_CK((the_result != k_no_satisf) || (re != k_yes_satisf));

	the_result = re;

	DBG_PRT(27, os << "RESULT " << satisf_val_nams[the_result]);
	DBG_PRT(28, os << "HIT ENTER TO CONTINUE...");
	DBG_COMMAND(28, getchar());
}

void
brain::get_quantons_from_lits(row_long_t& all_lits, long first, long last, row<quanton*>& neu_quas){
	neu_quas.clear();

	for(long ii = first; ii < last; ii++){
		BRAIN_CK(all_lits.is_valid_idx(ii));
		long nio_id = all_lits[ii];
	



		quanton* qua = get_quanton(nio_id);
		BRAIN_CK_0(qua != NULL_PT);
		neu_quas.push(qua);
	}
}

void
brain::add_neuron_from_lits(row_long_t& all_lits, long first, long last){
	row<quanton*>& quas = br_tmp_fixing_quantons;

	quas.clear();
	get_quantons_from_lits(all_lits, first, last, quas);

	DBG_PRT(29, os << "ADDING NEU=" << quas);

	BRAIN_CK_0(quas.size() > 0);
	if(quas.size() > 1){
		bool is_orig = true;
		quanton* forced_qua = NULL;
		add_neuron(quas, forced_qua, is_orig);
		BRAIN_CK_0(forced_qua == NULL);
	} else {
		BRAIN_CK_0(quas.size() == 1);
		BRAIN_CK_0(level() == ROOT_LEVEL);

		action_t ok = send_signal(quas[0], NULL);
		if(ok != ac_go_on){
			set_result(k_no_satisf);
		}
	}
}

void
brain::load_instance(long num_neu, long num_var, row_long_t& load_ccls){
	//solver& the_slv = slv();
	
	instance_info& inst_info = get_my_inst();
	inst_info.ist_num_vars = num_var;

	init_loading(num_var, num_neu);

	BRAIN_CK(load_ccls.last() == 0);
	BRAIN_CK(num_var > 0);

	br_neurons.set_cap(num_neu);

	long added_neus = 0;
	long added_dens = 0;

	long max_neu_sz = 0;

	long ii = 0;
	long first = 0;
	for(ii = 0; ii < load_ccls.size(); ii++){
		long nio_id = load_ccls[ii];
		if(nio_id == 0){
			added_neus++;
			long num_dens = ii - first;
			BRAIN_CK(num_dens > 0);
			added_dens += num_dens;

			if(num_dens > max_neu_sz){
				max_neu_sz = num_dens;
			}

			add_neuron_from_lits(load_ccls, first, ii);
			if(inst_info.ist_result != k_unknown_satisf){ 
				break; 
			}

			first = ii + 1;
		}
	}

	//BRAIN_CK(net.nt_neurons.size() == added_neus);

	inst_info.ist_num_ccls = added_neus;
	inst_info.ist_num_lits = added_dens;

	double end_load_tm = run_time();
	double ld_tm = (end_load_tm - br_start_load_tm);
	slv().batch_stat_load_tm.add_val(ld_tm);

	std::string f_nam = inst_info.get_f_nam();
	/*PRT_OUT(1,
	os << std::endl;
	os << "***********************************************";
	os << std::endl;
	os << "LOADED " << f_nam <<
		" " << slv().batch_consec << " of " << slv().batch_num_files <<
		std::endl;
	);*/
}

void
brain::check_timeout(){
	if(br_prt_timer.check_period(due_periodic_prt, this)){
		set_result(k_timeout);
	}
}

void
brain::set_assig(){
}

void
brain::check_sat_assig(){
	if(! brn_compute_binary()){
		std::cerr << "FATAL ERROR 001. Wrong is_sat answer !" << std::endl;
		abort_func(1);
	}

	//row<long>& the_chosen = slv().final_chosen_ids;
	row<quanton*>& the_assig = slv().final_assig;

	the_assig.clear();
	BRAIN_CK_0(	(br_trail.size() > 0) && 
			(br_trail[0] == &(quanton::ROOT_QUANTON)));
	br_trail.copy_to(the_assig, 1);	// skip ROOT_QUANTON

	if(! brn_compute_dots_of(the_assig)){
		std::cerr << "FATAL ERROR 002. Wrong is_sat answer !" << std::endl;
		abort_func(1);
	}

	//print_satifying(cho_nm);

}

void 
brain::solve_it(){
	instance_info& inst_info = get_my_inst();
	
	if(inst_info.ist_result != k_unknown_satisf){ 
		return;
	}

	if(slv().just_read){
		set_result(k_timeout);
		return;
	} 

	//BRAIN_CK(! slv().is_finishing());

	inst_info.ist_solve_time = run_time();

	std::string f_nam = inst_info.get_f_nam();

	config_brain(f_nam.c_str());
	br_choice_spin = cg_negative;
	br_choice_order = k_right_order;


	row<neuron*> conflicts;
	while(inst_info.ist_result == k_unknown_satisf){
		pulsate(conflicts);
	}

	//std::cout << "recoils= " << recoil() << std::endl; 
	BRAIN_CK(recoil() == (inst_info.ist_num_laps + 1));

	if(slv().op_ck_satisf && ! slv().doing_dbg() && (inst_info.ist_result == k_yes_satisf)){
		check_sat_assig();
	}

	DBG(
		DBG_PRT(32, os << "BRAIN=" << std::endl;
			print_brain(os); 
		);

		br_final_msg << f_nam << " ";

		satisf_val resp_solv = inst_info.ist_result;
		if(resp_solv == k_yes_satisf){
			check_sat_assig();
			br_final_msg << "IS_SAT_INSTANCE";
		} else if(resp_solv == k_no_satisf){
			br_final_msg << "IS_UNS_INSTANCE";

		} else if(resp_solv == k_timeout){
			br_final_msg << "HAS_TIMEOUT";
		}

		std::cout << br_final_msg.str() << std::endl; 
		std::cout.flush();

		//br_final_msg << " " << stats.num_start_syn;
		//br_final_msg << " " << solve_time();
		//std::cout << "recoils= " << recoil() << std::endl; 
		//std::cout << "num_batch_file= " << stats.batch_consec << std::endl; 
		//std::cout << std::endl;
		//std::cout << ".";

		if(dbginf().dbg_ic_active){
			dbg_ic_print();
		}
	);

	if(level() != ROOT_LEVEL){
		recoil_to_level(ROOT_LEVEL, br_reasons);
	}
}

bool
neuron::save_clause(row_long_t& save_ccls, long& num_lits){
	row_long_t the_lits;
	the_lits.clear();

	bool has_pos = false;
	MARK_USED(has_pos);
	for(long ii = 0; ii < size(); ii++){
		BRAIN_CK(ne_fibres[ii] != NULL);
		quanton& qua = *(ne_fibres[ii]);
		charge_t chg = qua.get_charge();
		if(chg == cg_positive){
			has_pos = true;
			the_lits.clear();
			break;
		}
		if(chg != cg_negative){
			the_lits.push(qua.qu_id);
		}
	}

	bool saved = false;
	if(! the_lits.is_empty()){
		num_lits += the_lits.size();

		the_lits.append_to(save_ccls);
		save_ccls.push(0);
		saved = true;
	}
	BRAIN_CK(saved || has_pos);
	return saved;
}

void
brain::save_instance(long& num_ccls, long& num_vars, long& num_lits, row_long_t& save_ccls){
	save_ccls.clear();
	num_ccls = 0;
	num_vars = 0;
	num_lits = 0;

	instance_info& inst_info = get_my_inst();
	if(inst_info.ist_result != k_unknown_satisf){
		return;
	}

	for(long ii = 0; ii < br_neurons.size(); ii++){
		if(br_neurons[ii].save_clause(save_ccls, num_lits)){
			num_ccls++;
		}
	}
	num_vars = br_positons.size();
}

void 
brain::simplify_it(long& num_ccls, long& num_vars, long& num_lits, row_long_t& after_ccls){
	instance_info& inst_info = get_my_inst();
	
	if(inst_info.ist_result != k_unknown_satisf){ 
		return;
	}

	if(slv().just_read){
		set_result(k_timeout);
		return;
	} 

	//BRAIN_CK(! slv().is_finishing());

	inst_info.ist_solve_time = run_time();

	std::string f_nam = inst_info.get_f_nam();

	config_brain(f_nam.c_str());
	br_choice_spin = cg_negative;
	br_choice_order = k_right_order;

	BRAIN_CK(inst_info.ist_result == k_unknown_satisf);
	BRAIN_CK(level() == ROOT_LEVEL);

	row<neuron*> conflicts;
	brn_tunnel_signals(conflicts);

	if(conflicts.size() > 0){
		BRAIN_CK(level() == ROOT_LEVEL);
		set_result(k_no_satisf);
	} else {
		quanton* qua = NULL;
		qua = choose_quanton();
		if(qua == NULL){
			set_result(k_yes_satisf);
		}
	}
	save_instance(num_ccls, num_vars, num_lits, after_ccls);
	if((inst_info.ist_result == k_unknown_satisf) && (num_ccls == 0)){
		BRAIN_CK(after_ccls.is_empty());
		set_result(k_yes_satisf);
	}
}

