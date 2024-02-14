/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _MM_PERF_H
#define _MM_PERF_H

#define GET_DURATION(start, end) \
	((end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec))

inline void *sys_mmap(void *addr, unsigned long len, unsigned long prot,
	unsigned long flags, unsigned long fd, unsigned long offset)
{
	void *sret;

	errno = 0;
	sret = (void *) syscall(__NR_mmap, addr, len, prot,
		flags, fd, offset);
	return sret;
}

inline int sys_munmap(void *ptr, size_t size)
{
	int sret;

	errno = 0;
	sret = syscall(__NR_munmap, ptr, size);
	return sret;
}

inline int sys_madvise(void *start, size_t len, int types)
{
	int sret;

	errno = 0;
	sret = syscall(__NR_madvise, start, len, types);
	return sret;
}

inline int sys_mprotect(void *ptr, size_t size, unsigned long prot)
{
	int sret;

	errno = 0;
	sret = syscall(__NR_mprotect, ptr, size, prot);
	return sret;
}

inline int sys_mprotect_pkey(void *ptr, size_t size, unsigned long orig_prot,
		unsigned long pkey)
{
	int sret;

	errno = 0;
	sret = syscall(__NR_pkey_mprotect, ptr, size, orig_prot, pkey);
	return sret;
}

inline int sys_mlock(void *ptr, size_t size)
{
	int sret;

	errno = 0;
	sret = syscall(__NR_mlock, ptr, size);
	return sret;
}

inline void *create_map(unsigned int numVma)
{
	void *ptr;
	unsigned long page_size = getpagesize();

	ptr = sys_mmap(NULL, numVma * page_size, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	assert(ptr != (void *)-1);

	for (int j = 0; j < numVma; j++) {
		if (j%2 == 0)
			sys_mprotect(ptr + j * page_size, page_size, PROT_NONE);
	}

	return ptr;
}

static inline void calculate_stats(long long counts[], int n,
	double *mean, double *stddev, double *stderr)
{
	double sum = 0;
	double variance = 0;

	for (int i = 0; i < n; i++)
		sum += counts[i];
	*mean = sum / n;

	for (int i = 0; i < n; i++)
		variance += pow(counts[i] - *mean, 2);

	variance /= n;
	*stddev = sqrt(variance);
	*stderr = *stddev / sqrt(n);
}

#endif
