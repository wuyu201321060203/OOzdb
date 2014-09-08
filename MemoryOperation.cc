#include "Config.h"

#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "MemoryException.h"


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

void *Mem_alloc(long size, const char *func, const char *file, int line){
	assert(size > 0);
	void *p = malloc(size);
	if(!p)
		ExceptionThrow(MemoryException() , func, file, line, "%s", System_getLastError());
	return p;
}


void *Mem_calloc(long count, long size, const char *func, const char *file, int line) {
	assert(count > 0);
	assert(size > 0);
	void *p = calloc(count, size);
	if (! p)
		ExceptionThrow(MemoryException(), func, file, line, "%s", System_getLastError());
	return p;
}


void Mem_free(void *p, const char *func, const char *file, int line) {
	if (p)
		free(p);
}


void *Mem_resize(void *p, long size, const char *func, const char *file, int line) {
	assert(p);
	assert(size > 0);
	p = realloc(p, size);
	if (! p)
		ExceptionThrow(MemoryException(), func, file, line, "%s", System_getLastError());
	return p;
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif
