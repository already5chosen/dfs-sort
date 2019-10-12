CC = clang
CPP = clang++
COPT = -Wall -O2

all: dfs-test-qsort.exe

dfs-main.o: dfs-main.cpp dfs-sort.h
	${CPP} ${COPT} -c $<

dfs-qsort.o: dfs-qsort.c dfs-sort.h
	${CC} ${COPT} -c $<

dfs-test-qsort.exe : dfs-main.o dfs-qsort.o
	${CPP} $+ -o $@
