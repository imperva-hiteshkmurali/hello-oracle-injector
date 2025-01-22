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

pid_t log_oracle_process_name() {
    FILE *fp;
    char path[1035];
    pid_t pid = -1;

    // Execute the command to get Oracle process names
    fp = popen("ps -eo comm,pid | grep -E '^ora_d|^oracle$'", "r");
    if (fp == NULL) {
        fprintf(log_file, "Failed to run command\n");
        fflush(log_file);
        return -1;
    }

    // Read the output and log it
    while (fgets(path, sizeof(path) - 1, fp) != NULL) {
        fprintf(log_file, "Oracle Process: %s", path);
        fflush(log_file);

        // Extract the PID from the output
        char *token = strtok(path, " ");
        token = strtok(NULL, " ");
        if (token != NULL) {
            pid = (pid_t)strtol(token, NULL, 10);
            break; // Return the first Oracle process ID found
        }
    }

    // Close the file pointer
    pclose(fp);

    return pid;
}

void set_breakpoint(pid_t pid, void *addr) {
    long data = ptrace(PTRACE_PEEKTEXT, pid, addr, NULL);
    long trap = (data & ~0xff) | 0xcc; // 0xcc is the INT3 instruction
    ptrace(PTRACE_POKETEXT, pid, addr, trap);
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

    // Set breakpoints at nsbsend and nsbrecv (example addresses)
    void *nsbsend_addr = dlsym(handle, "nsbsend");
    void *nsbrecv_addr = dlsym(handle, "nsbrecv");
    if (nsbsend_addr) {
        set_breakpoint(pid, nsbsend_addr);
    }
    if (nsbrecv_addr) {
        set_breakpoint(pid, nsbrecv_addr);
    }

    // Detach from the process
    if (ptrace(PTRACE_DETACH, pid, NULL, NULL) == -1) {
        perror("ptrace detach");
        fprintf(log_file, "ptrace detach failed\n");
        fflush(log_file);
    }
}

void cleanup_library(pid_t pid, const char *library_path) {
    char remote_dlclose_cmd[256];
    snprintf(remote_dlclose_cmd, sizeof(remote_dlclose_cmd),
             "dlclose(dlopen(\"%s\", RTLD_LAZY))", library_path);

    // Attach to the process
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
        perror("ptrace attach");
        fprintf(log_file, "ptrace attach failed\n");
        fflush(log_file);
        return;
    }

    // Wait for the process to stop
    waitpid(pid, NULL, 0);

    // Close the library
    void *handle = dlopen(library_path, RTLD_LAZY);
    if (handle) {
        dlclose(handle);
    } else {
        fprintf(stderr, "Error opening library for cleanup: %s\n", dlerror());
        fprintf(log_file, "Error opening library for cleanup: %s\n", dlerror());
        fflush(log_file);
    }

    // Detach from the process
    if (ptrace(PTRACE_DETACH, pid, NULL, NULL) == -1) {
        perror("ptrace detach");
        fprintf(log_file, "ptrace detach failed\n");
        fflush(log_file);
    }
}

int main() {
    create_logs();
    init_context();

    // Log the Oracle process name and get the process ID
    pid_t pid = log_oracle_process_name();
    if (pid == -1) {
        fprintf(log_file, "No Oracle process found\n");
        fflush(log_file);
        printf("No Oracle process found\n");
        return 1;
    }

    fprintf(log_file, "Oracle Process ID: %d\n", pid);
    fflush(log_file);
    printf("Oracle Process ID: %d\n", pid);

    // Inject the shared library into the Oracle process
    inject_library(pid, "/lib/imperva/hello_hook.so");

    // Cleanup logic
    cleanup_library(pid, "/lib/imperva/hello_hook.so");

    if (log_file) {
        fprintf(log_file, "Cleaning up\n");
        fclose(log_file);
    }

    return 0;
}