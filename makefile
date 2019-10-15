CC = clang
CPP = clang++
COPT = -Wall -O2

ALLEXE := dfs-test-qsort.exe
ALLEXE += dfs-test-stdsort.exe
ALLEXE += dfs-test-stdsortr.exe
ALLEXE += dfs-test-mergesort.exe
ALLEXE += dfs-test-mergesortr.exe
ALLEXE += dfs-test-radix4sort.exe
ALLEXE += dfs-test-radix2sort.exe

all: ${ALLEXE}

dfs-main.o: dfs-main.cpp dfs-sort.h
	${CPP} ${COPT} -c $<

dfs-qsort.o: dfs-qsort.c dfs-sort.h
	${CC} ${COPT} -c $<

dfs-test-qsort.exe : dfs-main.o dfs-qsort.o
	${CPP} $+ -o $@

dfs-stdsort.o: dfs-stdsort.cpp dfs-sort.h
	${CPP} ${COPT} -c $<

dfs-test-stdsort.exe : dfs-main.o dfs-stdsort.o
	${CPP} $+ -o $@

dfs-stdsortr.o: dfs-stdsortr.cpp dfs-sort.h
	${CPP} ${COPT} -c $<

dfs-test-stdsortr.exe : dfs-main.o dfs-stdsortr.o
	${CPP} $+ -o $@

dfs-mergesort.o: dfs-mergesort.c dfs-sort.h
	${CC} ${COPT} -c $<

dfs-test-mergesort.exe : dfs-main.o dfs-mergesort.o
	${CPP} $+ -o $@

dfs-mergesortr.o: dfs-mergesortr.c dfs-sort.h
	${CC} ${COPT} -c $<

dfs-test-mergesortr.exe : dfs-main.o dfs-mergesortr.o
	${CPP} $+ -o $@

dfs-radix4sort.o: dfs-radix4sort.c dfs-sort.h
	${CC} ${COPT} -c $<

dfs-test-radix4sort.exe : dfs-main.o dfs-radix4sort.o
	${CPP} $+ -o $@

dfs-radix2sort.o: dfs-radix2sort.c dfs-sort.h
	${CC} ${COPT} -c $<

dfs-test-radix2sort.exe : dfs-main.o dfs-radix2sort.o
	${CPP} $+ -o $@

