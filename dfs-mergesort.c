#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "dfs-sort.h"

enum {
  MIN_MERGE_LEN = 17,
};

static void short_section_sort(const char* x[], unsigned nElem);
static void recursive_merge_sort(const char* x[], size_t nElem, const char* wrk[]);
static void merge(const char* dst[], const char* x1[], size_t len1, const char* x2[], size_t len2);

int dfs_sort(const char* x[], size_t nElem)
{
  if (nElem < MIN_MERGE_LEN) {
    if (nElem > 1)
      short_section_sort(x, nElem);
    return 1; // success
  }

  const char** wrk = malloc(sizeof(const char*)*nElem);
  if (!wrk)
    return 0; // fail

  recursive_merge_sort(x, nElem, wrk);
  free(wrk);
  return 1; // success
}

// short_section_sort - sort by straight insertion
// nElem > 1
static void short_section_sort(const char* buf[], unsigned nElem)
{
  if (strcasecmp(buf[0], buf[1]) > 0) {
    const char* tmp = buf[0]; buf[0] = buf[1]; buf[1] = tmp;
  }

  for (unsigned i = 2; i < nElem; ++i) {
    const char* x = buf[i];
    if (strcasecmp(x, buf[i-1]) < 0) {
      buf[i] = buf[i-1];
      if (strcasecmp(x, buf[0]) >= 0) {
        unsigned k;
        for (k = i-2; strcasecmp(x, buf[k]) < 0; --k)
          buf[k+1] = buf[k];
        buf[k+1] = x;
      } else {
        for (unsigned k = i-1; k > 0; --k)
          buf[k] = buf[k-1];
        buf[0] = x;
      }
    }
  }
}

// recursive_merge_sort - sort by merge sort
// nElem > 1
static void recursive_merge_sort(const char* x[], size_t nElem, const char* wrk[])
{
  if (nElem < MIN_MERGE_LEN) {
    short_section_sort(x, nElem);
    return;
  }

  size_t len01 = nElem / 2;
  size_t len0  = len01 / 2;
  recursive_merge_sort(&x[0], len0, &wrk[0]);
  size_t len1  = len01 - len0;
  recursive_merge_sort(&x[len0], len1, &wrk[0]);
  merge(&wrk[0], &x[0], len0, &x[len0], len1);

  size_t len23 = nElem - len01;
  size_t len2  = len23 / 2;
  recursive_merge_sort(&x[len01], len2, &wrk[len01]);
  size_t len3  = len23 - len2;
  recursive_merge_sort(&x[len01+len2], len3, &wrk[len01]);
  merge(&wrk[len01], &x[len01], len2, &x[len01+len2], len3);

  merge(x, &wrk[0], len01, &wrk[len01], len23);
}

static void merge(const char* dst[], const char* src1[], size_t len1, const char* src2[], size_t len2)
{
  const char** end1 = &src1[len1];
  const char** end2 = &src2[len2];
  while ( src1 != end1 && src2 != end2) {
    len1 = end1 - src1;
    len2 = end2 - src2;
    size_t len = len1 <= len2 ? len1 : len2;
    const char** dstend = &dst[len];
    do {
      if (strcasecmp(*src1, *src2) <= 0)
        *dst = *src1++;
      else
        *dst = *src2++;
      ++dst;
    } while (dst != dstend);
  }
  while (src1 != end1)
    *dst++ = *src1++;
  while (src2 != end2)
    *dst++ = *src2++;
}