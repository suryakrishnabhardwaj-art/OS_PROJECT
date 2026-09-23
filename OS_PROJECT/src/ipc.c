#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

int main() {

    int pipefd[2];
    pid_t pid;

    char message[] = "Hello from Parent Process!";
    char buffer[100];

    printf("=====================================\n");
    printf("       IPC - PIPE MODULE\n");
    printf("=====================================\n");

    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return 1;
    }

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {

        close(pipefd[1]);

        read(pipefd[0], buffer, sizeof(buffer));

        printf("\n[CHILD PROCESS]\n");
        printf("Child PID: %d\n", getpid());
        printf("Message received: %s\n", buffer);

        close(pipefd[0]);
    }

    else {

        close(pipefd[0]);

        printf("\n[PARENT PROCESS]\n");
        printf("Parent PID: %d\n", getpid());
        printf("Sending message to child...\n");

        write(pipefd[1], message, strlen(message) + 1);

        close(pipefd[1]);

        wait(NULL);

        printf("Child process completed.\n");
    }

    return 0;
}
