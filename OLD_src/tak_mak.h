

/*************************************************************

yoshu

tak_mak.h
A C-program for MT19937, with initialization improved 2002/1/26.
Coded by Takuji Nishimura and Makoto Matsumoto.

Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
All rights reserved.                          

Adapted to c++ by 
(C 2010) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com

Classes and that implement the pseudo random number generator of 
Makoto and Takuji.

class for the tak_mak pseudo random number generator.

	tak_mak use:

	if(key_longs.size() == 1){
		for_bytes.init_with_long(key_longs[0]);
	} else {
		for_bytes.init_with_array(
			key_longs.get_c_array(), 
			key_longs.get_c_array_sz()
		);
	}

	op = tm_gen.gen_rand_int32();
	idx1 = tm_gen.gen_rand_int32_ie(0, key_bits.size());
	idx2 = tm_gen.gen_rand_int32_ie(0, op_bits.size());


--------------------------------------------------------------*/


#ifndef TAK_MAK_H
#define TAK_MAK_H

#include <cstdio>

#define TAK_MAK_INIT_LONG_1		 19650218UL
#define TAK_MAK_INIT_LONG_2		 5489UL

/* Period parameters */  
#define TAK_MAK_N 624
#define TAK_MAK_M 397
#define TAK_MAK_MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define TAK_MAK_UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define TAK_MAK_LOWER_MASK 0x7fffffffUL /* least significant r bits */

inline
long	to_interval(long val, long min, long max){
	long diff = 0, resp = 0;
	diff = max - min;
	if(diff <= 0){ 
		return min; 
	}
	long rr = (val % diff);
	if(rr < 0){ rr = -rr; }
	resp = min + rr;
	return resp;
}

class tak_mak {
private:
	unsigned long mt[TAK_MAK_N]; /* the array for the state vector  */
	int mti; /* mti==TAK_MAK_N+1 means mt[TAK_MAK_N] is not initialized */

public:
	tak_mak(unsigned long ini_val = 0){
		init_tak_mak();
		if(ini_val == 0){
			ini_val = TAK_MAK_INIT_LONG_1;
		}
		init_with_long(ini_val);
	}

	tak_mak(const unsigned long* init_key, int key_length){
		init_tak_mak();
		init_with_array(init_key, key_length);
	}

	void init_tak_mak(){
		mti=TAK_MAK_N+1;
	}

	void init_with_long(unsigned long s);
	void init_with_array(const unsigned long* init_key, int key_length);

	unsigned long gen_rand_int32(void);

	double gen_rand_i0_i1(void);
	double gen_rand_i0_e1(void);
	double gen_rand_e0_e1(void);

	long gen_rand_int32_ie(long min, long max){
		return to_interval(gen_rand_int32(), min, max);
	}

};


#endif	// TAK_MAK_H


