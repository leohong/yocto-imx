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

#if 0
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

#endif //1

#include <stdlib.h>
#include <stdio.h>
#include "utilHexToBinAPI.h"

BOOL utilIntel_Hex_Parser(const char *fileName) {
    char str[128];
    char data[128];
    int length = 0, address = 0, recType = 0;
    int lastaddr = 0, lastLength = 0, firstLine = TRUE, sectionNum = 0;
    int addrOffset = 0, sectionEnd = 0, sectorSize = 0, i = 0;
    int count = 0;

    sHexList sHeader;
    sHexList *psCurrent = &sHeader;
    FILE* fp = NULL;

    printf("1. Header %x\n", psCurrent);

    fp = fopen(fileName, "r");

    if(NULL == fp) {
        return FALSE;
    }

    while(NULL != fgets(str, 150, fp)) {
		if(4 == sscanf(str, ":%2X%4X%2X%s", &length, &address, &recType, data)) {

            switch(recType) {
            case 0x04:
                if(TRUE != firstLine) {
                    sectionEnd = addrOffset + lastaddr + lastLength;
                    printf("1. Section End %X\n", sectionEnd - 1);
                    printf("1. Section Size %X\n\n", sectorSize);

                    // TODO: Malloc a character array with the sectorSize
                    psCurrent->size = sectorSize;
                    psCurrent->pcBuffer = malloc(sizeof(char)*sectorSize);

                    printf("3. nb %d, %x\n", sectionNum, &psCurrent->pcBuffer);
                }
                sscanf(data, "%4X", &addrOffset);
                addrOffset *= 0x10000;
				firstLine = TRUE;
                break;

            case 0x00:
                if(TRUE == firstLine) {
                    printf("%X %X\n", sectionEnd, (addrOffset + address - 1));
                    if(sectionEnd != (addrOffset + address)) {
                        printf("Section %d 0x%X\n", sectionNum, addrOffset + address);
                        sectionNum++;
                        sectorSize = 0;

                        // TODO: Create a new list node, fill the start address
                        {
                            sHexList *psNewNode;
                            psNewNode = malloc(sizeof(sHexList));
                            psNewNode->next = NULL;
                            psNewNode->address = addrOffset + address;
                            psCurrent->next = psNewNode;
                            psCurrent = psNewNode;
                            printf("2. n %d, %x\n", sectionNum, psCurrent);
                        }
                    }
                    firstLine = FALSE;
                } 
                lastaddr = address;
                lastLength = length;
                sectorSize += length;
                break;

            case 0x01:
                printf("End of File\n");
                break;

            case 0x02:
                printf("Extended Segment Address 0x%s\n", data);
                break;

            case 0x03:
                printf("Start Segment Address 0x%s\n", data);
                break;

            case 0x05:
                sectionEnd = addrOffset + lastaddr + lastLength;
                // TODO: int *arr1 = realloc(size, sizeof(int));
                {
                    char *pcBuffer2 = realloc(psCurrent->pcBuffer, sectorSize);
                    psCurrent->size = sectorSize;
                    psCurrent->pcBuffer = pcBuffer2;
                    printf("3. nb %d, %x\n", sectionNum, &psCurrent->pcBuffer);
                }
                printf("2. Section End %X\n", sectionEnd - 1);
                printf("2. Section Size %X\n\n", sectorSize);
                printf("Start Linear Address 0x%s\n", data);
                break;

            default:
                break;
            }

        } else {
            printf("Invalid hex file\n");
            goto ERROR;
        }
	}

    rewind(fp);

    length = address = recType = 0;
    lastaddr = lastLength = sectionNum = 0;
    addrOffset = sectionEnd = sectorSize = i = 0;
    firstLine = TRUE;
    psCurrent = &sHeader;

    while(NULL != fgets(str, 150, fp)) {
		if(4 == sscanf(str, ":%2X%4X%2X%s", &length, &address, &recType, data)) {
            switch(recType) {
            case 0x04:
                if(TRUE != firstLine) {
                    sectionEnd = addrOffset + lastaddr + lastLength;
                    printf("1. Section End %X\n", sectionEnd - 1);
                    printf("1. Section Size %X\n", sectorSize);
                }
                sscanf(data, "%4X", &addrOffset);
                addrOffset *= 0x10000;
				firstLine = TRUE;
                break;

            case 0x00:
                if(TRUE == firstLine) {
                    printf("%X %X\n", sectionEnd, (addrOffset + address - 1));
                    if(sectionEnd != (addrOffset + address)) {
                        printf("Write Count %X\n\n", count*16);
                        count = 0;
                        printf("Section %d 0x%X\n", sectionNum, addrOffset + address);
                        sectionNum++;
                        sectorSize = 0;

                        psCurrent = psCurrent->next;
                        printf("Addr %X, Size %X\n", psCurrent->address, psCurrent->size);
                    }
                    firstLine = FALSE;
                } 
                lastaddr = address;
                lastLength = length;
                sectorSize += length;
                count++;

				// TODO: Copy data to buffer
                for(i = 0; i < length; i++) {
                    int tmp = 0;
                    if(1 == sscanf(&data[i*2], "%2X", &tmp)) {
                        //printf("%X", tmp);
                        psCurrent->pcBuffer[i] = tmp;
                    }
                }
                //printf("\n");
                break;

            case 0x01:
                printf("End of File\n");
                break;

            case 0x02:
                printf("Extended Segment Address 0x%s\n", data);

                break;

            case 0x03:
                printf("Start Segment Address 0x%s\n", data);
                break;

            case 0x05:
                sectionEnd = addrOffset + lastaddr + lastLength;
                printf("2. Section End %X\n", sectionEnd - 1);
                printf("2. Section Size %X\n\n", sectorSize);
                printf("Write Count %X\n\n", count*16);
                printf("Start Linear Address 0x%s\n", data);
                break;

            default:
                break;
			}
        } else {
            printf("Invalid hex file\n");
            goto ERROR;
        }
	}

	// Free lsit
    while(NULL != sHeader.next) {
        sHexList *psCurrent = sHeader.next;

        printf("%X\n", psCurrent);

        sHeader.next = psCurrent->next;

        printf("4. %X\n", psCurrent->address);
        printf("5. %X\n", psCurrent->size);
        printf("6. %X\n", psCurrent->pcBuffer);
        free(psCurrent->pcBuffer);
        printf("7. %X\n", psCurrent);
        free(psCurrent);
    }

ERROR:
    fclose(fp);

    return 0;
}


