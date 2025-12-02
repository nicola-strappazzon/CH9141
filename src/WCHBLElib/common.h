// common.h

#ifndef COMMON_H
#define COMMON_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void suppress_stdout();
void restore_stdout(int);
char* format_to_char(int);

#endif // COMMON_H
