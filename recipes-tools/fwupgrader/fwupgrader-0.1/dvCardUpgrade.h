#ifndef DVCARDUPGRADE_H
#define DVCARDUPGRADE_H

#include "Driver/Bus/types.h"


#pragma pack(push, 1) /* set alignment to 1 byte boundary */
typedef struct {
    D_WORD dwStart;
    D_WORD dwSize;
} sIAP_INFO;

typedef struct {
    D_WORD dwCodeVersion;
    D_WORD dwCodeStartupAddress;
    D_WORD dwCodeSize;
    D_WORD dwCodeChecksum;
} sMEM_TAG_PARAM;
#pragma pack(pop) /* set alignment to 1 byte boundary */

BOOL goToBootloader(void);
BOOL setIapEnable(D_WORD dwStartAddress, D_WORD dwBinSize);
BOOL writePageBinary(WORD wSize, BYTE *pcBuffer);
BOOL checkPageChecksum(D_WORD dwChecksum);
BOOL programPage(D_WORD dwAddress);
BOOL writeTag(D_WORD dwFwVersion);
BOOL goToAppCode();
// void loadFile(const QString &fileName);
BOOL upgrade(const D_WORD dwStartAddr, const WORD wPageSize, BYTE *pcBin, D_WORD dwSize);


#endif  // DVCARDUPGRADE_H
