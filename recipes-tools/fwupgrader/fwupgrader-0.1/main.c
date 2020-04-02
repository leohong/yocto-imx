#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXBYTES 80

#include "dvCardUpgrade.h"
#include "utilHexToBinAPI.h"

int main(int argc, char **argv)
{
    FILE *fp = NULL;
    sHexList sHeader;
    sHexList sAppCode;
    sMEM_TAG_PARAM *psTag = NULL;

    cardSlotReset();
    cardSlotInit();

    printf("Program Start\n");
    printf("File Name: %s\n", argv[1]);
    utilIntel_Hex_Parser(argv[1], &sHeader, 1024);

    if (TRUE == utilIntel_Hex_List_Find(&sHeader, 0x1A008000, &sAppCode)) {
        printf("Address = %X\n", sAppCode.address);
        printf("Size = %x\n", sAppCode.size);

		psTag = (sMEM_TAG_PARAM *)sAppCode.pcBuffer;

		printf("Version %x\n", psTag->dwCodeVersion);
		printf("Startup %x\n", psTag->dwCodeStartupAddress);
		printf("CodeSize %x\n", psTag->dwCodeSize);
		printf("Checksum %x\n", psTag->dwCodeChecksum);

    } else {
        printf("Not Found!!\n");
    }

    if (TRUE == utilIntel_Hex_List_Find(&sHeader, 0x1A008200, &sAppCode)) {
        printf("Address = %X\n", sAppCode.address);
        printf("Size = %x\n", sAppCode.size);

        upgrade(psTag, 1024, sAppCode.pcBuffer, sAppCode.size);
    } else {
        printf("Not Found!\n");
    }
    return 0;
}
