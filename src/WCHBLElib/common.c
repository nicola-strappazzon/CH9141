// common.c

#include "common.h"

void suppress_stdout() {
    fflush(stdout);
    int fd = open("/dev/null", O_WRONLY);
    dup2(fd, STDOUT_FILENO);
    close(fd);
}

void restore_stdout(int saved_stdout) {
    fflush(stdout);
    dup2(saved_stdout, STDOUT_FILENO);
}

char* format_to_char(int value) {
    char *buffer = (char *)malloc(5 * sizeof(char));

    if (buffer != NULL) {
        snprintf(buffer, 5, "%04x", value);
    }

    return buffer;
}
