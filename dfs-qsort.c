#include <stdlib.h>
#include <string.h>
#include "dfs-sort.h"

static int cmp(const void* a, const void* b) {
  return strcasecmp(*(const char**)a, *(const char**)b);
}

int dfs_sort(const char* x[], size_t nElem)
{
  qsort(x, nElem, sizeof(x[0]), cmp);
  return 1; // always succeed
}
