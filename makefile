CC = clang
CPP = clang++
COPT = -Wall -O2

all: dfs-test-qsort.exe dfs-test-stdsort.exe dfs-test-stdsortr.exe dfs-test-mergesort.exe

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

