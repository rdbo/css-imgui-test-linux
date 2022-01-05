#include "memory.hpp"

bool Memory::Protect(void *addr, size_t size, int prot)
{
	static uintptr_t pagesize = static_cast<uintptr_t>(sysconf(_SC_PAGE_SIZE));
	void *aligned_addr = reinterpret_cast<void *>(
		reinterpret_cast<uintptr_t>(addr) & -pagesize
	);
	size_t diff = reinterpret_cast<size_t>(
		reinterpret_cast<uintptr_t>(addr) - reinterpret_cast<uintptr_t>(aligned_addr)
	);
	size += diff;
	return !mprotect(aligned_addr, size, prot);
}

void *Memory::FindPattern(const char *pattern, const char *mask, void *start, size_t size)
{
	void *result = NULL;
	size_t pattern_size = strlen(mask);
	if (size < pattern_size)
		return result;

	size_t j;
	for (size_t i = 0; i < size - pattern_size; ++i) {
		for (j = 0; j < pattern_size; ++j) {
			char curbyte = reinterpret_cast<char *>(start)[i + j];
			if (curbyte != pattern[j] && mask[j] == 'x')
				break;
		}

		if (j == pattern_size) {
			result = reinterpret_cast<void *>(
				&(reinterpret_cast<char *>(start))[i]
			);

			break;
		}
	}

	return result;
}
