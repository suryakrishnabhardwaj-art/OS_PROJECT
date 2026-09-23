#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {

    pid_t pid;

    printf("=====================================\n");
    printf("          EXEC MODULE\n");
    printf("=====================================\n");

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {

        printf("\n[CHILD PROCESS]\n");
        printf("Child PID: %d\n", getpid());
        printf("Executing Linux command: ls\n\n");

        execlp("ls", "ls", "-l", NULL);

        perror("exec failed");
        exit(1);
    }

    else {

        printf("[PARENT PROCESS]\n");
        printf("Parent PID: %d\n", getpid());
        printf("Waiting for child process...\n");

        wait(NULL);

        printf("\nChild process completed.\n");
        printf("Parent process completed.\n");
    }

    return 0;
}
