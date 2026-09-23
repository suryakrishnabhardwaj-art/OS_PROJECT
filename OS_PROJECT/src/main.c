#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void process_info() {
    printf("\n--- PROCESS INFORMATION ---\n");
    printf("Current Process ID : %d\n", getpid());
    printf("Parent Process ID  : %d\n", getppid());
}

void system_info() {
    printf("\n--- SYSTEM INFORMATION ---\n");
    system("uname -a");
}

void memory_info() {
    printf("\n--- MEMORY INFORMATION ---\n");
    system("free -h");
}

void process_list() {
    printf("\n--- RUNNING PROCESSES ---\n");
    system("ps -eo pid,ppid,comm --sort=pid | head -15");
}

int main() {

    int choice;

    while (1) {

        printf("\n=====================================\n");
        printf("   LINUX PROCESS & RESOURCE MANAGER\n");
        printf("=====================================\n");

        printf("1. Process Information\n");
        printf("2. System Information\n");
        printf("3. Memory Information\n");
        printf("4. Running Processes\n");
        printf("5. Exit\n");

        printf("\nEnter your choice: ");
        scanf("%d", &choice);

        switch (choice) {

            case 1:
                process_info();
                break;

            case 2:
                system_info();
                break;

            case 3:
                memory_info();
                break;

            case 4:
                process_list();
                break;

            case 5:
                printf("\nExiting project...\n");
                return 0;

            default:
                printf("\nInvalid choice.\n");
        }
    }

    return 0;
}
