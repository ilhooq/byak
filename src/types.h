/**
* Byak, a UCI chess engine.
* Copyright (C) 2013  Sylvain Philip
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>

#ifndef DEBUG
	#define USE_INLINING
	/* Deactivate assertions */
	#define NDEBUG
#endif

/* inline is not valid in C ANSI */
#ifdef USE_INLINING
	#ifdef _MSC_VER
		#define INLINE __forceinline
	#elif defined(__GNUC__)
		#define INLINE __inline__ __attribute__((always_inline))
	#else
		#define INLINE inline
	#endif
#else
	#define INLINE
#endif


typedef uint64_t U64;
typedef int64_t  S64;
typedef uint16_t  U16;
typedef uint8_t  U8;


#define __64_BIT_INTEGER_DEFINED__

#define C64(constantU64) __UINT64_C(constantU64)
#define ULL(integer) ((unsigned long long int) integer)

typedef enum enumProtocol {
	DEFAULT,
	UCI,
	XBOARD
} Protocol;

extern Protocol protocol;

#endif
