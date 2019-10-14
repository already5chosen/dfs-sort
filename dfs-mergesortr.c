// Implement lexicographic case-insensitive sort as combination of radix sort and mergesort.
// mergesort is used as inner engine, radix sort by most-significant digit first and radix = 2**32
// serves as outer algorithm

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <memory.h>
#include "dfs-sort.h"

enum {
  MIN_MERGE_LEN = 110,
};

static void short_section_sort_u64(uint64_t buf[], unsigned nElem);
static void merge_sort_u64(uint64_t buf[], size_t nElem, uint64_t wrk[]);
static void merge_u64(uint64_t dst[], uint64_t src1[], size_t len1, uint64_t src2[], size_t len2);
static void recursive_sort(uint64_t y[], uint32_t nElem, uint64_t wrk[], const char* x[], unsigned offs);

static inline uint32_t load4(const char* str) {
  int c0 = str[0], c1=0, c2=0, c3 = 0;
  if (c0) {
    c0 = tolower(c0);
    c1 = str[1];
    if (c1) {
      c1 = tolower(c1);
      c2 = str[2];
      if (c2) {
        c2 = tolower(c2);
        c3 = tolower(str[3]);
      }
    }
  }
  return
    ((uint32_t)(uint8_t)c0<<24)|
    ((uint32_t)(uint8_t)c1<<16)|
    ((uint32_t)(uint8_t)c2<< 8)|
     (uint32_t)(uint8_t)c3;
}


int dfs_sort(const char* x[], size_t nElem)
{
  if (sizeof(uintptr_t) > sizeof(uint64_t))
    return 0; // platforms, on each pointer is bigger than 64 bits are not supported

  if (nElem > UINT32_MAX)
    return 0; // number of elements that does not fit in 32-bit word is not supported

  if (nElem <= 1)
    return 1; // success

  uint64_t* wrkbuf = malloc(sizeof(uint64_t)*nElem*2);
  if (!wrkbuf)
    return 0; // fail

  uint64_t* y = wrkbuf;
  for (uint32_t i = 0; i < (uint32_t)nElem; ++i)
    y[i] = ((uint64_t)load4(x[i]) << 32) | i;

  // sort by 4 MS characters
  uint64_t* wrk = wrkbuf + nElem;
  merge_sort_u64(y, nElem, wrk);

  // sort sub-sections that have identical prefix
  const uint64_t VAL_MSK = (uint64_t)(-1) << 32;
  uint64_t valLast = y[nElem-1] & VAL_MSK;
  for (uint32_t i0 = 0; i0 != (uint32_t)nElem;) {
    uint64_t val0 = y[i0] & VAL_MSK;
    uint32_t i1 = nElem;
    if (val0 != valLast) {
      i1 = i0 + 1;
      while ((y[i1] & VAL_MSK) == val0) ++i1;
    }
    if (i1 - i0 > 1 && (uint8_t)(val0 >> 32) != 0)
      recursive_sort(&y[i0], i1 - i0, wrk, x, 4);
    for (uint32_t k = i0; k < i1; ++k)
      y[k] = (uint64_t)x[(uint32_t)y[k]];
    i0 = i1;
  }

  for (uint32_t i = 0; i < (uint32_t)nElem; ++i)
    x[i] = (const char*)(y[i]);

  free(wrkbuf);
  return 1; // success
}

// short_section_sort_u64 - sort by straight insertion
// nElem > 1
static void short_section_sort_u64(uint64_t buf[], unsigned nElem)
{
  if (buf[0] > buf[1]) {
    uint64_t tmp = buf[0]; buf[0] = buf[1]; buf[1] = tmp;
  }

  for (unsigned i = 2; i < nElem; ++i) {
    uint64_t x = buf[i];
    if (x < buf[i-1]) {
      buf[i] = buf[i-1];
      if (x >= buf[0]) {
        unsigned k;
        for (k = i-2; x < buf[k]; --k)
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

// merge_sort_u64 - sort by merge sort
// nElem > 1
static void merge_sort_u64(uint64_t buf[], size_t nElem, uint64_t wrk[])
{
  if (nElem < MIN_MERGE_LEN) {
    short_section_sort_u64(buf, nElem);
    return;
  }

  size_t len01 = nElem / 2;
  size_t len0  = len01 / 2;
  merge_sort_u64(&buf[0], len0, &wrk[0]);
  size_t len1  = len01 - len0;
  merge_sort_u64(&buf[len0], len1, &wrk[0]);
  merge_u64(&wrk[0], &buf[0], len0, &buf[len0], len1);

  size_t len23 = nElem - len01;
  size_t len2  = len23 / 2;
  merge_sort_u64(&buf[len01], len2, &wrk[len01]);
  size_t len3  = len23 - len2;
  merge_sort_u64(&buf[len01+len2], len3, &wrk[len01]);
  merge_u64(&wrk[len01], &buf[len01], len2, &buf[len01+len2], len3);

  merge_u64(buf, &wrk[0], len01, &wrk[len01], len23);
}

static void merge_u64(uint64_t dst[], uint64_t src1[], size_t len1, uint64_t src2[], size_t len2)
{
  uint64_t* end1 = &src1[len1];
  uint64_t* end2 = &src2[len2];
  while ( src1 != end1 && src2 != end2) {
    len1 = end1 - src1;
    len2 = end2 - src2;
    size_t len = len1 <= len2 ? len1 : len2;
    uint64_t* dstend = &dst[len];
    do {
      if (*src1 <= *src2)
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

static void recursive_sort(uint64_t y[], uint32_t nElem, uint64_t wrk[], const char* x[], unsigned offs)
{
  const uint64_t VAL_MSK = (uint64_t)(-1) << 32;
  const uint64_t IX_MSK  = (uint64_t)(-1) >> 32;

  for (uint32_t i = 0; i < nElem; ++i) {
    uint32_t ix = y[i] & IX_MSK;
    y[i] = ((uint64_t)load4(x[ix]+offs) << 32) | ix;
  }

  merge_sort_u64(y, nElem, wrk);

  uint32_t i0 = 0;
  uint64_t val0 = y[0] & VAL_MSK;
  for (uint32_t i = 0; i < nElem; ++i) {
    if ((y[i] & VAL_MSK) != val0) {
      if (i - i0 > 1 && (uint8_t)(val0 >> 32) != 0)
        recursive_sort(&y[i0], i - i0, wrk, x, offs+4);
      val0 = y[i] & VAL_MSK;
      i0 = i;
    }
  }
  if (nElem - i0 > 1 && (uint8_t)(val0 >> 32) != 0)
    recursive_sort(&y[i0], nElem - i0, wrk, x, offs+4);
}
