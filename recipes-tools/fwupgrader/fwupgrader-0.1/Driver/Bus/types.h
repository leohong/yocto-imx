
#ifndef _TYPES_H_
#define _TYPES_H_

#include <wchar.h>

#define TRACEIF(...)
#define ASSERT(...)

typedef unsigned char		BOOL;
typedef unsigned char		BYTE;
typedef signed char		SBYTE;
typedef unsigned char		UBYTE;
typedef signed short		SWORD16;
typedef unsigned short		UWORD16;
typedef signed long		SWORD32;
typedef unsigned long		UWORD32;
typedef signed long long	SWORD64;
typedef unsigned long long	UWORD64;
typedef float				SFLOAT;
typedef double			DFLOAT;

typedef signed char		CHAR;
typedef unsigned short		WORD;
typedef unsigned long		DWORD;

typedef char * string;
typedef wchar_t * wstring;

#define TRUE 1
#define FALSE 0
//#define BOOL unsigned long

#define MAX8 0xFF
#define MAX15 0x7FFF
#define MAX16 0xFFFF
#define MAX32 0xFFFFFFFF

#define BIT0		0x01
#define BIT1		0x02
#define BIT2		0x04
#define BIT3		0x08
#define BIT4		0x10
#define BIT5		0x20
#define BIT6		0x40
#define BIT7		0x80
#define BIT8		0x0100
#define BIT9		0x0200
#define BIT10		0x0400
#define BIT11		0x0800
#define BIT12		0x1000
#define BIT13		0x2000
#define BIT14		0x4000
#define BIT15		0x8000
#define BIT16		0x010000
#define BIT17		0x020000
#define BIT18		0x040000
#define BIT19		0x080000
#define BIT20		0x100000
#define BIT21		0x200000
#define BIT22		0x400000
#define BIT23		0x800000
#define BIT24		0x01000000
#define BIT25		0x02000000
#define BIT26		0x04000000
#define BIT27		0x08000000
#define BIT28		0x10000000
#define BIT29		0x20000000
#define BIT30		0x40000000
#define BIT31		0x80000000

#endif // _TYPES_H_

