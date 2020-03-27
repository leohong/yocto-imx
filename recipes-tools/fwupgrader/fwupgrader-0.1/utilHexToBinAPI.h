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

#if 0
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

typedef struct {
    // BYTE cColon;
    BYTE cLength;
    DWORD dwAddress;
    BYTE cRecType;
    BYTE acData[128];
} sINTELHEX;

typedef enum {
    eH2B_STATE_NEXT_LINE,
    eH2B_STATE_END_OF_FILE,
    eH2B_STATE_DATA_ERROR,

    eH2B_STATE_NUMBERS,
} eH2B_STATE;

eH2B_STATE utilIntelHexConverter(WORD wNumData, sINTELHEX *psHexData,
                                 BYTE *pcInPut);
#endif  // 0
#include "Driver/Bus/types.h"

struct node {
    int address;
    int size;
    // int pageSize;
    struct node *next;
    char *pcBuffer;
};

typedef struct node sHexList;


BOOL utilIntel_Hex_Parser(const char *fileName, sHexList *psHeader, int pageSize);
void utilIntel_Hex_List_Free(sHexList *psHeader);
BOOL utilIntel_Hex_List_Find(sHexList *psHeader, int address, sHexList *psNode);
#endif /* UTILHEXTOBINAPI_H.h */
