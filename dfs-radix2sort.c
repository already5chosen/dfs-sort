// Implement lexicographic case-insensitive sort as radix sort
// Outer algorithm is a recursive 'MS digit first' variant of radix sort that
// treats a group of 2 8-bit characters as a single super-digit.
// Inner sorting within a super-digits is done by 'LS digit first' radix sort.
// Very short sections are sorted by straight insertion sort.
//
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <memory.h>
#include "dfs-sort.h"

enum {
  MIN_RADIXSORT_LEN = 42,
};

static void short_section_sort_u64(uint64_t buf[], unsigned nElem);
static void radix2_sort(uint64_t buf[], size_t nElem, uint64_t wrk[], int lsw);
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

  recursive_sort(y, nElem, wrkbuf + nElem, x, 0);

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

// radix2_sort - sort by radix sort, only 16 specified bits of 64-bit words are taken into account
// nElem > 1
// when lsw == 0 - sort by bits [63:48] otherwise  sort by bits [47:32]
static void radix2_sort(uint64_t buf[], size_t nElem, uint64_t wrk[], int lsw)
{
  if (nElem < MIN_RADIXSORT_LEN) {
    short_section_sort_u64(buf, nElem);
    return;
  }

  // calculate histograms
  uint32_t h[2][256] = {{0}};
  if (lsw) {
    for (uint32_t i = 0; i != nElem; ++i) {
      uint32_t x = (uint32_t)(buf[i] >> 32);
      ++h[1][x & 0xFF]; x >>= 8;
      ++h[0][x & 0xFF];
    }
  } else {
    for (uint32_t i = 0; i != nElem; ++i) {
      uint32_t x = (uint32_t)(buf[i] >> 48);
      ++h[1][x & 0xFF]; x >>= 8;
      ++h[0][x];
    }
  }

  // sort using histogram
  for (int k = 1; k >= 0; --k) {
    // sort by k-th digit
    uint32_t* hk = h[k];
    // integrate histogram
    uint32_t acc = 0;
    for (unsigned i = 0; i < 256; ++i) {
      uint32_t hval = hk[i];
      hk[i] = acc;
      acc += hval;
    }
    // sort
    int sh = lsw ? 32 + (1-k)*8 : 48 + (1-k)*8;
    uint64_t* src = k ? buf : wrk;
    uint64_t* dst = k ? wrk : buf;
    for (uint32_t i = 0; i != nElem; ++i) {
      uint64_t x = src[i];
      unsigned ch = (x >> sh) & 0xFF;
      uint32_t ix = hk[ch];
      dst[ix] = x;
      hk[ch] = ix + 1;
    }
  }
}

static void recursive_sort(uint64_t y[], uint32_t nElem, uint64_t wrk[], const char* x[], unsigned offs)
{
  // sort by 4 MS characters
  radix2_sort(y, nElem, wrk, offs & 2);
  // sort sub-sections that have identical prefix
  const uint64_t IX_MSK  = (uint64_t)(-1) >> 32;
  const uint64_t VAL_MSK = offs & 2 ? (uint64_t)(0xFFFF) << 32 : (uint64_t)(0xFFFF) << 48;
  const uint64_t LSB_MSK = offs & 2 ? (uint64_t)(0x00FF) << 32 : (uint64_t)(0x00FF) << 48;
  uint64_t valLast = y[nElem-1] & VAL_MSK;
  for (uint32_t i0 = 0; i0 != nElem;) {
    uint64_t val0 = y[i0] & VAL_MSK;
    uint32_t i1 = nElem;
    if (val0 != valLast) {
      i1 = i0 + 1;
      while ((y[i1] & VAL_MSK) == val0) ++i1;
    }
    if (i1 - i0 > 1 && (val0 & LSB_MSK) != 0) {
      if (offs & 2) {
        // load 4 more characters
        for (uint32_t i = i0; i < i1; ++i) {
          uint32_t ix = y[i] & IX_MSK;
          y[i] = ((uint64_t)load4(x[ix]+offs+2) << 32) | ix;
        }
      }
      recursive_sort(&y[i0], i1 - i0, wrk, x, offs+2);
    } else {
      for (uint32_t i = i0; i < i1; ++i) {
        uint32_t ix = y[i] & IX_MSK;
        y[i] = (uint64_t)x[ix];
      }
    }
    i0 = i1;
  }
}
