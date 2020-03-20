#ifndef UTILHEXTOBINAPI_H
#define UTILHEXTOBINAPI_H
// ===============================================================================
// FILE NAME: utilHexToBinAPI.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2014/06/14, Leo Create
// --------------------
// ===============================================================================


#include "Driver/Bus/types.h"

/*
struct segmentNode {
    DWORD       dwAddress;
    WORD        wTotalPages;
    BYTE        acBuffer[0xFFFF];
    segmentNode *next;
};

typedef segmentNode sSegmentNode;
*/

typedef struct
{
    //BYTE cColon;
    BYTE 	cLength;
    DWORD 	dwAddress;
    BYTE 	cRecType;
    BYTE 	acData[33];
} sINTELHEX;

typedef enum
{
    eH2B_STATE_NEXT_LINE,
    eH2B_STATE_END_OF_FILE,
    eH2B_STATE_DATA_ERROR,

    eH2B_STATE_NUMBERS,
} eH2B_STATE;

eH2B_STATE utilIntelHexConverter(WORD wNumData, sINTELHEX *psHexData, BYTE *pcInPut);
BOOL utilIntel_Hex_Parser(const char *fileName);

#endif /* UTILHEXTOBINAPI_H.h */

