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
	char *str[100];

    cardSlotReset();
    cardSlotInit();

	while(1) {
		printf("Program Start\n");
		printf("File Name: %s\n", argv[1]);

		fp = fopen(argv[1], "r");
		fseek(fp, 0, SEEK_END);
		dwFileSize = ftell(fp) - 0x8000;
		rewind(fp);

		dwBufferSz = ((dwFileSize / 1024) + 1)*1024;

		pcBuffer = (char*)malloc(dwBufferSz);
		memset(pcBuffer, 0xFF, dwBufferSz);

		pcBin = pcBuffer;
		fseek(fp, 0x8000, SEEK_SET);
		fread (pcBin, 1, dwFileSize, fp);

		for(count = 0x200; count < dwBufferSz; count++)
		    dwChecksum += pcBin[count];

        printf("0x8200 to 0x%0lX", (0x8000+dwBufferSz));
		printf("File size = %ld", dwFileSize);
		printf("Mem size = %ld \n", dwBufferSz);
		printf("checksum = 0x%08lX 0x%08lX\n", dwChecksum, (~dwChecksum+1));

		upgrade(pcBin, dwBufferSz);
		gets(str);
	}
    free(pcBuffer);
    fclose(fp);

    return 0;
}


