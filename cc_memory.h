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

void* cc_malloc(const char* func, int line, size_t size);
void* cc_calloc(const char* func, int line, size_t count,
                size_t size);
void* cc_realloc(const char* func, int line, void* ptr,
                 size_t size);
void  cc_free(const char* func, int line, void* ptr);
void  cc_meminfo(void);

#ifndef MALLOC
	#ifdef MEMORY_DEBUG
		#define MALLOC(...) (cc_malloc(__func__, __LINE__, __VA_ARGS__))
	#else
		#define MALLOC(...) (malloc(__VA_ARGS__))
	#endif
#endif

#ifndef CALLOC
	#ifdef MEMORY_DEBUG
		#define CALLOC(...) (cc_calloc(__func__, __LINE__, __VA_ARGS__))
	#else
		#define CALLOC(...) (calloc(__VA_ARGS__))
	#endif
#endif

#ifndef REALLOC
	#ifdef MEMORY_DEBUG
		#define REALLOC(...) (cc_realloc(__func__, __LINE__, __VA_ARGS__))
	#else
		#define REALLOC(...) (realloc(__VA_ARGS__))
	#endif
#endif

#ifndef FREE
	#ifdef MEMORY_DEBUG
		#define FREE(...) (cc_free(__func__, __LINE__, __VA_ARGS__))
	#else
		#define FREE(...) (free(__VA_ARGS__))
	#endif
#endif

#ifndef MEMINFO
	#ifdef MEMORY_DEBUG
		#define MEMINFO(...) (cc_meminfo())
	#else
		#define MEMINFO(...)
	#endif
#endif

#endif
