/*
 * Copyright (c) 2018 Jeff Boody
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef cc_memory_H
#define cc_memory_H

#ifdef MEMORY_DEBUG
void* cc_malloc_debug(const char* func, int line,
                      size_t size);
void* cc_calloc_debug(const char* func, int line,
                      size_t count, size_t size);
void* cc_realloc_debug(const char* func, int line,
                       void* ptr, size_t size);
void  cc_free_debug(const char* func, int line, void* ptr);
int   cc_memcheckptr_debug(const char* func, int line, void* ptr);
void  cc_meminfo_debug(void);
#endif

void*  cc_malloc(size_t size);
void*  cc_calloc(size_t count, size_t size);
void*  cc_realloc(void* ptr, size_t size);
void   cc_free(void* ptr);
size_t cc_memcount(void);
void   cc_meminfo(void);
size_t cc_memsize(void);
size_t cc_memsizeptr(void* ptr);

#ifndef MALLOC
	#ifdef MEMORY_DEBUG
		#define MALLOC(...) (cc_malloc_debug(__func__, __LINE__, __VA_ARGS__))
	#else
		#define MALLOC(...) (cc_malloc(__VA_ARGS__))
	#endif
#endif

#ifndef CALLOC
	#ifdef MEMORY_DEBUG
		#define CALLOC(...) (cc_calloc_debug(__func__, __LINE__, __VA_ARGS__))
	#else
		#define CALLOC(...) (cc_calloc(__VA_ARGS__))
	#endif
#endif

#ifndef REALLOC
	#ifdef MEMORY_DEBUG
		#define REALLOC(...) (cc_realloc_debug(__func__, __LINE__, __VA_ARGS__))
	#else
		#define REALLOC(...) (cc_realloc(__VA_ARGS__))
	#endif
#endif

#ifndef FREE
	#ifdef MEMORY_DEBUG
		#define FREE(...) (cc_free_debug(__func__, __LINE__, __VA_ARGS__))
	#else
		#define FREE(...) (cc_free(__VA_ARGS__))
	#endif
#endif

#ifndef MEMCHECK
	#ifdef MEMORY_DEBUG
		#define MEMCHECKPTR(...) (cc_memcheckptr_debug(__func__, __LINE__, __VA_ARGS__))
	#else
		#define MEMCHECKPTR(...)
	#endif
#endif

#ifndef MEMCOUNT
	#define MEMCOUNT(...) (cc_memcount())
#endif

#ifndef MEMINFO
	#ifdef MEMORY_DEBUG
		#define MEMINFO(...) (cc_meminfo_debug())
	#else
		#define MEMINFO(...) (cc_meminfo())
	#endif
#endif

#ifndef MEMSIZE
	#define MEMSIZE(...) (cc_memsize())
#endif

#ifndef MEMSIZEPTR
	#define MEMSIZEPTR(...) (cc_memsizeptr(__VA_ARGS__))
#endif

#endif
