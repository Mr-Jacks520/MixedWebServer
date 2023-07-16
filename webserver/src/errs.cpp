#include "errs.h"
#include <cstdio>
#include <cstdlib>

void errif(bool cond, const char* msg) {
    if (cond) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}