#include <stdlib.h>
#include <string.h>
#include <algorithm>
extern "C" {
#include "dfs-sort.h"
}

static inline bool cmp(const char* a, const char* b) {
  return strcasecmp(a, b) < 0;
}

int dfs_sort(const char* x[], size_t nElem)
{
  std::sort(x, x+nElem, cmp);
  return 1; // always succeed
}
