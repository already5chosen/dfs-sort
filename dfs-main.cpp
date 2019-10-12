#define  VALIDATE_BY_MAP 0
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <vector>
#include <chrono>
#if  VALIDATE_BY_MAP
#include <unordered_map>
#endif

extern "C" {
#include "dfs-sort.h"
}

int main(int argz, char** argv)
{
  if (argz < 3) {
    fprintf(stderr,
      "dfs-test\n"
      "Usage:\n"
      "dfs-test input-file-name output-file-name [-n]\n"
      "where\n"
      "input-file-name - name of the ASCII text file to be sorted in ascending order by case-insensitive lexicographic sort\n"
      "output-file-name - name of result file\n"
      "-n               - flag that disables validation of the sorting algorithm\n"
      );
    return 1;
  }

  const char* inpname = argv[1];
  const char* outname = argv[2];
  bool validate = !((argz > 3) && (strcmp(argv[3], "-n")==0));

  struct stat sb;
  if (stat(inpname, &sb) !=0) {
    perror(inpname);
    return 1;
  }
  // printf("%lld\n", sb.st_size);
  const long long MAX_INP_SZ = 500000000;
  if (sb.st_size > MAX_INP_SZ) {
    fprintf(stderr
      ,"size of input file %s = %lld bytes exeeds supported maximum of %lld bytes. Sorry.\n"
      ,inpname, (long long)sb.st_size, MAX_INP_SZ);
    return 1;
  }

  if (sb.st_size == 0) {
    fprintf(stderr, "input file %s is empty\n", inpname);
    return 1;
  }

  FILE* fpinp = fopen(inpname, "rb");
  if (!fpinp) {
    perror(inpname);
    return 1;
  }

  size_t inpfilesize = sb.st_size;
  std::vector<char> inpbuf(inpfilesize+1);
  size_t inpsize = fread(inpbuf.data(), 1, inpfilesize, fpinp);
  if (inpsize != inpfilesize) {
    perror(inpname);
    fclose(fpinp);
    return 1;
  }
  fclose(fpinp);

  if (inpbuf[inpsize-1] != '\n') {
    // append line terminator in case last line is not terminated
    inpbuf[inpsize] = '\n';
    ++inpsize;
  }

  std::vector<const char*> inparr;
  size_t beg = 0;
  for (size_t i = 0; ; ++i) {
    char c = inpbuf[i];
    switch (c) {
      case '\n':
      {
        const size_t MAX_LINES = 50000000;
        if (inparr.size() == MAX_LINES) {
          fprintf(stderr
            ,"input file %s contains more than %llu lines. Unsupported. Sorry.\n"
            ,inpname, MAX_LINES);
          return 1;
        }
        inpbuf[i] = 0; // replace EOL with zero
        inparr.push_back(&inpbuf[beg]);
        beg = i + 1;
        if (beg == inpsize)
          goto inp_parsing_done;
      } break;

      case 0:
        inpbuf[i] = ' '; // replace zero with space
        break;

      default:
        break;
    }
  }
  inp_parsing_done:;

  // printf("%zu %zu\n", inpbuf.size()-1, inparr.size());

  // Prepare for validation
  std::vector<char> outbuf(inpsize);
  #if  VALIDATE_BY_MAP
  std::unordered_map<const char*, bool> inpMap;
  #endif
  if (validate) {
    #if  VALIDATE_BY_MAP
    inpMap.reserve(inparr.size());
    for (auto it = inparr.begin(); it != inparr.end(); ++it)
      inpMap.insert(std::pair<const char*, bool>(*it,false));
    #else
    outbuf.resize(inpsize);
    memset(outbuf.data(), 0, inpsize);
    for (auto it = inparr.begin(); it != inparr.end(); ++it)
      outbuf[*it - inpbuf.data()] = 1;
    #endif
  }

  // sort
  std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
  if (!dfs_sort(inparr.data(), inparr.size())) {
    fprintf(stderr, "dfs_sort failed. Memory allocation problem or unsupported number of elements (%zu).\n", inparr.size());
    return 1;
  }
  std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
  printf("%.6f s\n",  std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() *1e-6);

  if (validate) {
    const char** arr = inparr.data();
    size_t len = inparr.size();
    // check that result array contains exactly the same elements as original array
    for (size_t i = 0; i < len; ++i) {
      #if  VALIDATE_BY_MAP
      auto mapIt = inpMap.find(arr[i]);
      if (mapIt == inpMap.end()) {
        fprintf(stderr
          ,"Validation failed at element %zu. Pointer %p does not present in input vector.\n"
          , i
          , arr[i]
        );
        return 1;
      }
      if (mapIt->second) {
        fprintf(stderr
          ,"Validation failed at element %zu. Pointer %p appears twice.\n"
          , i
          , arr[i]
        );
        return 1;
      }
      mapIt->second = true;
      #else
      const char* str = arr[i];
      if (str < inpbuf.data() || str >= inpbuf.data()+inpsize) {
        fprintf(stderr
          ,"Validation failed at element %zu. Pointer %p does not point into range [%p..%p].\n"
          , i
          , str
          , inpbuf.data()
          , inpbuf.data()+inpsize+1
        );
        return 1;
      }
      // str points into inpbuf
      size_t ix = str - inpbuf.data();
      if (outbuf[ix] != 1) {
        if (outbuf[ix] == 0)
          fprintf(stderr
            ,"Validation failed at element %zu. Pointer %p does not point to the middle input string.\n"
            , i
            , str
          );
        else
          fprintf(stderr
            ,"Validation failed at element %zu. Pointer %p appears twice.\n"
            , i
            , str
          );
        return 1;
      }
      outbuf[ix] = 2;
      #endif
    }

    // check that elements of result array are sorted
    for (size_t i = 0; i < len-1; ++i) {
      if (strcasecmp(arr[i], arr[i+1]) > 0) {
        fprintf(stderr
          ,"Validation failed at element %zu. Pointers %p and %p appear in wrong order.\n"
          , i
          , arr[i]
          , arr[i+1]
        );
        return 1;
      }
    }
  }
  // printf("o.k.\n");

  // flatten sorted array into output buffer
  outbuf.resize(inpsize);
  char* pout = outbuf.data();
  for (auto it = inparr.begin(); it != inparr.end(); ++it) {
    const char* str = *it;
    size_t l = strlen(str);
    if (l > 0) {
      memcpy(pout, str, l);
      pout += l;
    }
    *pout++ = '\n';
  }

  FILE* fpout = fopen(outname, "wb");
  if (!fpout) {
    perror(outname);
    return 1;
  }

  size_t outsize = fwrite(outbuf.data(), 1, inpsize, fpout);
  bool succ = (outsize == inpsize);
  if (!succ)
    perror(outname);
  if (succ) {
    succ = (fflush(fpout)==0);
    if (!succ)
      perror(outname);
  }
  fclose(fpout);

  if (!succ)
    remove(outname);

  return succ ? 0 : 1;
}
