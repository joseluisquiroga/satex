

/*************************************************************

yoshu

platform.h
(C 2010) QUIROGA BELTRAN, Jose Luis. Bogotá - Colombia.

Date of birth: December 28 of 1970.
Place of birth: Bogota - Colombia - Southamerica.
Id (cedula): 79523732 de Bogota.
email: JoseLuisQuiroga@yahoo.com


Some macros that allow to indent better platform 
dependant code.

Insted of

#ifdef MY_PLATFORM
						<some_code>
#endif

that can be messy when code has several platforms 
dependant code. Specially inside functions.

Write:
		
					MY_PLAT_COD(
						<some_code>
					);

to make the code more clear.

--------------------------------------------------------------*/


#ifndef PLATFORM_H
#define PLATFORM_H

#ifndef MARK_USED
#define MARK_USED(X)  ((void)(&(X)))
#endif // MARK_USED

#ifdef WIN32
#define WIN32_COD(prm)	prm
#else
#define WIN32_COD(prm)	/**/
#endif


#ifdef __linux
#define LINUX_COD(prm)	prm
#else
#define LINUX_COD(prm)	/**/
#endif


#endif // PLATFORM_H


