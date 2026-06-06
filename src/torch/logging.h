#pragma once

#define DEBUG_LOGGING

#if defined(DEBUG_LOGGING)

#include <stdio.h>

#define KNRM "\x1B[0m"
#define KYEL "\x1B[33m"
#define KRED "\x1B[31m"

// printf style formatting
#define loginfo(...) printf(KNRM "[INFO] "), printf(__VA_ARGS__), printf(KNRM "\n");
#define logwarning(...) printf(KYEL "[WARNING] "), printf(__VA_ARGS__), printf( KNRM"\n");
#define logerror(...) printf(KRED "[ERROR] "), printf(__VA_ARGS__), printf( KNRM"\n");

#else

#define loginfo(...)
#define logwarning(...)
#define logerror(...)

#endif
