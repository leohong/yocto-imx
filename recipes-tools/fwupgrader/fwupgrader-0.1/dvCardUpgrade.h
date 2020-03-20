#ifndef DVCARDUPGRADE_H
#define DVCARDUPGRADE_H

#include "Driver/Bus/types.h"


#pragma pack(push, 1)     /* set alignment to 1 byte boundary */
typedef struct
{
    DWORD dwStart;
    DWORD dwSize;
} sIAP_INFO;

typedef struct
{
    DWORD dwCodeVersion;
    DWORD dwCodeStartupAddress;
    DWORD dwCodeSize;
    DWORD dwCodeChecksum;
    DWORD dwCompany;
} sMEM_TAG_PARAM;
#pragma pack(pop)     /* set alignment to 1 byte boundary */

BOOL goToBootloader(void);
BOOL setIapEnable(DWORD dwStartAddress, DWORD dwBinSize);
BOOL writePageBinary(WORD wSize, BYTE *pcBuffer);
BOOL checkPageChecksum(DWORD dwChecksum);
BOOL programPage(DWORD dwAddress);
BOOL writeTag(DWORD dwFwVersion);
BOOL goToAppCode();
//void loadFile(const QString &fileName);
BOOL upgrade(BYTE *pcBin, DWORD dwSize);


#endif // DVCARDUPGRADE_H

