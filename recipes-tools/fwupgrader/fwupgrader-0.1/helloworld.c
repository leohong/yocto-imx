#if 1

#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define MAXBYTES 80

int main(int argc, char *argv[])
{
    struct timeval tv;
    fd_set readfds;
    int    num_readable;
    int    num_bytes;
    char   buf[MAXBYTES];
    int    fd_stdin;

    fd_stdin = fileno(stdin);

    while(1) {
        FD_ZERO(&readfds);
        FD_SET(fileno(stdin), &readfds);

        tv.tv_sec = 10;
        tv.tv_usec = 0;

        printf("Enter command: ");
        fflush(stdout);
        num_readable = select(fd_stdin + 1, &readfds, NULL, NULL, &tv);
        if (num_readable == -1) {
            fprintf(stderr, "\nError in select : %s\n", strerror(errno));
            exit(1);
        }
        if (num_readable == 0) {
            printf("\nPerforming default action after 10 seconds\n");
            break;  /* since I don't want to test forever */
        } else {
            num_bytes = read(fd_stdin, buf, MAXBYTES);
            if (num_bytes < 0) {
                fprintf(stderr, "\nError on read : %s\n", strerror(errno));
                exit(1);
            }
            /* process command, maybe by sscanf */
            printf("\nRead %d bytes\n", num_bytes);
            printf("\nInput String %s\n", buf);
            break; /* to terminate loop, since I don't process anything */
        }
    }

    return 0;
}

#else

#include <stdio.h>
#include "card_slot.h"

int main(int argc, char **argv)
{
    printf("Hello World!\n");

    while(1) {
	cardSlotReset();
        cardSlotInit();

	usleep(1000*1000);
    }

    return 0;
}
#endif // 1

