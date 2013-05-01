#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>

/* inline is not valid in C ANSI */
#define INLINE
/*
This works with C 99
#define INLINE inline
*/

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
