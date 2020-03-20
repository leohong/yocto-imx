#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define MAXBYTES 80

#include "dvCardUpgrade.h"

int main(int argc, char **argv) {
    FILE *fp = NULL;
    DWORD dwFileSize = 0, dwBufferSz = 0, count = 0, dwChecksum = 0;
    char *pcBin = NULL, cChar = 0;
    char *pcBuffer = NULL;

    cardSlotReset();
    cardSlotInit();

    printf("Program Start\n");
    printf("File Name: %s\n", argv[1]);

    fp = fopen(argv[1], "r");
    fseek(fp, 0, SEEK_END);
    dwFileSize = ftell(fp) - 0x8200;
    rewind(fp);

    dwBufferSz = ((dwFileSize / 1024) + 1)*1024;

    printf("File size = %ld\n", dwFileSize);
    printf("Mem size = %ld\n", dwBufferSz);

    pcBuffer = (char*)malloc(dwBufferSz);
	memset(pcBuffer, 0xFF, dwBufferSz);

    pcBin = pcBuffer;
    fseek(fp, 0x8200, SEEK_SET);
    fread (pcBin, 1, dwFileSize, fp);

    for(count = 0; count < dwBufferSz; count++)
        dwChecksum += pcBin[count];

    printf("checksum = 0x%08lX ox%08lX\n", dwChecksum, (~dwChecksum+1));

    upgrade(pcBin, dwBufferSz);

    free(pcBuffer);
    fclose(fp);

    return 0;
}


