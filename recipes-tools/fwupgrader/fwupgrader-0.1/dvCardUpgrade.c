
#include <string.h>
#include <stdio.h>
#include "dvCard.h"
#include "dvCardUpgrade.h"


void readFwVersion()
{
}

BOOL goToBootloader(void)
{
    BOOL ret = FALSE;

    if(rcSUCCESS == dvCard_Command_Write(eCMD_MODULE_IAP, eIAP_CMD_GO_TO_BOOTLOADER, 0, NULL)) {
        ret = TRUE;    
    }

    return ret;
}

BOOL setIapEnable(DWORD dwStartAddress, DWORD dwBinSize)
{
    BOOL ret = FALSE;
    BYTE cDummy = 0;
    sIAP_INFO sInfo;

    sInfo.dwStart = dwStartAddress;
    sInfo.dwSize = dwBinSize;

    if(rcSUCCESS == dvCard_Command_Write(eCMD_MODULE_IAP, eIAP_CMD_APP_INFO, sizeof(sIAP_INFO), (BYTE *)&sInfo)) {
        if(rcSUCCESS == dvCard_Command_Write(eCMD_MODULE_IAP, eIAP_CMD_IAP_ENABLE, 0, &cDummy)) {
            printf("Pass\n");
            ret = TRUE;    
        }
    }

    return ret;
}

BOOL writePageBinary(WORD wSize, BYTE *pcBuffer)
{
    BOOL ret = FALSE;

    if(rcSUCCESS == dvCard_Command_Write(eCMD_MODULE_IAP, eIAP_CMD_BIN_DATA, wSize, pcBuffer)) {
        ret = TRUE;    
    }

    return ret;
}

BOOL checkPageChecksum(DWORD dwChecksum)
{
    BOOL ret = FALSE;
    DWORD dwRetChecksum = 0;

    if(rcSUCCESS == dvCard_Command_Read(eCMD_MODULE_IAP, eIAP_CMD_BIN_CHECK_SUM, sizeof(DWORD), (BYTE *)&dwRetChecksum)) {
        if(0 == ((dwRetChecksum + dwChecksum) & 0xFFFFFFFF)) {
            ret = TRUE;
        }
    }

    return ret;
}

BOOL programPage(DWORD dwAddress)
{
    BOOL ret = FALSE;

    if(rcSUCCESS == dvCard_Command_Write(eCMD_MODULE_IAP, eIAP_CMD_PROGRAMING, sizeof(DWORD), (BYTE *)&dwAddress)) {
        ret = TRUE;    
    }

    return ret;
}

BOOL writeTag(DWORD dwFwVersion)
{
    BOOL ret = FALSE;

    if(rcSUCCESS == dvCard_Command_Write(eCMD_MODULE_IAP, eIAP_CMD_PROGRAMING_FINISH, sizeof(DWORD), (BYTE *)&dwFwVersion)) {
        ret = TRUE;    
    }

    return ret;
}

BOOL goToAppCode()
{
    BOOL ret = FALSE;
    if(rcSUCCESS == dvCard_Command_Write(eCMD_MODULE_IAP, eIAP_CMD_RUN_APP, 0, NULL)) {
        ret = TRUE;    
    }
    return ret;
}

#if 0
void loadFile(const QString &fileName)
{
    sMEM_TAG_PARAM *psTag = NULL;
    quint32 address = 0;
    char *data = NULL;
    QString string;
    int count = 0;

    m_intelToBin->open(fileName, (1024));
    m_intelToBin->reReadAll();
    m_intelToBin->selectSegment(1);
    m_intelToBin->readPage(address, &data, 0);

    psTag = reinterpret_cast<sMEM_TAG_PARAM*>(data);

    m_dwFwVersion = psTag->dwCodeVersion;

    for(count = 0; count < GET_COMPANY_SIZE; count++)
    {
        if(psTag->dwCompany == static_cast<DWORD>(m_strCompanyCfg.at(GET_COMPANY_ID(count)).toInt()))
        {
            m_dwCompany = static_cast<DWORD>(count);
            break;
        }
    }

    if(GET_COMPANY_SIZE > count){
        string = tr("%1 V%2.%3").arg(m_strCompanyCfg.at(GET_COMPANY_NAME(count))).arg((m_dwFwVersion >> 8 )&0x00FF).arg(QString::number(m_dwFwVersion&0x00FF));
        //showString(string);
    }
    else {
        //popMessage(tr("Firmware file is incorrect!!\r\nPlease try again!!"));
    }
}
#endif // 0

BOOL upgrade(const DWORD dwStartAddr, const WORD wPageSize, BYTE *pcBin, DWORD dwSize)
{
    DWORD dwVersion = 0x1234;
    DWORD dwChecksum = 0;
    DWORD dwAddress = 0, i = 0;

    dwAddress = dwStartAddr;

    // 1. go to bootloader
    printf("1. Go to bootloader\n");
    goToBootloader();
    usleep(2000 * 1000);

    // 2. Enable In Application Programming
    printf("2. Enable Iap ");
    if(FALSE == setIapEnable(dwAddress, dwSize)) {
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
        if(FALSE == writePageBinary(wPageSize, pcBin)) {
            printf("Transfer data Error!\n");
            goto ERROR;
        }

        printf("5. Read Checksum\n");
        if(TRUE == checkPageChecksum(dwChecksum)) {
            printf("6. Prog\n");
            programPage(dwAddress);
            if(TRUE == programPage(dwAddress)) {
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
    if(FALSE == writeTag(dwVersion)){
        printf("Transfer data Error!\n");
        goto ERROR;
    }

    // 5. go to appcode
    printf("8. Go to appCode\n");
    if(FALSE == goToAppCode()) {
        printf("Go to appCode Error!\n");
        goto ERROR;
    }

    return TRUE;

ERROR:
    printf("Error! Please try again.");

    return FALSE;
}

