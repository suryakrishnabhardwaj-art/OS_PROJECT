#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {

    int fd;
    char message[] = "OS Project File Management Test\n";

    printf("=====================================\n");
    printf("        FILE MANAGEMENT MODULE\n");
    printf("=====================================\n");

    fd = open("output/os_log.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);

    if (fd < 0) {
        perror("File open failed");
        return 1;
    }

    write(fd, message, strlen(message));

    printf("File created successfully.\n");
    printf("Data written successfully.\n");

    close(fd);

    printf("File closed successfully.\n");

    return 0;
}
