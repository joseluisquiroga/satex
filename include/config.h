

/*************************************************************

yoshu

config.h
(C 2010) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

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

std::ostringstream& parse_err_msg(const char* hd_msg, long num_line, char ch_err, 
		const char* msg);

void skip_whitespace(const char*& pt_in, long& line);
void skip_line(const char*& pt_in, long& line);
integer parse_int(const char*& pt_in, long line);


//=================================================================
// config_reader

class config_reader {
	public:

	config_reader(){
	}

	~config_reader(){}

	void 	parse_debug_line(row<long>& dbg_line, std::string& str_ln);
	void	add_config_line(std::string& str_ln);
	void	read_config(const char* f_name);
};



#endif		// CONFIG_H

