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
#include <stdlib.h>
#include "utilHexToBinAPI.h"

BOOL utilIntel_Hex_List_Find(sHexList *psHeader, int address, sHexList *psNode)
{
    sHexList *psCurrent = psHeader->next;

    while (NULL != psCurrent) {
        if (address == psCurrent->address) {
            *psNode = *psCurrent;
            return TRUE;
        }
        psCurrent = psCurrent->next;
    }
    return FALSE;
}

void utilIntel_Hex_List_Free(sHexList *psHeader)
{
    // Free lsit
    while (NULL != psHeader->next) {
        sHexList *psCurrent = psHeader->next;

        // printf("%X\n", psCurrent);

        psHeader->next = psCurrent->next;

        // printf("4. %X\n", psCurrent->address);
        // printf("5. %X\n", psCurrent->size);
        // printf("6. %X\n", psCurrent->pcBuffer);
        free(psCurrent->pcBuffer);
        // printf("7. %X\n", psCurrent);
        free(psCurrent);
    }
}

BOOL utilIntel_Hex_Parser(const char *fileName, sHexList *psHeader, int pageSize)
{
    char str[128];
    char data[128];
    int length = 0, address = 0, recType = 0;
    int lastaddr = 0, lastLength = 0, firstLine = TRUE, sectionNum = 0;
    int addrOffset = 0, sectionEnd = 0, sectorSize = 0, i = 0;
    int count = 0;
    // sHexList sHeader;
    sHexList *psCurrent = psHeader;
    FILE *fp = NULL;

    // printf("1. Header %x\n", psCurrent);

    fp = fopen(fileName, "r");

    if (NULL == fp) {
        return FALSE;
    }

    while (NULL != fgets(str, 150, fp)) {
        if (4 == sscanf(str, ":%2X%4X%2X%s", &length, &address, &recType, data)) {
            switch (recType) {
            case 0x04:
                if (TRUE != firstLine) {
                    sectionEnd = addrOffset + lastaddr + lastLength;
                    // printf("1. Section End %X\n", sectionEnd - 1);
                    // printf("1. Section Size %X\n\n", sectorSize);

                    // TODO: Malloc a character array with the sectorSize
                    psCurrent->size = ((sectorSize / pageSize) + 1) * pageSize;
                    psCurrent->pcBuffer = malloc(sizeof(char) * sectorSize);
                    // printf("3. nb %d, %x\n", sectionNum, &psCurrent->pcBuffer);
                }
                sscanf(data, "%4X", &addrOffset);
                addrOffset *= 0x10000;
                firstLine = TRUE;
                break;

            case 0x00:
                if (TRUE == firstLine) {
                    // printf("%X %X\n", sectionEnd, (addrOffset + address - 1));
                    if (sectionEnd != (addrOffset + address)) {
                        // printf("Section %d 0x%X\n", sectionNum, addrOffset + address);
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
                            // printf("2. n %d, %x\n", sectionNum, psCurrent);
                        }
                    }
                    firstLine = FALSE;
                }
                lastaddr = address;
                lastLength = length;
                sectorSize += length;
                break;

            case 0x01:
                // printf("End of File\n");
                break;

            case 0x02:
                // printf("Extended Segment Address 0x%s\n", data);
                break;

            case 0x03:
                // printf("Start Segment Address 0x%s\n", data);
                break;

            case 0x05:
                sectionEnd = addrOffset + lastaddr + lastLength;
                // TODO: int *arr1 = realloc(size, sizeof(int));
                {
                    char *pcBuffer2 = NULL;
                    psCurrent->size = ((sectorSize / pageSize) + 1) * pageSize;
                    pcBuffer2 = realloc(psCurrent->pcBuffer, sectorSize);
                    psCurrent->pcBuffer = pcBuffer2;
                    // printf("3. nb %d, %x\n", sectionNum, &psCurrent->pcBuffer);
                }
                // printf("2. Section End %X\n", sectionEnd - 1);
                // printf("2. Section Size %X\n\n", sectorSize);
                // printf("Start Linear Address 0x%s\n", data);
                break;

            default:
                break;
            }

        } else {
            // printf("Invalid hex file\n");
            goto ERROR;
        }
    }

    rewind(fp);

    length = address = recType = 0;
    lastaddr = lastLength = sectionNum = 0;
    addrOffset = sectionEnd = sectorSize = i = 0;
    firstLine = TRUE;
    psCurrent = psHeader;

    while (NULL != fgets(str, 150, fp)) {
        if (4 == sscanf(str, ":%2X%4X%2X%s", &length, &address, &recType, data)) {
            switch (recType) {
            case 0x04:
                if (TRUE != firstLine) {
                    sectionEnd = addrOffset + lastaddr + lastLength;
                    // printf("1. Section End %X\n", sectionEnd - 1);
                    // printf("1. Section Size %X\n", sectorSize);
                }
                sscanf(data, "%4X", &addrOffset);
                addrOffset *= 0x10000;
                firstLine = TRUE;
                break;

            case 0x00:
                if (TRUE == firstLine) {
                    // printf("%X %X\n", sectionEnd, (addrOffset + address - 1));
                    if (sectionEnd != (addrOffset + address)) {
                        // printf("Write Count %X\n\n", count);
                        count = 0;
                        // printf("Section %d 0x%X\n", sectionNum, addrOffset + address);
                        sectionNum++;
                        sectorSize = 0;

                        psCurrent = psCurrent->next;
                        // printf("Addr %X, Size %X\n", psCurrent->address, psCurrent->size);
                    }
                    firstLine = FALSE;
                }
                lastaddr = address;
                lastLength = length;
                sectorSize += length;

                // TODO: Copy data to buffer
                for (i = 0; i < length; i++) {
                    int tmp = 0;
                    if (1 == sscanf(&data[i * 2], "%2X", &tmp)) {
                        // printf("%X", tmp);
                        psCurrent->pcBuffer[count++] = tmp;
                    }
                }
                // printf("\n");
                break;

            case 0x01:
                // printf("End of File\n");
                break;

            case 0x02:
                // printf("Extended Segment Address 0x%s\n", data);
                break;

            case 0x03:
                // printf("Start Segment Address 0x%s\n", data);
                break;

            case 0x05:
                sectionEnd = addrOffset + lastaddr + lastLength;
                // printf("2. Section End %X\n", sectionEnd - 1);
                // printf("2. Section Size %X\n\n", sectorSize);
                // printf("Write Count %X\n\n", count);
                // printf("Start Linear Address 0x%s\n", data);
                break;

            default:
                break;
            }

        } else {
            // printf("Invalid hex file\n");
            goto ERROR;
        }
    }

ERROR:
    fclose(fp);

    return 0;
}
