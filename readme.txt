Play with sorting of short asciiz character strings

Inspired by comp.lang discussion: https://groups.google.com/forum/#!topic/comp.lang.c/sFya11lqLDA

A list of tested algorithms

1. dfs-qsort.c
Quicksort algorithm implemented C RTL function qsort.
Comparison function is based on strcasecmp().

2. dfs-stdsort.cpp
Quicksort algorithm implemented C++ STL template std::sort
Comparison function is based on strcasecmp().

3. dfs-mergesort.c
Merge sort algorithm.
Comparison function is based on strcasecmp().

4. dfs-stdsortr.cpp
Combination of radix sort and quicksort.
Quicksort (std::sort<uint64_t>) is used as inner engine.
Radix sort by most-significant digit first and radix == 2**32 serves as outer algorithm.
The data is preprocessed during copy to temporary buffers - converted to lower case in chunks of 4 characters.
Comparison function is a 64-bit unsigned comparison.

5. dfs-mergesortr.c
Combination of radix sort and merge sort.
Merge sort is used as inner engine.
Radix sort by most-significant digit first and radix == 2**32 serves as outer algorithm.
The data is preprocessed during copy to temporary buffers - converted to lower case in chunks of 4 characters.
Comparison function is a 64-bit unsigned comparison.

6. dfs-radix4sort.c
Combination of radix sort 'MS digit first' and radix sort 'LS digit first'.
Outer algorithm is a recursive 'MS digit first' variant of radix sort that treats a group
of 4 8-bit characters as a single super-digit.
Inner sorting within a super-digits is done by 'LS digit first' variant of radix sort.
Very short sections are sorted by straight insertion sort.
The data is preprocessed during copy to temporary buffers - converted to lower case in chunks of 4 characters.
Comparison function: N/A. The sorting is based on counting rather than comparison.

7. dfs-radix2sort.c
Combination of radix sort 'MS digit first' and radix sort 'LS digit first'.
Outer algorithm is a recursive 'MS digit first' variant of radix sort that treats a group
of 2 8-bit characters as a single super-digit.
Inner sorting within a super-digits is done by 'LS digit first' variant of radix sort.
Very short sections are sorted by straight insertion sort.
The data is preprocessed during copy to temporary buffers - converted to lower case in chunks of 4 characters.
Comparison function: N/A. The sorting is based on counting rather than comparison.

8. dfs-radix1sort.c
Recursive radix sort 'MS digit first' with individual characters as digits.
Very short sections are sorted by straight insertion sort.
The data is preprocessed during copy to temporary buffers - converted to lower case in chunks of 4 characters.
Comparison function: N/A. The sorting is based on counting rather than comparison.
This function is not yet optimized for maximal possible performance. In particular, it does many copy operations
that could be avoided with slightly more complex housekeeping.

Speed on my test case (~10M words taken from classic English-language novels):

dfs-test-qsort.exe      : 6.211450 s
dfs-test-stdsort.exe    : 5.226997 s
dfs-test-mergesort.exe  : 4.839399 s
dfs-test-stdsortr.exe   : 1.624570 s
dfs-test-mergesortr.exe : 1.765105 s
dfs-test-radix4sort.exe : 0.850378 s
dfs-test-radix2sort.exe : 0.846078 s
dfs-test-radix1sort.exe : 0.882004 s
