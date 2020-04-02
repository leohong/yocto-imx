

#include <stdio.h>
#include <string.h>
#include "dvCard.h"
#include "dvCardUpgrade.h"

void readFwVersion() {}

BOOL goToBootloader(void)
{
    BOOL ret = FALSE;

    if (rcSUCCESS == dvCard_Command_Write(eCMD_MODULE_IAP, eIAP_CMD_GO_TO_BOOTLOADER, 0, NULL)) {
        ret = TRUE;
    }

    return ret;
}

BOOL setIapEnable(D_WORD dwStartAddress, D_WORD dwBinSize)
{
    BOOL ret = FALSE;
    BYTE cDummy = 0;
    sIAP_INFO sInfo;

    sInfo.dwStart = dwStartAddress;
    sInfo.dwSize = dwBinSize;

    if (rcSUCCESS == dvCard_Command_Write(eCMD_MODULE_IAP, eIAP_CMD_APP_INFO, sizeof(sIAP_INFO), (BYTE *) &sInfo)) {
        if (rcSUCCESS == dvCard_Command_Write(eCMD_MODULE_IAP, eIAP_CMD_IAP_ENABLE, 0, &cDummy)) {
            printf("Pass\n");
            ret = TRUE;
        }
    }

    return ret;
}

BOOL writePageBinary(WORD wSize, BYTE *pcBuffer)
{
    BOOL ret = FALSE;

    if (rcSUCCESS == dvCard_Command_Write(eCMD_MODULE_IAP, eIAP_CMD_BIN_DATA, wSize, pcBuffer)) {
        ret = TRUE;
    }

    return ret;
}

BOOL checkPageChecksum(D_WORD dwChecksum)
{
    BOOL ret = FALSE;
    D_WORD dwRetChecksum = 0;

    if (rcSUCCESS == dvCard_Command_Read(eCMD_MODULE_IAP, eIAP_CMD_BIN_CHECK_SUM, sizeof(D_WORD), (BYTE *) &dwRetChecksum)) {
        if (0 == ((dwRetChecksum + dwChecksum) & 0xFFFFFFFF)) {
            ret = TRUE;
        }
    }

    return ret;
}

BOOL programPage(D_WORD dwAddress)
{
    BOOL ret = FALSE;

    if (rcSUCCESS == dvCard_Command_Write(eCMD_MODULE_IAP, eIAP_CMD_PROGRAMING, sizeof(D_WORD), (BYTE *) &dwAddress)) {
        ret = TRUE;
    }

    return ret;
}

BOOL writeTag(D_WORD dwFwVersion)
{
    BOOL ret = FALSE;

    if (rcSUCCESS == dvCard_Command_Write(eCMD_MODULE_IAP, eIAP_CMD_PROGRAMING_FINISH, sizeof(D_WORD), (BYTE *) &dwFwVersion)) {
        ret = TRUE;
    }

    return ret;
}

BOOL goToAppCode()
{
    BOOL ret = FALSE;
    if (rcSUCCESS == dvCard_Command_Write(eCMD_MODULE_IAP, eIAP_CMD_RUN_APP, 0, NULL)) {
        ret = TRUE;
    }
    return ret;
}

BOOL upgrade(const sMEM_TAG_PARAM *psMemTag, const WORD wPageSize, BYTE *pcBin, D_WORD dwSize)
{
    D_WORD dwVersion = psMemTag->dwCodeVersion;
    D_WORD dwChecksum = 0;
    D_WORD dwAddress = 0, i = 0;

    dwAddress = psMemTag->dwCodeStartupAddress;

    // 1. go to bootloader
    printf("1. Go to bootloader\n");
    goToBootloader();
    usleep(2000 * 1000);

    // 2. Enable In Application Programming
    printf("2. Enable Iap ");
    if (FALSE == setIapEnable(dwAddress, dwSize)) {
        printf("Error\n");
        printf("Can't enter programming mode!\n");
        goto ERROR;
    }

    // 3. write binary data and check the checksum
    printf("3. Write binary\n");
    while (dwSize) {
        dwChecksum = 0;

        for (i = 0; i < wPageSize; i++) {
            dwChecksum += pcBin[i];
        }

        printf("4. Write Addr=0x%X, checksum = 0x%lX\n", dwAddress, dwChecksum);
        if (FALSE == writePageBinary(wPageSize, pcBin)) {
            printf("Transfer data Error!\n");
            goto ERROR;
        }

        printf("5. Read Checksum\n");
        if (TRUE == checkPageChecksum(dwChecksum)) {
            printf("6. Prog\n");
            programPage(dwAddress);
            if (TRUE == programPage(dwAddress)) {
                pcBin += wPageSize;
                dwSize -= wPageSize;
                dwAddress += wPageSize;
            } else {
                printf("Program Error!\n");
                goto ERROR;
            }
        } else {
            printf("Checksum Error!\n");
            goto ERROR;
        }
    }

    // 4. write tag
    printf("7. Write Tag\n");
    if (FALSE == writeTag(dwVersion)) {
        printf("Transfer data Error!\n");
        goto ERROR;
    }

    // 5. go to appcode
    printf("8. Go to appCode\n");
    if (FALSE == goToAppCode()) {
        printf("Go to appCode Error!\n");
        goto ERROR;
    }

    return TRUE;

ERROR:
    printf("Error! Please try again.");

    return FALSE;
}
