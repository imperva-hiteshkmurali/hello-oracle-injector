#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function pointers to hold the original functions
ssize_t (*original_nsbsend)(int, const void *, size_t, int);
ssize_t (*original_nsbrecv)(int, void *, size_t, int);

// Override nsbsend
ssize_t nsbsend(int sockfd, const void *buf, size_t len, int flags) {
    printf("Intercepted nsbsend: sockfd=%d, len=%zu\n", sockfd, len);
    // Call the original nsbsend function
    return original_nsbsend(sockfd, buf, len, flags);
}

// Override nsbrecv
ssize_t nsbrecv(int sockfd, void *buf, size_t len, int flags) {
    printf("Intercepted nsbrecv: sockfd=%d, len=%zu\n", sockfd, len);
    // Call the original nsbrecv function
    return original_nsbrecv(sockfd, buf, len, flags);
}

// Constructor to initialize the function pointers
__attribute__((constructor)) void init() {
    original_nsbsend = dlsym(RTLD_NEXT, "nsbsend");
    original_nsbrecv = dlsym(RTLD_NEXT, "nsbrecv");
    if (!original_nsbsend || !original_nsbrecv) {
        fprintf(stderr, "Error locating original functions\n");
        exit(1);
    }
}
