

/*************************************************************

bible_sat

dbg_ic.cpp  
(C 2025) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com

funcs for inference graph printing

--------------------------------------------------------------*/

//include <iostream>
#include <fstream>
#include <sstream>

#include <cassert>
#include <cstdlib>
#include <cstring>

#include "support.h"
#include "brain.h"

#define DBG_IC_RANKS		1
#define DBG_IC_SUBGRAPHS	2

#define DBG_PREF_IC "ic_"
#define DBG_PREF_OLD_IC "old_ic_"
//define DBG_SUFI_TLP ".tlp"
#define DBG_SUFI_DOT ".dot"
#define DBG_PREF_NEU "neus_"
#define DBG_SUFI_NEU ".myl"

// IC GRAPH PRT_FUNCS

void
brain::dbg_ic_print(){
	if((GLB.dbg_ic_seq >= GLB.dbg_ic_max_seq) && (GLB.dbg_ic_max_seq >= 0)){
		return;
	}
	DBG_PRT(30, os << "**IC_PRT**");
	DBG_PRT(31, os << "THE BRAIN=" << std::endl;
		print_brain(os);
		os << "Type ENTER to print the IC: " << std::endl;
		DO_GETCHAR;
	);

	std::ofstream ff;
	GLB.dbg_ic_seq++;

	std::string f_suf = DBG_SUFI_DOT;
	std::string f_nam = dbg_name(DBG_PREF_IC, GLB.dbg_ic_seq, f_suf);

	ff.open(f_nam.c_str());
	dbg_ic_prt_dotty_file(ff, DBG_IC_SUBGRAPHS);
	ff.close();

	if(GLB.dbg_ic_gen_jpg){
		//long ancho = 3;
		//char ch_cero = '0';

		std::ostringstream o_str;
		o_str << "echo dot -Tjpg -o ";

		std::string jpg_suf = ".jpg";
		std::string nm_jpg = dbg_name(DBG_PREF_IC, GLB.dbg_ic_seq, jpg_suf);

		o_str << nm_jpg << " " << f_nam;

		/*
		o_str << DBG_DIR << DBG_PREF_IC;
		o_str.width(ancho);
		o_str.fill(ch_cero);
		o_str << GLB.dbg_ic_seq << ".jpg ";

		o_str << DBG_DIR << DBG_PREF_IC;
		o_str.width(ancho);
		o_str.fill(ch_cero);
		o_str << GLB.dbg_ic_seq << f_suf;
		*/

		o_str << " >> " << DBG_DIR << "all_dot_to_jpg.bat";

		system_exec(o_str);
	}

	if(recoil() == 1){
		f_suf = DBG_SUFI_NEU;
		f_nam = dbg_name(DBG_PREF_NEU, 0, f_suf);

		ff.open(f_nam.c_str());
		br_neurons.print_row_data(ff, false, "\n", -1, -1, true);
		ff.close();
	}
}

std::ostream& 	
brain::dbg_ic_prt_dotty_label(std::ostream& os){
	//stats.print_info(os, this, 0.0, true, true);
	os << "\\n \\" << std::endl;
	//os << "i#n " << stats.num_start_neu << " ";
	//os << "#n " << stats.num_neurons << " ";
	os << "\\n \\" << std::endl;
	//os << br_final_msg << " "; // JLQ_2025_05_13
	os << br_final_msg.str() << " ";
	os << "recoil " << recoil() << "  ";	
	os << "excited_level " << br_excited_level << "  ";
	os << "\\n \\" << std::endl;
	os << "Quantons: ";
	print_all_quantons(os, 10, "\\n\\\n");
	os << "\\n \\" << std::endl;

	os << "Reasons learned: ";
	os << "\\n \\" << std::endl;
	br_reasons.print_row_data(os, true, "\\n\\\n");
	return os;
}

std::ostream& 	
brain::dbg_ic_prt_dotty_file(std::ostream& os, long style){
	long ii;
	row<quanton*> nodes;

	os << "digraph \"ic_graph\" {" << std::endl;
	//os << "node [style=filled,fontsize=20];" << std::endl;
	os << "node [fontsize=20];" << std::endl;
	os << "edge [style=bold];" << std::endl;
	os << "graph [fontsize=20];" << std::endl;
	os << "compound=true" << std::endl;

	os << "subgraph cluster0 {" << std::endl;
	os << "\"" << br_file_name_in_ic << "\" [shape=box];" << std::endl;
	os << std::endl << std::endl;
	os << "label = \"";
	dbg_ic_prt_dotty_label(os);
	os << "\";" << std::endl;
	os << "}" << std::endl << std::endl;

	for(ii = 1; ii < br_trail.size(); ii++){
		quanton* qua = br_trail[ii];
		nodes.push(qua);

		DBG_SOURCES(
			quanton* inv = qua->qu_inverse;
			if(inv->qu_dbg_ic_all_sources.size() > 0){
				nodes.push(inv);
			}
		)
	}
	os << std::endl << std::endl;

	long clus = 1;
	long q_lv_old = -1;
	long q_lv = q_lv_old;
	long lev = level();

	long edg_id = 0;
	MARK_USED(edg_id);
	for(ii = 0; ii < nodes.size(); ii++){
		quanton* qua1 = nodes[ii];
		long num_srcs = 0;
		MARK_USED(num_srcs);
		DBG_SOURCES(num_srcs = qua1->qu_dbg_ic_all_sources.size();)

		q_lv_old = q_lv;
		q_lv = qua1->qlevel();

		if((style == DBG_IC_SUBGRAPHS) && (q_lv_old != q_lv)){
			if(q_lv_old != -1){
				os << "} " << std::endl;
			}
			os << "subgraph cluster" << clus << " { " << std::endl;
			clus++;
		}

		quanton* qua = qua1;
		qua->print_ic_label(os);
		assert(qua->qlevel() <= lev);

		long num_sources = 0;
		DBG_SOURCES(num_sources = qua->qu_dbg_ic_all_sources.size();)

		if(qua->qlevel() == lev){ 
			os << " [";
			if(num_sources == 0){
				os << "shape=\"diamond\",";
			} else {
				os << "shape=\"box\",";
			}
			os << "fontcolor=\"blue\"";
			os << "]";
		} else if(num_sources == 0){
			os << " [shape=\"diamond\"]";
		} 
		os << ";" << std::endl;

		if(qua1->qu_charge == cg_negative){ 
			quanton* inv = qua1->qu_inverse;

			inv->print_ic_label(os);
			os << " -> ";
			qua1->print_ic_label(os);
			os << " [color=\"red\"]";
			os << ";" << std::endl;
		}

		long num_srcs1 = 0;
		DBG_SOURCES(num_srcs1 = qua1->qu_dbg_ic_all_sources.size();)

		for(long jj = 0; jj < num_srcs1; jj++){
			neuron* neu = NULL;
			DBG_SOURCES(neu = qua1->qu_dbg_ic_all_sources[jj];)
			assert(neu != NULL);
			for(long kk = 0; kk < neu->size(); kk++){
				quanton* qua2 = (neu->ne_fibres)[kk];
				long num_srcs2 = 0;
				DBG_SOURCES(num_srcs2 = qua2->qu_dbg_ic_all_sources.size();)
				if(	(qua2->qu_charge == cg_negative) && 
					(	(qua1->qu_charge == cg_positive) ||
						(num_srcs2 == 0)	
					)	
				){ 
					qua2 = qua2->qu_inverse;
				}

				if(qua1 == qua2){ continue; }

				qua2->print_ic_label(os);
				os << " -> ";
				qua1->print_ic_label(os);

				//char* mark = "";
				std::string mark = "";

				os << "[label=\"" << mark;
				os << neu->ne_index;
				os << mark << "\"]";
				os << ";" << std::endl;
			}
		}
	}
	if((style == DBG_IC_SUBGRAPHS) && (q_lv_old != -1)){
		os << "} " << std::endl;
	}

	os << "}" << std::endl;

	return os;
}


