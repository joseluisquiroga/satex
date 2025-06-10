

/*************************************************************

satex

test.cpp  
(C 2010) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com

Code for testing purposes only.

--------------------------------------------------------------*/


#define _XOPEN_SOURCE 500
#include <ftw.h>

#include <cstring>
#include <set>
#include <map>
#include <iterator>

#include <iostream>
#include <gmpxx.h>

//     g++ t1.cpp -lgmpxx -lgmp -o tt.exe

#include "unistd.h"
#include "limits.h"

#include "solver.h"

//include "brain.h"
#include "tak_mak.h"

// 		init_random_nums();
//		long num_red = gen_random_num(0, rr2.size());
//		os << CARRIAGE_RETURN << aa;
// CAP_ATTRIB	DATA_ATTRIB

void	test_big_num(){
	mpz_class a, b, c, d;

	a = 0;
	mpz_setbit(a.get_mpz_t(), 6);
	mpz_setbit(a.get_mpz_t(), 12);
	mpz_setbit(a.get_mpz_t(), 17);
	//mpz_setbit(a.get_mpz_t(), 112);
	std::cout << "a is " << a << "\n";

	std::string s1 = a.get_str(2);
	std::cout << "s1 is " << s1 << std::endl;

	//long cm1 = mpz_cmp(nu1.get_mpz_t(), nu2.get_mpz_t());

}

int	tests_main_(int argc, char** argv){
	MARK_USED(argc);
	MARK_USED(argv);
	std::ostream& os = std::cout;

	MEM_CTRL(mem_size tt_mem_in_u = MEM_STATS.num_bytes_in_use;)
	MEM_CTRL(MARK_USED(tt_mem_in_u));

	//test_ini_map();

	os << "ENDING TESTS___________" << std::endl;
	os.flush();

	SUPPORT_CK_0(tt_mem_in_u == MEM_STATS.num_bytes_in_use);

	//MEM_CTRL(mem_size mem_u = MEM_STATS.num_bytes_in_use;)  // not nedded. tested during
	//SUPPORT_CK(mem_u == MEM_STATS.num_bytes_in_use); // mem.cpp MEM_STATS destruction

	os << std::endl;
	os << "End of tests" << std::endl;
	//getchar();

	return 0;
}


