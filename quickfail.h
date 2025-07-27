#include <stdio.h>
#include <stdlib.h>

#define fail(msg, ...) do {\
  fprintf(stderr, msg"\n", ##__VA_ARGS__);\
  exit(EXIT_FAILURE);\
} while (0)

#define check(cond, msg, ...) do {\
  if (!(cond)) fail(msg, ##__VA_ARGS__);\
} while (0)

#define check_mem(ptr) check(ptr, "Out of memory")
