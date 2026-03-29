

/*************************************************************

satex

print_stm.h
(C 2025) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com


Some macros that simplify to printing handling

--------------------------------------------------------------*/


#ifndef PRINT_STM_H
#define PRINT_STM_H

#include "mem.h"

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

#define DEFINE_PRINT_FUNCS_WITH(obj_t, prt_func) \
inline \
std::ostream& operator << (std::ostream& os, obj_t& obj1){ \
	obj1.prt_func(os); \
	os.flush(); \
	return os; \
} \
\
inline \
std::ostream& operator << (std::ostream& os, obj_t* obj1){ \
	if(obj1 == NULL_PT){ \
		os << "NULL_" << #obj_t; \
	} else { \
		obj1->prt_func(os, true); \
	} \
	os.flush(); \
	return os; \
} \
\

// end_of_define


#endif // PRINT_STM_H


