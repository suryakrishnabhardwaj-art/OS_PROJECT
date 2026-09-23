#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    printf("============================================\n");
    printf("   LINUX PROCESS AND RESOURCE MANAGER\n");
    printf("============================================\n");

    printf("\nProcess Information\n");
    printf("-------------------\n");

    printf("Process ID        : %d\n", getpid());
    printf("Parent Process ID : %d\n", getppid());

    printf("\nProject Status: Running Successfully\n");

    return 0;
}
