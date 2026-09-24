#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>

/* ============================================================
                     GLOBAL VARIABLES
   ============================================================ */

/* CO-3 : Signal communication */
volatile sig_atomic_t signal_received = 0;

/* CO-6 : Shared data */
int shared_counter = 0;

/* CO-6 : Mutex */
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

/* CO-6 : Condition variable */
pthread_cond_t counter_condition = PTHREAD_COND_INITIALIZER;

/* CO-6 : Semaphore */
sem_t monitor_semaphore;

/* CO-6 : Thread coordination */
int thread_ready = 0;


/* ============================================================
                    SIGNAL HANDLER
                    CO-3
   ============================================================ */

void signal_handler(int sig)
{
    if (sig == SIGUSR1)
    {
        signal_received = 1;
    }
}


/* ============================================================
                  CO-6 THREAD FUNCTION
   ============================================================ */

void *thread_monitor(void *arg)
{
    (void)arg;

    /* Semaphore synchronization */
    sem_wait(&monitor_semaphore);

    /* Mutex protects shared data */
    pthread_mutex_lock(&counter_mutex);

    shared_counter++;

    thread_ready = 1;

    /* Condition variable notification */
    pthread_cond_signal(&counter_condition);

    pthread_mutex_unlock(&counter_mutex);

    sem_post(&monitor_semaphore);

    return NULL;
}


/* ============================================================
             READ /PROC FILE HELPER
             CO-1 / CO-4 / CO-5
   ============================================================ */

int read_proc_value(const char *path, char *buffer, size_t size)
{
    int fd;
    ssize_t n;

    fd = open(path, O_RDONLY);

    if (fd == -1)
        return -1;

    n = read(fd, buffer, size - 1);

    close(fd);

    if (n <= 0)
        return -1;

    buffer[n] = '\0';

    return 0;
}


/* ============================================================
                GET MEMORY INFORMATION
                CO-4
   ============================================================ */

void show_memory_information(pid_t pid)
{
    char path[PATH_MAX];
    char buffer[8192];

    printf("\n========== CO-4 MEMORY MANAGEMENT ==========\n");

    printf("Process Virtual Memory Information\n");

    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    if (read_proc_value(path, buffer, sizeof(buffer)) == 0)
    {
        char *line;

        line = strtok(buffer, "\n");

        while (line != NULL)
        {
            if (strncmp(line, "VmSize:", 7) == 0 ||
                strncmp(line, "VmRSS:", 6) == 0 ||
                strncmp(line, "VmData:", 7) == 0 ||
                strncmp(line, "VmStk:", 6) == 0 ||
                strncmp(line, "VmExe:", 6) == 0 ||
                strncmp(line, "VmLib:", 6) == 0)
            {
                printf("%s\n", line);
            }

            line = strtok(NULL, "\n");
        }
    }

    /*
        Linux process address space layout
    */

    printf("\nVirtual Address Space / Page Mapping:\n");

    snprintf(path, sizeof(path), "/proc/%d/maps", pid);

    if (read_proc_value(path, buffer, sizeof(buffer)) == 0)
    {
        char *line;
        int count = 0;

        line = strtok(buffer, "\n");

        while (line != NULL && count < 8)
        {
            printf("%s\n", line);

            line = strtok(NULL, "\n");
            count++;
        }

        if (count >= 8)
            printf("... additional mappings omitted ...\n");
    }

    /*
        Dynamic memory allocation
    */

    int *dynamic_memory = malloc(10 * sizeof(int));

    if (dynamic_memory != NULL)
    {
        for (int i = 0; i < 10; i++)
            dynamic_memory[i] = i;

        printf("\nDynamic memory allocation: SUCCESS\n");
        printf("Heap allocation address: %p\n",
               (void *)dynamic_memory);

        free(dynamic_memory);

        printf("Dynamic memory released using free().\n");
    }

    /*
        Anonymous memory mapping
    */

    void *memory = mmap(NULL,
                        4096,
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS,
                        -1,
                        0);

    if (memory != MAP_FAILED)
    {
        strcpy((char *)memory, "Kernel Monitor mmap memory");

        printf("mmap() allocation: SUCCESS\n");
        printf("Mapped memory address: %p\n", memory);
        printf("Mapped content: %s\n", (char *)memory);

        munmap(memory, 4096);

        printf("munmap(): SUCCESS\n");
    }

    printf("\nMemory concepts demonstrated:\n");
    printf("- Virtual memory\n");
    printf("- Process address space\n");
    printf("- Page mappings through /proc/PID/maps\n");
    printf("- Dynamic memory allocation\n");
    printf("- mmap() memory mapping\n");
    printf("- Copy-on-Write through fork()\n");
}


/* ============================================================
                  CO-5 FILE SYSTEM INFORMATION
   ============================================================ */

void show_file_information(void)
{
    const char *file_name = "/tmp/kernel_monitor_file.txt";

    int fd;
    struct stat file_stat;

    printf("\n========== CO-5 FILE SYSTEM ==========\n");

    /*
        Create/open file
    */

    fd = open(file_name,
              O_CREAT | O_RDWR | O_TRUNC,
              0644);

    if (fd == -1)
    {
        perror("open");
        return;
    }

    const char *message =
        "Kernel Monitor File System Test\n";

    /*
        Unbuffered file I/O
    */

    write(fd, message, strlen(message));

    /*
        File information
    */

    if (fstat(fd, &file_stat) == 0)
    {
        printf("File: %s\n", file_name);
        printf("File Descriptor: %d\n", fd);
        printf("Inode Number: %lu\n",
               (unsigned long)file_stat.st_ino);
        printf("File Size: %ld bytes\n",
               (long)file_stat.st_size);
        printf("File Permissions: %o\n",
               file_stat.st_mode & 0777);
    }

    /*
        Move file offset back to beginning
    */

    lseek(fd, 0, SEEK_SET);

    char buffer[128];

    ssize_t bytes = read(fd,
                         buffer,
                         sizeof(buffer) - 1);

    if (bytes > 0)
    {
        buffer[bytes] = '\0';

        printf("\nFile read using read():\n");
        printf("%s", buffer);
    }

    /*
        Memory mapped file I/O
    */

    if (file_stat.st_size > 0)
    {
        void *mapped = mmap(NULL,
                            file_stat.st_size,
                            PROT_READ,
                            MAP_PRIVATE,
                            fd,
                            0);

        if (mapped != MAP_FAILED)
        {
            printf("\nMemory-mapped file I/O: SUCCESS\n");
            printf("Mapped content: %s",
                   (char *)mapped);

            munmap(mapped,
                   file_stat.st_size);
        }
    }

    close(fd);

    printf("\nFile descriptor closed.\n");

    printf("\nFile-system concepts demonstrated:\n");
    printf("- Unix file abstraction\n");
    printf("- File descriptor\n");
    printf("- open/read/write/close\n");
    printf("- inode\n");
    printf("- File metadata using stat\n");
    printf("- Memory-mapped file I/O\n");
    printf("- Linux /proc virtual filesystem\n");
}


/* ============================================================
                CO-3 IPC INFORMATION
   ============================================================ */

void demonstrate_ipc(pid_t child_pid)
{
    int pipefd[2];

    printf("\n========== CO-3 IPC ==========\n");

    /*
        Anonymous pipe
    */

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return;
    }

    printf("Anonymous pipe created.\n");
    printf("Read FD  : %d\n", pipefd[0]);
    printf("Write FD : %d\n", pipefd[1]);

    const char *pipe_message =
        "IPC message from Kernel Monitor";

    write(pipefd[1],
          pipe_message,
          strlen(pipe_message) + 1);

    char pipe_buffer[128];

    read(pipefd[0],
         pipe_buffer,
         sizeof(pipe_buffer));

    printf("Pipe received: %s\n",
           pipe_buffer);

    close(pipefd[0]);
    close(pipefd[1]);

    /*
        Named pipe / FIFO
    */

    const char *fifo_name =
        "/tmp/kernel_monitor_fifo";

    unlink(fifo_name);

    if (mkfifo(fifo_name, 0666) == 0)
    {
        printf("Named pipe (FIFO) created: %s\n",
               fifo_name);

        unlink(fifo_name);

        printf("Named pipe removed after demonstration.\n");
    }

    /*
        Signal information
    */

    printf("\nSignal information:\n");
    printf("SIGUSR1 handler installed.\n");

    if (child_pid > 0)
    {
        printf("Child PID available for signal communication: %d\n",
               child_pid);
    }

    printf("Signal handler uses signal(SIGUSR1, ...).\n");
    printf("kill() is used for asynchronous process notification.\n");

    /*
        Process group/session information
    */

    printf("\nProcess Group Information:\n");
    printf("Current PID       : %d\n", getpid());
    printf("Parent PID        : %d\n", getppid());
    printf("Process Group ID  : %d\n", getpgrp());
    printf("Session ID        : %d\n",
           getsid(0));
}


/* ============================================================
                    CO-2 PROCESS INFORMATION
   ============================================================ */

void show_process_information(pid_t child_pid)
{
    char path[PATH_MAX];
    char buffer[8192];

    printf("\n========== CO-2 PROCESS CONTROL ==========\n");

    printf("Monitor PID       : %d\n", getpid());
    printf("Parent PID        : %d\n", getppid());

    if (child_pid > 0)
        printf("Child PID         : %d\n", child_pid);

    printf("Process Group ID  : %d\n", getpgrp());

    printf("\nProcess lifecycle:\n");
    printf("1. Parent process created\n");
    printf("2. fork() creates child\n");
    printf("3. Child executes command\n");
    printf("4. Parent waits using waitpid()\n");
    printf("5. Child terminates\n");

    /*
        Linux scheduling information
    */

    snprintf(path,
             sizeof(path),
             "/proc/%d/sched",
             child_pid > 0 ? child_pid : getpid());

    if (read_proc_value(path,
                        buffer,
                        sizeof(buffer)) == 0)
    {
        char *line;

        line = strtok(buffer, "\n");

        printf("\nLinux Scheduling Information:\n");

        int count = 0;

        while (line != NULL && count < 8)
        {
            printf("%s\n", line);

            line = strtok(NULL, "\n");
            count++;
        }
    }
}


/* ============================================================
                 CO-1 SYSTEM INFORMATION
   ============================================================ */

void show_system_information(void)
{
    struct sysinfo info;

    printf("\n========== CO-1 OS SERVICE LAYER ==========\n");

    printf("Kernel interface: Linux /proc and system calls\n");

    printf("Current User PID : %d\n", getpid());

    /*
        System call
    */

    long thread_id = syscall(SYS_gettid);

    printf("Current Thread ID: %ld\n",
           thread_id);

    /*
        System information
    */

    if (sysinfo(&info) == 0)
    {
        printf("System Uptime: %ld seconds\n",
               info.uptime);

        printf("Total RAM: %lu MB\n",
               info.totalram / (1024 * 1024));

        printf("Free RAM: %lu MB\n",
               info.freeram / (1024 * 1024));
    }

    printf("\nOS Service Concepts:\n");
    printf("- User Space\n");
    printf("- Kernel Space\n");
    printf("- System Calls\n");
    printf("- Kernel Services\n");
    printf("- Linux system programming\n");
    printf("- Shell command execution\n");
}


/* ============================================================
              CO-6 THREAD / SYNCHRONIZATION
   ============================================================ */

void demonstrate_threads(void)
{
    pthread_t thread;

    printf("\n========== CO-6 CONCURRENCY ==========\n");

    /*
        Initialize semaphore
    */

    if (sem_init(&monitor_semaphore,
                 0,
                 1) != 0)
    {
        perror("sem_init");
        return;
    }

    shared_counter = 0;
    thread_ready = 0;

    /*
        Create POSIX thread
    */

    if (pthread_create(&thread,
                       NULL,
                       thread_monitor,
                       NULL) != 0)
    {
        perror("pthread_create");
        sem_destroy(&monitor_semaphore);
        return;
    }

    /*
        Wait for thread
    */

    pthread_mutex_lock(&counter_mutex);

    while (!thread_ready)
    {
        pthread_cond_wait(&counter_condition,
                          &counter_mutex);
    }

    pthread_mutex_unlock(&counter_mutex);

    pthread_join(thread, NULL);

    printf("POSIX thread created successfully.\n");
    printf("Shared counter: %d\n",
           shared_counter);

    printf("Mutex synchronization: SUCCESS\n");
    printf("Condition variable: SUCCESS\n");
    printf("Counting semaphore: SUCCESS\n");
    printf("Thread coordination: SUCCESS\n");

    sem_destroy(&monitor_semaphore);

    printf("\nConcurrency concepts demonstrated:\n");
    printf("- Process vs thread\n");
    printf("- POSIX threads\n");
    printf("- Shared data\n");
    printf("- Race-condition prevention\n");
    printf("- Mutex\n");
    printf("- Condition variable\n");
    printf("- Semaphore\n");
    printf("- Thread synchronization\n");
}


/* ============================================================
                 COMMAND EXECUTION
                 CO-1 / CO-2
   ============================================================ */

pid_t execute_command(const char *command)
{
    int command_pipe[2];

    if (pipe(command_pipe) == -1)
    {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        close(command_pipe[0]);
        close(command_pipe[1]);
        return -1;
    }

    /*
        CHILD
    */

    if (pid == 0)
    {
        char received_command[512];

        close(command_pipe[1]);

        ssize_t bytes = read(command_pipe[0],
                             received_command,
                             sizeof(received_command) - 1);

        close(command_pipe[0]);

        if (bytes <= 0)
            exit(1);

        received_command[bytes] = '\0';

        /*
            Signal parent
        */

        kill(getppid(), SIGUSR1);

        /*
            Command execution through shell.
            This demonstrates the shell as a user-space
            program and command execution journey.
        */

        char *args[] =
        {
            "/bin/sh",
            "-c",
            received_command,
            NULL
        };

        execvp("/bin/sh", args);

        perror("execvp");
        exit(1);
    }

    /*
        PARENT
    */

    close(command_pipe[0]);

    write(command_pipe[1],
          command,
          strlen(command) + 1);

    close(command_pipe[1]);

    return pid;
}


/* ============================================================
                         MAIN
   ============================================================ */

int main(void)
{
    char command[512];

    /*
        Install SIGUSR1 handler
    */

    signal(SIGUSR1, signal_handler);

    /*
        SINGLE INPUT
    */

    printf("====================================================\n");
    printf("              KERNEL MONITOR\n");
    printf("             CO-1 TO CO-6 SYSTEM\n");
    printf("====================================================\n");

    printf("\nEnter ONE Linux command: ");

    if (fgets(command,
              sizeof(command),
              stdin) == NULL)
    {
        return 1;
    }

    command[strcspn(command, "\n")] = '\0';

    if (strlen(command) == 0)
    {
        printf("No command entered.\n");
        return 1;
    }

    /*
        Execute command using process creation
    */

    pid_t child_pid =
        execute_command(command);

    if (child_pid < 0)
        return 1;

    /*
        CO-1
    */

    show_system_information();

    /*
        CO-2
    */

    show_process_information(child_pid);

    /*
        CO-3
    */

    demonstrate_ipc(child_pid);

    /*
        CO-4
    */

    show_memory_information(getpid());

    /*
        CO-5
    */

    show_file_information();

    /*
        CO-6
    */

    demonstrate_threads();

    /*
        Wait for child
    */

    int status;

    waitpid(child_pid,
            &status,
            0);

    /*
        Final kernel monitor information
    */

    printf("\n====================================================\n");
    printf("             KERNEL MONITOR SUMMARY\n");
    printf("====================================================\n");

    printf("Command monitored : %s\n",
           command);

    printf("Child PID         : %d\n",
           child_pid);

    if (WIFEXITED(status))
    {
        printf("Child exit status : %d\n",
               WEXITSTATUS(status));
    }

    printf("SIGUSR1 received  : %s\n",
           signal_received ? "YES" : "NO");

    printf("\nCO-1 : OS Service Layer              [DONE]\n");
    printf("CO-2 : Process Control               [DONE]\n");
    printf("CO-3 : IPC and Signals               [DONE]\n");
    printf("CO-4 : Memory Management             [DONE]\n");
    printf("CO-5 : File Systems and File I/O     [DONE]\n");
    printf("CO-6 : Concurrency and Synchronization[DONE]\n");

    printf("\n====================================================\n");
    printf("          KERNEL MONITOR COMPLETED\n");
    printf("====================================================\n");

    return 0;
}
