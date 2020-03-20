// ===============================================================================
// FILE NAME: utilHexToBin.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2014/06/14, Leo Create
// --------------------
// ===============================================================================

#include <stdio.h>
#include "utilHexToBinAPI.h"

// ==============================================================================
// FUNCTION NAME: LOW2UPCASE
// DESCRIPTION:
//
//
// Params:
// BYTE *pcData:
//
// Returns:
//
//
// modification history
// --------------------
// 29/05/2011, Leohong written
// --------------------
// ==============================================================================
static void utilLow2UpCase(BYTE *pcData)        // A70LH_Jonas_0013, Re-declared as static function
{
    if((*pcData >= 'a') && (*pcData <= 'z'))
    {
        *pcData = *pcData - 'a' + 'A';
    }
}

// ==============================================================================
// FUNCTION NAME: HEX2BIN
// DESCRIPTION:
//
//
// Params:
// BYTE *pcData:
//
// Returns:
//
//
// modification history
// --------------------
// 29/05/2011, Leohong written
// --------------------
// ==============================================================================
static eRESULT utilHex2Bin(BYTE *pcData)        // A70LH_Jonas_0013, Re-declared as static function
{
    utilLow2UpCase(pcData);

    if((*pcData >= '0') && (*pcData <= '9'))
    {
        *pcData = *pcData - '0';
    }
    else if((*pcData >= 'A') && (*pcData <= 'F'))
    {
        *pcData = *pcData - 'A' + 10;
    }
    else
    {
        return rcERROR;
    }

    return rcSUCCESS;
}

// ==============================================================================
// FUNCTION NAME: ASCIITOHEX
// DESCRIPTION:
//
//
// Params:
// BYTE cHighByte:
// BYTE cLowByte:
// BYTE *pcData:
//
// Returns:
//
//
// modification history
// --------------------
// 29/05/2011, Leohong written
// --------------------
// ==============================================================================
static eRESULT utilAsciiToHex(BYTE cHighByte, BYTE cLowByte, BYTE *pcData)  // A70LH_Jonas_0013, Re-declared as static function
{
    eRESULT eResult = rcERROR;

    if((utilHex2Bin(&cHighByte) == rcSUCCESS) && (utilHex2Bin(&cLowByte) == rcSUCCESS))
    {
        *pcData = (cHighByte << 4) | cLowByte;
        eResult = rcSUCCESS;
    }

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: utilIntelHexConverter
// DESCRIPTION:
//
// Intel Hex Format
// :10246200464C5549442050524F46494C4500464C33
// |||||||||||                              CC->Checksum
// |||||||||DD->Data
// |||||||TT->Record Type
// |||AAAA->Address
// |LL->Record Length
// :->Colon
//
// Checksum for example:
// :0300300002337A1E
// 03 + 00 + 30 + 00 + 02 + 33 + 7A = E2, 2's complement is 1E
//
// Params:
// sINTELHEX *psHexData:
// BYTE *pcInPut:
//
// Returns:
//
//
// modification history
// --------------------
// 12/05/2011, Leohong written
// --------------------
// ==============================================================================
eH2B_STATE utilIntelHexConverter(WORD wNumData, sINTELHEX *psHexData, BYTE *pcInPut)
{
    eRESULT eResult = rcSUCCESS;
    BYTE cTmpData = 0;
    BYTE cCount = 0;
    BYTE cCheckSum = 0;

    //Search ':'
    while(wNumData--)
    {
        if((BYTE)*pcInPut == ':')
        {
            //Get Colon
            pcInPut++;
            break;
        }

        //Parsing next data
        pcInPut++;
    }

    if(0 == wNumData)
    {
        return eH2B_STATE_DATA_ERROR;
    }

    //Get data length
    if(utilAsciiToHex(pcInPut[0], pcInPut[1], &cTmpData) != rcSUCCESS)
    {
        return eH2B_STATE_DATA_ERROR;
    }

    psHexData->cLength = cTmpData;
    cCheckSum += cTmpData;
    pcInPut += 2;

    //Get Address High Byte
    if(utilAsciiToHex(pcInPut[0], pcInPut[1], &cTmpData) != rcSUCCESS)
    {
        return eH2B_STATE_DATA_ERROR;
    }

    psHexData->dwAddress = (psHexData->dwAddress & 0xFF00) | cTmpData << 8;
    cCheckSum += cTmpData;
    pcInPut += 2;

    //Get Address Low Byte
    if(utilAsciiToHex(pcInPut[0], pcInPut[1], &cTmpData) != rcSUCCESS)
    {
        return eH2B_STATE_DATA_ERROR;
    }

    psHexData->dwAddress |= cTmpData;
    cCheckSum += cTmpData;
    pcInPut += 2;

    //Get Data Type
    if(utilAsciiToHex(pcInPut[0], pcInPut[1], &cTmpData) != rcSUCCESS)
    {
        return eH2B_STATE_DATA_ERROR;
    }

    psHexData->cRecType = cTmpData;
    cCheckSum += cTmpData;
    pcInPut += 2;

    for(cCount = 0; cCount < psHexData->cLength; cCount++)
    {
        eResult = utilAsciiToHex(pcInPut[0], pcInPut[1], &cTmpData);

        if(eResult != rcSUCCESS)
        {
            return eH2B_STATE_DATA_ERROR;
        }

        psHexData->acData[cCount] = cTmpData;
        cCheckSum += cTmpData;
        pcInPut += 2;
    }

    //Count checksum
    cCheckSum = ~cCheckSum + 1;
    eResult = utilAsciiToHex(pcInPut[0], pcInPut[1], &cTmpData);

    if((eResult != rcSUCCESS) && (cCheckSum != cTmpData))
    {
        return eH2B_STATE_DATA_ERROR;
    }

    //Add checksum at last byte
    psHexData->acData[cCount] = cCheckSum;

    if(0x01 == psHexData->cRecType)
    {
        return eH2B_STATE_END_OF_FILE;
    }
    else
    {
        return eH2B_STATE_NEXT_LINE;
    }
}

#if 1

BOOL utilIntel_Hex_Parser(const char *fileName) {
    int iLine = 0;
    char str[150];
    sINTELHEX sIntelHex;
    DWORD dwAddrOffset = 0;
    DWORD dwFileSize = 0;
    FILE* fp = NULL;

    fp = fopen(fileName, "r");

    if(NULL == fp) {
        return FALSE;
    }

    while(NULL != fgets(str, 150, fp)) {
	    if(eH2B_STATE_DATA_ERROR != utilIntelHexConverter(150, &sIntelHex, str)) {

			switch(sIntelHex.cRecType) {
			case 0x00:
			    break;

		    case 0x04:
            {
		        dwAddrOffset = (sIntelHex.acData[0] << 8) | sIntelHex.acData[1];
		        printf("%X, %X ", sIntelHex.acData[0], sIntelHex.acData[1]); 
		        printf("wAddrOffset = %04lX\n", dwAddrOffset);
                printf("0x04 addr = %ld\n", ftell(fp));
            }
		    break;

            case 0x05:
                printf("0x05 addr = %ld\n", ftell(fp));
                break;

			default:
			    break;
		    }
		}
	}

    fclose(fp);

    return 0;
}

#else

static sSegmentList *m_psSegmentListHead;

BOOL utilIntel_Hex_Parser(const char *fileName) {
    int iLine = 0;
    char str[150];
    sINTELHEX sIntelHex;
    DWORD dwAddrOffset = 0;
    FILE* fp = NULL;

    fp = fopen(fileName, "r");

    if(NULL == fp) {
        return FALSE;
    }

    m_psSegmentListHead = (sSegmentNode*)malloc(sizeof(sSegmentNode));

    while(NULL != fgets(str, 150, fp)) {
	    if(eH2B_STATE_DATA_ERROR != utilIntelHexConverter(150, &sIntelHex, str) {

			switch(sIntelHex.cRecType) {
			case 0x00:
			    break;

		    case 0x04:
            {
                sSegmentNode *psNewNode = NULL;

                psNewNode = (sSegmentNode*)malloc(sizeof(sSegmentNode))
		        dwAddrOffset = (sIntelHex.acData[0] << 8) | sIntelHex.acData[1];;
		        printf("%X, %X ", sIntelHex.acData[0], sIntelHex.acData[1]); 
		        printf("wAddrOffset = %04lX\n", dwAddrOffset);

                psNewNode->dwAddress = dwAddrOffset * 0x1000;
                m_psSegmentListHead->next = ;
            }
		    break;

			default:
			    break;
		    }
		}
	}

    fclose(fp);

    return 0;
}
#endif //1

