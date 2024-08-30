/*
 * Copyright (c) 2020 Jeff Boody
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

#ifndef cc_jsmnWrapper_H
#define cc_jsmnWrapper_H

#include "../cc_list.h"

typedef enum
{
	CC_JSMN_TYPE_UNDEFINED = 0,
	CC_JSMN_TYPE_OBJECT    = 1 << 0,
	CC_JSMN_TYPE_ARRAY     = 1 << 1,
	CC_JSMN_TYPE_STRING    = 1 << 2,
	CC_JSMN_TYPE_PRIMITIVE = 1 << 3
} cc_jsmnType_e;

typedef struct
{
	// list of keyval pairs
	cc_list_t* list;
} cc_jsmnObject_t;

typedef struct
{
	// list of vals
	cc_list_t* list;
} cc_jsmnArray_t;

typedef struct
{
	cc_jsmnType_e type;

	union
	{
		cc_jsmnObject_t* obj;
		cc_jsmnArray_t*  array;
		char*            data;
	};
} cc_jsmnVal_t;

typedef struct
{
	char*         key;
	cc_jsmnVal_t* val;
} cc_jsmnKeyval_t;

cc_jsmnVal_t* cc_jsmnVal_new(const char* str, size_t len);
cc_jsmnVal_t* cc_jsmnVal_import(const char* fname);
void          cc_jsmnVal_delete(cc_jsmnVal_t** _self);
void          cc_jsmnVal_print(cc_jsmnVal_t* val);

#endif
