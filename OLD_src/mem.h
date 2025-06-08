

/*************************************************************

satex

mem.h
(C 2025) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com

Declaration of mem trace funcs and other.

--------------------------------------------------------------*/

#ifndef JLQ_MEM_H
#define JLQ_MEM_H

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <execinfo.h>
#include <cxxabi.h>

#include "platform.h"

#define STACK_STR get_stack_trace(__FILE__, __LINE__)

bool
print_backtrace( const std::string & file, int line );

std::string
demangle_cxx_name( const std::string &stack_string );

std::string 
get_stack_trace( const std::string & file, int line );

inline
bool 
call_assert(bool vv_ck, const std::string & file, int line, std::string ck_str){
	if(! vv_ck){
		std::cout << "ASSERT '" << ck_str << "' FAILED" << std::endl;
		std::cout << get_stack_trace(file, line) << std::endl;
	}
	assert(vv_ck);
	return vv_ck;
};

#define glb_assert(vv) call_assert(vv, __FILE__, __LINE__, #vv)

#define NULL_PT		NULL

#ifdef FULL_DEBUG
#define DBG(prm) prm
#else
#define DBG(prm) /**/ \

// end_of_def
#endif

#define DBG_CK(prm)	   	DBG(glb_assert(prm))
#define DBG_THROW(prm) 		DBG(prm)
//define DBG_THROW(prm) 		;

#ifdef NO_MEM_CTRL
#define MEM_CTRL(prm) ;
#else
#define MEM_CTRL(prm) prm
#endif

#ifdef SECURE_MEM
#define MEM_SRTY(prm) prm
#else
#define MEM_SRTY(prm) ;
#endif

#define MEM_CK(prm)		DBG_CK(prm)

typedef long			error_code_t;
typedef unsigned long	mem_size;
typedef char			t_1byte;
typedef unsigned int	t_4byte;
typedef t_4byte			t_dword;
typedef std::string		t_string;
typedef std::ostream 	t_ostream;
typedef std::ostringstream 	t_ostr_stream;
typedef std::ofstream 	t_ofstream;

#define t_dbg_os std::cerr

#define MAX_UTYPE(type)		((type)(-1))

#define MAX_MEM_SZ		MAX_UTYPE(mem_size)

#define as_pt_char(the_str) (const_cast<char *>(the_str))

void abort_func(long val, const char* msg = as_pt_char("Aborting."));

//=================================================================
// top_exception

typedef enum {
	prx_bad_cicle_1,
} pr_ex_cod_t;


class top_exception {
private:
	top_exception&  operator = (top_exception& other){
		abort_func(0);
		return (*this);
	}
	
public:
	long		ex_id;
	t_string 	ex_stk;
	t_string 	ex_assrt;
	
	top_exception(long the_id = 0, t_string assrt_str = ""){
		ex_id = the_id;
		ex_stk = STACK_STR;
		ex_assrt = assrt_str;
	}
	
	~top_exception(){
	}
	
	virtual t_string name(){ t_string nm = "top_exception"; return nm; }
	
	virtual void prt_ex(std::ostream& os){
		os << name() << "\n";
		os << "id:" << ex_id << "\n";
		os << "stack:\n" << ex_stk << "\n";
		os << "assert:\n" << ex_assrt << "\n";
		DBG(abort_func(0);)
	}
};

//======================================================================
// memory_exception

typedef enum {
	mex_memout,
	mex_memory_exhausted,
	mex_memory_fully_exhausted,
	mex_memory_exhausted_during_realloc,
	mex_memory_fully_exhausted_during_realloc,
	mex_memory_fully_exhausted_during_secure_realloc, 
} me_ex_cod_t;

class memory_exception : public top_exception {
public:
	
	memory_exception(long the_id = 0) : top_exception(the_id)
	{
	}

	virtual t_string name(){ t_string nm = "memory_exception"; return nm; }
};

//======================================================================
// glb_mem_data

class glb_mem_data;
extern glb_mem_data MEM_STATS;

//define DEFINE_MEM_STATS	glb_mem_data MEM_STATS

class glb_mem_data {
public:
	typedef void (*memout_func_t)();

	mem_size 		num_bytes_in_use;
	mem_size 		num_bytes_available;
	memout_func_t	set_memout_func;
	bool			use_secure_alloc;

	glb_mem_data(){
		num_bytes_in_use = 0;
		num_bytes_available = 0;
		set_memout_func = NULL_PT;
		use_secure_alloc = false;
	}

	~glb_mem_data(){
		MEM_CK(num_bytes_in_use == 0);
	}
};


//======================================================================
// 'malloc()'-style memory allocation -- never returns NULL_PT; aborts instead:

template<class obj_t> static inline obj_t* 
tpl_malloc(size_t the_size = 1){
	mem_size mem_sz = the_size * sizeof(obj_t);
	MEM_CTRL(
		MEM_CK((MAX_MEM_SZ - mem_sz) > MEM_STATS.num_bytes_in_use);
		MEM_STATS.num_bytes_in_use += mem_sz;

		if(	(MEM_STATS.num_bytes_available > 0) && 
			(MEM_STATS.num_bytes_in_use > MEM_STATS.num_bytes_available) )
		{
			if(MEM_STATS.set_memout_func != NULL_PT){
				(*MEM_STATS.set_memout_func)();
			} else {
				throw memory_exception(mex_memory_exhausted);
			}
		}
	);

	obj_t*   tmp = (obj_t*)malloc(mem_sz);
	if((tmp == NULL_PT) && (the_size != 0)){
		throw memory_exception(mex_memory_fully_exhausted);
	}
	return tmp; 
}

template<class obj_t> static inline obj_t* 
tpl_secure_realloc(obj_t* ptr, size_t old_size, size_t the_size){
	MEM_CK(the_size > old_size);

	mem_size mem_sz = the_size * sizeof(obj_t);
	obj_t*   tmp = (obj_t*)malloc(mem_sz);
	if((tmp == NULL_PT) && (the_size != 0)){
		throw memory_exception(mex_memory_fully_exhausted_during_secure_realloc);
	}

	if(ptr != NULL_PT){
		mem_size old_mem_sz = old_size * sizeof(obj_t);
		memcpy(tmp, ptr, old_mem_sz);
		memset(ptr, 0, old_mem_sz);
		free(ptr);
	}
	return tmp; 
}

template<class obj_t> static inline obj_t* 
tpl_realloc(obj_t* ptr, size_t old_size, size_t the_size){
	mem_size mem_sz = the_size * sizeof(obj_t);
	MEM_CTRL(
		mem_size old_mem_sz = old_size * sizeof(obj_t);
		MEM_CK(MEM_STATS.num_bytes_in_use >= old_mem_sz);
		MEM_STATS.num_bytes_in_use -= old_mem_sz;
		MEM_CK((MAX_MEM_SZ - mem_sz) > MEM_STATS.num_bytes_in_use);
		MEM_STATS.num_bytes_in_use += mem_sz;

		if(	(MEM_STATS.num_bytes_available > 0) && 
			(MEM_STATS.num_bytes_in_use > MEM_STATS.num_bytes_available) )
		{
			if(MEM_STATS.set_memout_func != NULL_PT){
				(*MEM_STATS.set_memout_func)();
			} else {
				throw memory_exception(mex_memory_exhausted_during_realloc);
			}
		}
	);
	MEM_SRTY(
		if(MEM_STATS.use_secure_alloc){
			return tpl_secure_realloc(ptr, old_size, the_size); 
		}
	);
	obj_t*   tmp = (obj_t*)realloc((void*)ptr, mem_sz);
	if((tmp == NULL_PT) && (the_size != 0)){
		throw memory_exception(mex_memory_fully_exhausted_during_realloc);
	}
	return tmp; 
}

template<class obj_t> static inline void 
tpl_free(obj_t*& ptr, size_t the_size = 1){
	if(ptr != NULL_PT){ 
		MEM_SRTY(
			mem_size s_old_mem_sz = the_size * sizeof(obj_t);
			if(MEM_STATS.use_secure_alloc){
				memset(ptr, 0, s_old_mem_sz);
			}
		);
		free(ptr); 
		ptr = NULL_PT;
	}
	MEM_CTRL(
		mem_size old_mem_sz = the_size * sizeof(obj_t);
		MEM_CK(MEM_STATS.num_bytes_in_use >= old_mem_sz);
		MEM_STATS.num_bytes_in_use -= old_mem_sz;
	);
}




#endif // JLQ_MEM_H


