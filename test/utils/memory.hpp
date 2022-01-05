#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>

namespace Memory {
	bool Protect(void *addr, size_t size, int prot);
	void *FindPattern(const char *pattern, const char *mask, void *start, size_t size);
	template <typename T>
	inline T FindPattern(const char *pattern, const char *mask, void *start, size_t size)
	{
		return reinterpret_cast<T>(FindPattern(pattern, mask, start, size));
	}
}

#endif
