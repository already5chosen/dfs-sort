// Implement lexicographic case-insensitive sort as 'MS digit first' variant of radix sort
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

static void recursive_sort(uint64_t* restrict y, uint32_t nElem, const char* restrict x[], unsigned offs, uint64_t* restrict wrk);

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
  recursive_sort(y, nElem, x, 0, wrkbuf + nElem);

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

// radix_sort - sort by radix sort, only 16 specified bits of 64-bit words are taken into account
// nElem > 1
// when lsw == 0 - sort by bits [63:56]
// when lsw == 1 - sort by bits [55:48]
// when lsw == 2 - sort by bits [47:40]
// when lsw == 3 - sort by bits [39:32]
static void radix_sort(uint64_t* restrict src, size_t nElem, int lsw, uint64_t* restrict dst, uint32_t* restrict h)
{
  // calculate histograms and copy buf to wrk
  int sh = (7 - lsw)*8;
  for (uint32_t i = 0; i != nElem; ++i) {
    uint64_t x = src[i];
    ++h[(x >> sh) & 0xFF];
  }

  // integrate histogram
  uint32_t acc = 0;
  for (unsigned i = 0; acc != nElem; ++i) {
    uint32_t hval = h[i];
    h[i] = acc;
    acc += hval;
  }

  // sort
  for (uint32_t i = 0; i != nElem; ++i) {
    uint64_t x = src[i];
    unsigned ch = (x >> sh) & 0xFF;
    uint32_t ix = h[ch];
    dst[ix] = x;
    h[ch] = ix + 1;
  }
}

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

static void recursive_sort_load4(uint64_t y[], uint32_t nElem, const char* restrict x[], unsigned offs)
{
  const uint64_t IX_MSK  = (uint64_t)(-1) >> 32;
  if ((offs & 3)==0) {
    // all 4 loaded characters are sorted, load next portion
    if (offs == 0) {
      for (uint32_t i = 0; i < nElem; ++i) {
        y[i] = ((uint64_t)load4(x[i]) << 32) | i;
      }
    } else {
      for (uint32_t i = 0; i < nElem; ++i) {
        uint32_t ix = y[i] & IX_MSK;
        y[i] = ((uint64_t)load4(x[ix]+offs) << 32) | ix;
      }
    }
  }
}

static void recursive_sort_from_x(uint64_t src[], uint32_t nElem, const char* restrict x[], uint64_t dst[])
{ // replace fully sorted with sorted original vector
  const uint64_t IX_MSK  = (uint64_t)(-1) >> 32;
  for (uint32_t i = 0; i < nElem; ++i) {
    uint32_t ix = src[i] & IX_MSK;
    dst[i] = (uint64_t)x[ix];
  }
}

static void recursive_sort_short(uint64_t* restrict y, uint32_t nElem, const char* restrict x[], unsigned offs)
{
  for (;;) {
    recursive_sort_load4(y, nElem, x, offs);
    // sort by 4 characters
    short_section_sort_u64(y, nElem);
    offs += 4 - (offs & 3); // next 4-character element

    // sort sub-sections that have identical prefix
    const uint64_t VAL_MSK = (uint64_t)(-1)   << 32;
    const uint64_t LSB_MSK = (uint64_t)(0xFF) << 32;
    uint64_t valLast = y[nElem-1] & VAL_MSK;
    const uint32_t hnElem = nElem /2;
    uint32_t big_i0 = 0;
    uint32_t big_len = 0;
    for (uint32_t i0 = 0; i0 != nElem;) {
      uint64_t val0 = y[i0] & VAL_MSK;
      uint32_t i1 = nElem;
      if (val0 != valLast) {
        i1 = i0 + 1;
        while ((y[i1] & VAL_MSK) == val0) ++i1;
      }
      uint32_t len = i1 - i0;
      if (len > 1 && (val0 & LSB_MSK) != 0) { // sort by next characters
        if (len <= hnElem)
          recursive_sort_short(&y[i0], len, x, offs);
        else { // don't process very big sections here
          big_i0  = i0;
          big_len = len;
        }
      }
      i0 = i1;
    }
    if (big_len == 0)
      break;

    // Handle very big sections in non-recursive manner
    // It's done in order to limit a maximal recursion depth to ceil(log2(nElem))
    y += big_i0;
    nElem = big_len;
  }
}

static void recursive_sort(uint64_t* restrict y, uint32_t nElem, const char* restrict x[], unsigned offs, uint64_t* restrict wrk)
{
  for (;;) {
    uint64_t* src = (offs & 1)==0 ? y   : wrk;
    uint64_t* dst = (offs & 1)==0 ? wrk : y;
    if (nElem < MIN_RADIXSORT_LEN) {
      recursive_sort_short(src,  nElem, x, offs);
      recursive_sort_from_x(src, nElem, x, y);
      break;
    }

    recursive_sort_load4(y, nElem, x, offs);

    // sort by 1 character
    uint32_t h[256] = {0};
    radix_sort(src, nElem, offs & 3, dst, h);
    offs += 1;

    // sort sub-sections that have identical prefix
    if (h[0] > 0) // the section 0 is fully sorted, replace with sorted original vector
      recursive_sort_from_x(dst, h[0], x, y);

    const uint32_t hnElem = nElem /2;
    const uint64_t IX_MSK  = (uint64_t)(-1) >> 32;
    unsigned big_ci = 0;
    for (unsigned ci = 0; h[ci] != nElem; ++ci) {
      uint32_t i0 = h[ci];
      uint32_t len = h[ci+1] - i0;
      if (len != 0) {
        if (len > 1) {
          // sort by next characters
          if (len <= hnElem)
            recursive_sort(&y[i0], len, x, offs, &wrk[i0]);
          else
            big_ci = ci+1; // don't process very big sections here
        } else { // the section is fully sorted, replace with sorted original vector
          y[i0] = (uint64_t)x[dst[i0] & IX_MSK];
        }
      }
    }

    if (big_ci == 0)
      break;

    // Handle very big sections in non-recursive manner
    // It's done in order to limit a maximal recursion depth to ceil(log2(nElem))
    uint32_t i0 = h[big_ci-1];
    y   += i0;
    wrk += i0;
    nElem = h[big_ci] - i0;
  }
}
