#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    FILE *file;
    char buffer[256];

    printf("=====================================\n");
    printf("       LINUX SYSTEM MONITOR\n");
    printf("=====================================\n");

    printf("\nProcess ID : %d\n", getpid());

    file = fopen("/proc/meminfo", "r");

    if (file == NULL) {
        perror("Unable to open /proc/meminfo");
        return 1;
    }

    printf("\nMemory Information:\n");

    for (int i = 0; i < 5; i++) {
        if (fgets(buffer, sizeof(buffer), file)) {
            printf("%s", buffer);
        }
    }

    fclose(file);

    printf("\nSystem monitoring completed.\n");

    return 0;
}

