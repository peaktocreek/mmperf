// SPDX-License-Identifier: GPL-2.0
#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdbool.h>
#include "../kselftest.h"
#include <syscall.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include "./mm_perf.h"

#define NUM_TESTS 1000

static void test_munmap(int numVma)
{
	unsigned long mapflags = MAP_ANONYMOUS | MAP_PRIVATE;
	struct timespec start, end;
	struct timespec startT, endT;
	unsigned long page_size = getpagesize();
	unsigned long size;
	unsigned long long durations[NUM_TESTS];
	int currVma = 1;
	unsigned long long duration;
	void *ptr[1000];
	double mean, stddev, stderr;

	clock_gettime(CLOCK_MONOTONIC, &startT);
	for (int r = 0; r < NUM_TESTS; r++) {
		for (int i = 0; i < 1000; i++)
			ptr[i] = create_map(numVma);

		clock_gettime(CLOCK_MONOTONIC, &start);
		for (int i = 0; i < 1000; i++)
			sys_munmap(ptr[i], numVma * page_size);

		clock_gettime(CLOCK_MONOTONIC, &end);
		durations[r] = GET_DURATION(start, end);
	}

	calculate_stats(durations, NUM_TESTS, &mean, &stddev, &stderr);
	duration = GET_DURATION(startT, end);
	ksft_print_msg("vma=%d, munmap(ns): mean:%.f, stdDev:%.f, stdErr:%.f, test_duration:%.2f(s)\n",
		numVma, mean, stddev, stderr, (double) duration/1000000000L);
}

static void test_mprotect(int numVma)
{
	unsigned long mapflags = MAP_ANONYMOUS | MAP_PRIVATE;
	struct timespec start, end;
	struct timespec startT, endT;
	unsigned long page_size = getpagesize();
	unsigned long size;
	long long durations[NUM_TESTS];
	int currVma = 1;
	unsigned long long duration;
	void *ptr[1000];
	double mean, stddev, stderr;

	clock_gettime(CLOCK_MONOTONIC, &startT);
	for (int r = 0; r < NUM_TESTS; r++) {
		for (int i = 0; i < 1000; i++)
			ptr[i] = create_map(numVma);

		clock_gettime(CLOCK_MONOTONIC, &start);
		for (int i = 0; i < 1000; i++)
			sys_mprotect(ptr[i], numVma * page_size, PROT_READ);
		clock_gettime(CLOCK_MONOTONIC, &end);
		durations[r] = GET_DURATION(start, end);

		for (int i = 0; i < 1000; i++)
			sys_munmap(ptr[i], numVma * page_size);
	}

	calculate_stats(durations, NUM_TESTS, &mean, &stddev, &stderr);
	duration = GET_DURATION(startT, end);
	ksft_print_msg("vma=%d, mprotect(ns): mean:%.f, stdDev:%.f, stdErr:%.f, test_duration:%.2f(s)\n",
		numVma, mean, stddev, stderr, (double) duration/1000000000L);
}

static void test_madvise(int numVma)
{
	unsigned long mapflags = MAP_ANONYMOUS | MAP_PRIVATE;
	struct timespec start, end;
	struct timespec startT, endT;
	unsigned long page_size = getpagesize();
	unsigned long size;
	long long durations[NUM_TESTS];
	int currVma = 1;
	unsigned long long duration;
	void *ptr[1000];
	double mean, stddev, stderr;

	clock_gettime(CLOCK_MONOTONIC, &startT);
	for (int r = 0; r < NUM_TESTS; r++) {
		for (int i = 0; i < 1000; i++)
			ptr[i] = create_map(numVma);

		clock_gettime(CLOCK_MONOTONIC, &start);
		for (int i = 0; i < 1000; i++)
			sys_madvise(ptr[i], numVma * page_size, MADV_DONTNEED);
		clock_gettime(CLOCK_MONOTONIC, &end);
		durations[r] = GET_DURATION(start, end);

		for (int i = 0; i < 1000; i++)
			sys_munmap(ptr[i], numVma * page_size);
	}
	calculate_stats(durations, NUM_TESTS, &mean, &stddev, &stderr);
	duration = GET_DURATION(startT, end);
	ksft_print_msg("vma=%d, madvise(ns): mean:%.f, stdDev:%.f, stdErr:%.f, test_duration:%.2f(s)\n",
		numVma, mean, stddev, stderr, (double) duration/1000000000L);
}

int main(int argc, char **argv)
{
	ksft_print_header();
	ksft_print_msg("mm_perf2, pid=%d\n", getpid());

	//ksft_set_plan(1);

	test_munmap(1);
	test_munmap(2);
	test_munmap(4);
	test_munmap(8);
	test_munmap(16);
	test_munmap(32);

	test_mprotect(1);
	test_mprotect(2);
	test_mprotect(4);
	test_mprotect(8);
	test_mprotect(16);
	test_mprotect(32);

	test_madvise(1);
	test_madvise(2);
	test_madvise(4);
	test_madvise(8);
	test_madvise(16);
	test_madvise(32);

	//ksft_finished();
	return 0;
}
