#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {

    pid_t pid;

    printf("=====================================\n");
    printf("       PROCESS CREATION MODULE\n");
    printf("=====================================\n");

    printf("Parent Process ID: %d\n", getpid());

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        printf("\n[CHILD PROCESS]\n");
        printf("Child Process ID : %d\n", getpid());
        printf("Parent Process ID: %d\n", getppid());
        printf("Child process is running...\n");

        sleep(2);

        printf("Child process completed.\n");
        exit(0);
    }

    else {
        printf("\n[PARENT PROCESS]\n");
        printf("Parent Process ID: %d\n", getpid());
        printf("Child Process ID : %d\n", pid);

        wait(NULL);

        printf("Child process has completed.\n");
        printf("Parent process completed.\n");
    }

    return 0;
}
