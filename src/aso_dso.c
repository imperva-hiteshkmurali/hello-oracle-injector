#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

FILE *log_file;

void create_logs() {
    log_file = fopen("/var/log/oracle_injector.log", "a");
    if (log_file == NULL) {
        perror("Failed to create log file");
        exit(1);
    }
    fprintf(log_file, "Log file created\n");
    fflush(log_file);
}

void init_context() {
    // Placeholder for context initialization logic
    fprintf(log_file, "Context initialized\n");
    fflush(log_file);
}

void log_oracle_process_name() {
    FILE *fp;
    char path[1035];

    // Execute the command to get Oracle process names
    fp = popen("ps -eo comm,pid | grep -E '^ora_|^oracle$'", "r");
    if (fp == NULL) {
        fprintf(log_file, "Failed to run command\n");
        fflush(log_file);
        exit(1);
    }

    // Read the output and log it
    while (fgets(path, sizeof(path) - 1, fp) != NULL) {
        fprintf(log_file, "Oracle Process: %s", path);
        fflush(log_file);
    }

    // Close the file pointer
    pclose(fp);
}

void inject_library(pid_t pid, const char *library_path) {
    char remote_dlopen_cmd[256];
    snprintf(remote_dlopen_cmd, sizeof(remote_dlopen_cmd),
             "dlopen(\"%s\", RTLD_LAZY)", library_path);

    // Attach to the process
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
        perror("ptrace attach");
        fprintf(log_file, "ptrace attach failed\n");
        fflush(log_file);
        return;
    }

    // Wait for the process to stop
    waitpid(pid, NULL, 0);

    // Inject the library
    void *handle = dlopen(library_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error opening library: %s\n", dlerror());
        fprintf(log_file, "Error opening library: %s\n", dlerror());
        fflush(log_file);
        ptrace(PTRACE_DETACH, pid, NULL, NULL);
        return;
    }

    // Set breakpoints at nsbsend and nsbrecv (this is a simplified example)
    // In a real scenario, you would need to find the addresses of these functions
    // and use ptrace to set breakpoints at those addresses.

    // Detach from the process
    if (ptrace(PTRACE_DETACH, pid, NULL, NULL) == -1) {
        perror("ptrace detach");
        fprintf(log_file, "ptrace detach failed\n");
        fflush(log_file);
    }
}

void _start() {
    printf("Hello, World!\n");

    // Simulate receiving a new data packet
    printf("New data packet received.\n");

    // Log the Oracle process name
    log_oracle_process_name();

    // Identify the Oracle process ID (for simplicity, we assume the first one)
    FILE *fp = popen("pgrep -f '^ora_|^oracle$'", "r");
    if (fp == NULL) {
        fprintf(log_file, "Failed to run command\n");
        fflush(log_file);
        printf("Failed to run command\n");
        exit(1);
    }

    char pid_str[10];
    if (fgets(pid_str, sizeof(pid_str) - 1, fp) != NULL) {
        pid_t pid = (pid_t)strtol(pid_str, NULL, 10);
        fprintf(log_file, "Oracle Process ID: %d\n", pid);
        fflush(log_file);
        printf("Oracle Process ID: %d\n", pid);

        // Inject the shared library into the Oracle process
        inject_library(pid, "/path/to/your/library.so");
    } else {
        fprintf(log_file, "No Oracle process found\n");
        fflush(log_file);
        printf("No Oracle process found\n");
    }

    // Close the file pointer
    pclose(fp);
}

void _aso_dso_init(void) __attribute__((constructor));
void _aso_dso_fini(void) __attribute__((destructor));

void _aso_dso_init(void) {
    create_logs();
    init_context();
    _start();
}

void _aso_dso_fini(void) {
    // Placeholder for cleanup logic
    if (log_file) {
        fprintf(log_file, "Cleaning up\n");
        fclose(log_file);
    }
}