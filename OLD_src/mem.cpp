

/*************************************************************

satex

mem.cpp  
(C 2025) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com

mem trace funcs and other.

--------------------------------------------------------------*/

#include "mem.h"

glb_mem_data MEM_STATS;

void abort_func(long val, const char* msg){
	std::cerr << "\nABORTING! " << msg << "\n"; 
	exit(val);
}

bool
print_backtrace( const std::string & file, int line ){
	std::cerr << get_stack_trace(file, line) << std::endl;
	return true;
}

std::string 
demangle_cxx_name( const std::string &stack_string ){
	const size_t k_max_nm_len = 4096;
	char function[k_max_nm_len];
	size_t start = stack_string.find( '(' );
	size_t end = stack_string.find( '+' );

	if( std::string::npos == start || std::string::npos == end ){
		return stack_string;
	}

	int status;

	++start; // puts us pass the '('

	size_t n = end - start;
	size_t len;
	std::string mangled = stack_string.substr( start, n );
	char *ret = abi::__cxa_demangle( mangled.c_str(), function, &len, &status );
	MARK_USED(ret);

	if( 0 == status ){ // demanging successful
		return std::string( function );
	}

	// If demangling fails, returned mangled name with some parens
	return mangled + "()";
}

// USE:        std::cout << get_stack_trace( __FILE__, __LINE__ ) << std::endl;

std::string 
get_stack_trace( const std::string & file, int line ){
	std::stringstream result;
	result << "Call Stack from " << file << ":" << line << "\n";
	const size_t k_max_depth = 100;
	void *stack_addrs[k_max_depth];
	size_t stack_depth;
	char **stack_strings;

	stack_depth = backtrace( stack_addrs, k_max_depth );
	stack_strings = backtrace_symbols( stack_addrs, stack_depth );
	for( size_t i = 1; i < stack_depth; ++i ) {
		//result << "   " << demangle_cxx_name( stack_strings[i] ) << "\n";
		result << "   " << stack_strings[i] << "\n";
	}
	std::free( stack_strings );

	return result.str();
}





