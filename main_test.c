#include <stdio.h>
#include <stdlib.h>		// For exit() and srand()
#include <unistd.h>		// For fork() and close()
#include <sys/wait.h>	// For wait()
#include <errno.h>		// Guess
#include <pthread.h>	// Testing with threads
#include <semaphore.h>	// For use with threads
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>		// For time(), used in srand()
#include "snake.h"		// For the ioctl functions
#include <sys/ioctl.h>

int main(){
    int pid = fork();
    int x = open("/dev/MAX", O_RDONLY);
    printf("\n[pid=%d|x=%d|errno=%d]\n", pid, x, errno);
    char* buffer = malloc(sizeof(*buffer)*100);
    int y = read(x, buffer, 100);
    printf("\n{buffer:\n%s}\n", buffer);
    printf("\n[pid=%d|y=%d|errno=%d]\n", pid, y, errno);
    close(x);
    return 0;

    /*
    int x = 100;
    char* cc=malloc(sizeof(char)*4);
    sprintf(cc, "deane");
    printf("\n[%s]\n", cc);
    */
}
