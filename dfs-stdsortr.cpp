#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cctype>
#include <algorithm>
extern "C" {
#include "dfs-sort.h"
}

// Implement lexicographic case-insensitive sort as combination of radix sort and quicksort.
// quicksort is used as inner engine, radix sort by most-significant digit first and radix = 2**32
// serves as outer algorithm


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
    (uint32_t(uint8_t(c0))<<24)|
    (uint32_t(uint8_t(c1))<<16)|
    (uint32_t(uint8_t(c2))<< 8)|
     uint32_t(uint8_t(c3));
}

static void recursive_sort(uint64_t* y, uint32_t nElem, const char* x[], unsigned offs)
{
  const uint64_t VAL_MSK = uint64_t(-1) << 32;
  const uint64_t IX_MSK  = uint64_t(-1) >> 32;

  for (uint32_t i = 0; i < nElem; ++i) {
    uint32_t ix = y[i] & IX_MSK;
    y[i] = (uint64_t(load4(x[ix]+offs)) << 32) | ix;
  }

  std::sort(y, y+nElem);

  uint32_t i0 = 0;
  uint64_t val0 = y[0] & VAL_MSK;
  for (uint32_t i = 0; i < nElem; ++i) {
    if ((y[i] & VAL_MSK) != val0) {
      if (i - i0 > 1 && uint8_t(val0 >> 32) != 0)
        recursive_sort(&y[i0], i - i0, x, offs+4);
      val0 = y[i] & VAL_MSK;
      i0 = i;
    }
  }
  if (nElem - i0 > 1 && uint8_t(val0 >> 32) != 0)
    recursive_sort(&y[i0], nElem - i0, x, offs+4);
}

int dfs_sort(const char* x[], size_t nElem)
{
  if (sizeof(uintptr_t) > sizeof(uint64_t))
    return 0; // platforms, on each pointer is bigger than 64 bits are not supported

  if (nElem > UINT32_MAX)
    return 0; // number of elements that does not fit in 32-bit word is not supported

  if (nElem <= 1)
    return 1; // success

  uint64_t* y = new uint64_t[nElem];
  for (uint32_t i = 0; i < uint32_t(nElem); ++i)
    y[i] = (uint64_t(load4(x[i])) << 32) | i;

  std::sort(y, y+nElem);

  const uint64_t VAL_MSK = uint64_t(-1) << 32;
  uint64_t valLast = y[nElem-1] & VAL_MSK;
  for (uint32_t i0 = 0; i0 != uint32_t(nElem);) {
    uint64_t val0 = y[i0] & VAL_MSK;
    uint32_t i1 = nElem;
    if (val0 != valLast) {
      i1 = i0 + 1;
      while ((y[i1] & VAL_MSK) == val0) ++i1;
    }
    if (i1 - i0 > 1 && uint8_t(val0 >> 32) != 0)
      recursive_sort(&y[i0], i1 - i0, x, 4);
    for (uint32_t k = i0; k < i1; ++k)
      y[k] = uint64_t(x[uint32_t(y[k])]);
    i0 = i1;
  }

  for (uint32_t i = 0; i < uint32_t(nElem); ++i)
    x[i] = (const char*)(y[i]);

  delete [] y;
  return 1; // success
}
