#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef DEBUG
#define debug(str, ...) fprintf(stderr, "DEBUG %s:%d: " str "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define debug(str, ...)
#endif

#define log_error(str, ...) fprintf(stderr, "ERROR %s:%d: " str "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define log_warning(str, ...) fprintf(stderr, "WARNING %s:%d: " str "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define log_info(str, ...) fprintf(stderr, "INFO %s:%d: " str "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#endif
