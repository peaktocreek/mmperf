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
#include <linux/perf_event.h>
#include "./mm_perf.h"

#define NUM_TESTS 1000

inline int sys_perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
	int cpu, int group_fd, unsigned long flags)
{
	int sret;

	errno = 0;
	sret = syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
	return sret;
}


void perf_start(struct perf_event_attr *pe, int *fd)
{
	memset(pe, 0, sizeof(struct perf_event_attr));
	pe->type = PERF_TYPE_HARDWARE;
	pe->size = sizeof(struct perf_event_attr);
	pe->config = PERF_COUNT_HW_REF_CPU_CYCLES;
	pe->disabled = 1;
	pe->exclude_user = 1;
	*fd = sys_perf_event_open(pe, 0, -1, -1, 0);
	assert(*fd != -1);

	ioctl(*fd, PERF_EVENT_IOC_RESET, 0);
	ioctl(*fd, PERF_EVENT_IOC_ENABLE, 0);
}

void perf_stop(struct perf_event_attr *pe, int fd, long long *count)
{
	ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
	read(fd, count, sizeof(long long));
	close(fd);
}

static void test_munmap(int numVma)
{
	unsigned long mapflags = MAP_ANONYMOUS | MAP_PRIVATE;
	struct timespec startT, endT;
	unsigned long page_size = getpagesize();
	unsigned long size;
	unsigned long long counts[NUM_TESTS];
	int currVma = 1;
	unsigned long long duration;
	void *ptr[1000];
	struct perf_event_attr pe;
	int fd;
	long long count;
	double mean, stddev, stderr;

	clock_gettime(CLOCK_MONOTONIC, &startT);
	for (int r = 0; r < NUM_TESTS; r++) {
		for (int i = 0; i < 1000; i++)
			ptr[i] = create_map(numVma);

		perf_start(&pe, &fd);
		for (int i = 0; i < 1000; i++)
			sys_munmap(ptr[i], numVma * page_size);
		perf_stop(&pe, fd, &count);
		counts[r] = count;
	}
	clock_gettime(CLOCK_MONOTONIC, &endT);
	calculate_stats(counts, NUM_TESTS, &mean, &stddev, &stderr);
	duration = GET_DURATION(startT, endT);
	ksft_print_msg("vma=%d, munmap: mean:%.f, stdDev:%.f, stdErr:%.f, test_duration:%.2f(s)\n",
		numVma, mean, stddev, stderr, (double) duration/1000000000L);
}

static void test_mprotect(int numVma)
{
	unsigned long mapflags = MAP_ANONYMOUS | MAP_PRIVATE;
	struct timespec startT, endT;
	unsigned long page_size = getpagesize();
	unsigned long size;
	unsigned long long counts[NUM_TESTS];
	int currVma = 1;
	unsigned long long duration;
	void *ptr[1000];
	struct perf_event_attr pe;
	int fd;
	long long count;
	double mean, stddev, stderr;

	clock_gettime(CLOCK_MONOTONIC, &startT);
	for (int r = 0; r < NUM_TESTS; r++) {
		for (int i = 0; i < 1000; i++)
			ptr[i] = create_map(numVma);

		perf_start(&pe, &fd);
		for (int i = 0; i < 1000; i++)
			sys_mprotect(ptr[i], numVma * page_size, PROT_READ|PROT_WRITE);
		perf_stop(&pe, fd, &count);
		counts[r] = count;

		for (int i = 0; i < 1000; i++)
			sys_munmap(ptr[i], numVma * page_size);
	}
	clock_gettime(CLOCK_MONOTONIC, &endT);
	calculate_stats(counts, NUM_TESTS, &mean, &stddev, &stderr);
	duration = GET_DURATION(startT, endT);
	ksft_print_msg("vma=%d, mprotect: mean:%.f, stdDev:%.f, stdErr:%.f, test_duration:%.2f(s)\n",
		numVma, mean, stddev, stderr, (double) duration/1000000000L);
}

static void test_madvise(int numVma)
{
	unsigned long mapflags = MAP_ANONYMOUS | MAP_PRIVATE;
	struct timespec startT, endT;
	unsigned long page_size = getpagesize();
	unsigned long size;
	unsigned long long counts[NUM_TESTS];
	int currVma = 1;
	unsigned long long duration;
	void *ptr[1000];
	struct perf_event_attr pe;
	int fd;
	long long count;
	double mean, stddev, stderr;

	clock_gettime(CLOCK_MONOTONIC, &startT);
	for (int r = 0; r < NUM_TESTS; r++) {
		for (int i = 0; i < 1000; i++)
			ptr[i] = create_map(numVma);

		perf_start(&pe, &fd);
		for (int i = 0; i < 1000; i++)
			sys_madvise(ptr[i], numVma * page_size, MADV_DONTNEED);
		perf_stop(&pe, fd, &count);
		counts[r] = count;

		for (int i = 0; i < 1000; i++)
			sys_munmap(ptr[i], numVma * page_size);
	}
	clock_gettime(CLOCK_MONOTONIC, &endT);

	calculate_stats(counts, NUM_TESTS, &mean, &stddev, &stderr);
	duration = GET_DURATION(startT, endT);
	ksft_print_msg("vma=%d, madvise: mean:%.f, stdDev:%.f, stdErr:%.f, test_duration:%.2f(s)\n",
		numVma, mean, stddev, stderr, (double) duration/1000000000L);
}

int main(int argc, char **argv)
{
	ksft_print_header();
	ksft_print_msg("mm_perf3, pid=%d\n", getpid());

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
