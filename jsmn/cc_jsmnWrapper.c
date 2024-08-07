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

#include <stdio.h>
#include <stdlib.h>

#define LOG_TAG "cc"
#include "../cc_log.h"
#include "../cc_memory.h"
#include "cc_jsmnWrapper.h"

// import the jsmn implementation
#define JSMN_STATIC
#include "../../jsmn/jsmn.h"

typedef struct
{
	// arguments
	const char* str;
	size_t      len;

	// parser
	jsmn_parser parser;
	int         pos;
	int         count;
	jsmntok_t*  tokens;
} cc_jsmnWrapper_t;

// forward declarations
static cc_jsmnObject_t* cc_jsmnObject_wrap(cc_jsmnWrapper_t* jw);
static void             cc_jsmnObject_delete(cc_jsmnObject_t** _self);
static cc_jsmnArray_t*  cc_jsmnArray_wrap(cc_jsmnWrapper_t* jw);
static void             cc_jsmnArray_delete(cc_jsmnArray_t** _self);
static cc_jsmnVal_t*    cc_jsmnVal_wrap(cc_jsmnWrapper_t* jw);
static cc_jsmnKeyval_t* cc_jsmnKeyval_wrap(cc_jsmnWrapper_t* jw);
static void             cc_jsmnKeyval_delete(cc_jsmnKeyval_t** _self);

/***********************************************************
* private                                                  *
***********************************************************/

static cc_jsmnWrapper_t*
cc_jsmnWrapper_new(const char* str, size_t len)
{
	ASSERT(str);

	cc_jsmnWrapper_t* self;
	self = (cc_jsmnWrapper_t*)
	       CALLOC(1, sizeof(cc_jsmnWrapper_t));
	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	self->str = str,
	self->len = len,

	jsmn_init(&self->parser);
	self->count = jsmn_parse(&self->parser, str, len, NULL, 0);
	if(self->count <= 0)
	{
		LOGE("jsmn_parse failed");
		goto fail_parse1;
	}

	self->tokens = (jsmntok_t*)
	               CALLOC(self->count, sizeof(jsmntok_t));
	if(self->tokens == NULL)
	{
		LOGE("CALLOC failed");
		goto fail_tokens;
	}

	jsmn_init(&self->parser);
	int count = jsmn_parse(&(self->parser), str, len,
	                       self->tokens, self->count);
	if(self->count != count)
	{
		LOGE("invalid count1=%i, count2=%i",
		     self->count, count);
		goto fail_parse2;
	}

	// success
	return self;

	// failure
	fail_parse2:
		FREE(self->tokens);
	fail_tokens:
	fail_parse1:
		FREE(self);
	return NULL;
}

static void
cc_jsmnWrapper_delete(cc_jsmnWrapper_t** _self)
{
	ASSERT(_self);

	cc_jsmnWrapper_t* self = (cc_jsmnWrapper_t*) *_self;
	if(self)
	{
		FREE(self->tokens);
		FREE(self);
		*_self = NULL;
	}
}

static jsmntok_t*
cc_jsmnWrapper_step(cc_jsmnWrapper_t* self, char** _data)
{
	ASSERT(self);
	ASSERT(_data);
	ASSERT(*_data == NULL);

	if(self->pos >= self->count)
	{
		return NULL;
	}

	jsmntok_t* tok = &(self->tokens[self->pos]);
	if((tok->type == JSMN_UNDEFINED) ||
	   (tok->type == JSMN_STRING)    ||
	   (tok->type == JSMN_PRIMITIVE))
	{
		size_t len  = tok->end - tok->start;
		char*  data = CALLOC(len + 1, sizeof(char));
		if(data == NULL)
		{
			LOGE("CALLOC failed");
			return NULL;
		}

		int i;
		int start = tok->start;
		for(i = 0; i < len; ++i)
		{
			data[i] = self->str[start + i];
		}
		data[len] = '\0';

		*_data = data;
	}

	++self->pos;

	return tok;
}

static jsmntok_t*
cc_jsmnWrapper_peek(cc_jsmnWrapper_t* self)
{
	ASSERT(self);

	if(self->pos >= self->count)
	{
		return NULL;
	}

	return &(self->tokens[self->pos]);
}

static cc_jsmnObject_t*
cc_jsmnObject_wrap(cc_jsmnWrapper_t* jw)
{
	ASSERT(jw);

	cc_jsmnObject_t* self;
	self = (cc_jsmnObject_t*)
	       CALLOC(1, sizeof(cc_jsmnObject_t));
	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	self->list = cc_list_new();
	if(self->list == NULL)
	{
		goto fail_list;
	}

	char*      str = NULL;
	jsmntok_t* tok = cc_jsmnWrapper_step(jw, &str);
	if(tok == NULL)
	{
		goto fail_tok;
	}

	if(tok->type != JSMN_OBJECT)
	{
		goto fail_type;
	}

	// parse kv pairs
	int i;
	cc_jsmnKeyval_t* kv;
	for(i = 0; i < tok->size; ++i)
	{
		kv = cc_jsmnKeyval_wrap(jw);
		if(kv == NULL)
		{
			goto fail_kv;
		}

		if(cc_list_append(self->list, NULL,
		                  (const void*) kv) == NULL)
		{
			goto fail_append;
		}
	}

	// success
	return self;

	// failure
	fail_append:
		cc_jsmnKeyval_delete(&kv);
	fail_kv:
	{
		cc_listIter_t* iter;
		iter = cc_list_head(self->list);
		while(iter)
		{
			kv = (cc_jsmnKeyval_t*)
			     cc_list_remove(self->list, &iter);
			cc_jsmnKeyval_delete(&kv);
		}
	}
	fail_type:
		FREE(str);
	fail_tok:
		cc_list_delete(&self->list);
	fail_list:
		FREE(self);
	return NULL;
}

static void cc_jsmnObject_delete(cc_jsmnObject_t** _self)
{
	ASSERT(_self);

	cc_jsmnObject_t* self = *_self;
	if(self)
	{
		cc_listIter_t* iter = cc_list_head(self->list);
		while(iter)
		{
			cc_jsmnKeyval_t* kv;
			kv = (cc_jsmnKeyval_t*)
			     cc_list_remove(self->list, &iter);
			cc_jsmnKeyval_delete(&kv);
		}

		cc_list_delete(&self->list);
		FREE(self);
		*_self = NULL;
	}
}

static cc_jsmnArray_t*
cc_jsmnArray_wrap(cc_jsmnWrapper_t* jw)
{
	ASSERT(jw);

	cc_jsmnArray_t* self;
	self = (cc_jsmnArray_t*)
	       CALLOC(1, sizeof(cc_jsmnArray_t));
	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	self->list = cc_list_new();
	if(self->list == NULL)
	{
		goto fail_list;
	}

	char*      str = NULL;
	jsmntok_t* tok = cc_jsmnWrapper_step(jw, &str);
	if(tok == NULL)
	{
		goto fail_tok;
	}

	if(tok->type != JSMN_ARRAY)
	{
		goto fail_type;
	}

	// parse vals
	int i;
	cc_jsmnVal_t* val;
	for(i = 0; i < tok->size; ++i)
	{
		val = cc_jsmnVal_wrap(jw);
		if(val == NULL)
		{
			goto fail_val;
		}

		if(cc_list_append(self->list, NULL,
		                  (const void*) val) == NULL)
		{
			goto fail_append;
		}
	}

	// success
	return self;

	// failure
	fail_append:
		cc_jsmnVal_delete(&val);
	fail_val:
	{
		cc_listIter_t* iter;
		iter = cc_list_head(self->list);
		while(iter)
		{
			val = (cc_jsmnVal_t*)
			      cc_list_remove(self->list, &iter);
			cc_jsmnVal_delete(&val);
		}
	}
	fail_type:
		FREE(str);
	fail_tok:
		cc_list_delete(&self->list);
	fail_list:
		FREE(self);
	return NULL;
}

static void cc_jsmnArray_delete(cc_jsmnArray_t** _self)
{
	ASSERT(_self);

	cc_jsmnArray_t* self = *_self;
	if(self)
	{
		cc_listIter_t* iter = cc_list_head(self->list);
		while(iter)
		{
			cc_jsmnVal_t* v;
			v = (cc_jsmnVal_t*)
			    cc_list_remove(self->list, &iter);
			cc_jsmnVal_delete(&v);
		}

		cc_list_delete(&self->list);
		FREE(self);
		*_self = NULL;
	}
}

static cc_jsmnVal_t*
cc_jsmnVal_wrap(cc_jsmnWrapper_t* jw)
{
	ASSERT(jw);

	cc_jsmnVal_t* self;
	self = (cc_jsmnVal_t*)
	       CALLOC(1, sizeof(cc_jsmnVal_t));
	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	jsmntok_t* tok = cc_jsmnWrapper_peek(jw);
	if(tok == NULL)
	{
		goto fail_tok;
	}

	// parse val
	char* str = NULL;
	if(tok->type == JSMN_OBJECT)
	{
		self->obj = cc_jsmnObject_wrap(jw);
		if(self->obj == NULL)
		{
			goto fail_val;
		}
	}
	else if(tok->type == JSMN_ARRAY)
	{
		self->array = cc_jsmnArray_wrap(jw);
		if(self->array == NULL)
		{
			goto fail_val;
		}
	}
	else
	{
		tok = cc_jsmnWrapper_step(jw, &str);
		if((tok == NULL) || (str == NULL))
		{
			goto fail_val;
		}

		self->data = str;
	}

	self->type = (cc_jsmnType_e) tok->type;

	// success
	return self;

	// failure
	fail_val:
		FREE(str);
	fail_tok:
		FREE(self);
	return NULL;
}

static cc_jsmnKeyval_t*
cc_jsmnKeyval_wrap(cc_jsmnWrapper_t* jw)
{
	ASSERT(jw);

	cc_jsmnKeyval_t* self;
	self = (cc_jsmnKeyval_t*)
	       CALLOC(1, sizeof(cc_jsmnKeyval_t));
	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	jsmntok_t* tok = cc_jsmnWrapper_step(jw, &self->key);
	if(tok == NULL)
	{
		goto fail_tok;
	}

	if(tok->type != JSMN_STRING)
	{
		goto fail_type;
	}

	// parse val
	self->val = cc_jsmnVal_wrap(jw);
	if(self->val == NULL)
	{
		goto fail_val;
	}

	// success
	return self;

	// failure
	fail_val:
	fail_type:
		FREE(self->key);
	fail_tok:
		FREE(self);
	return NULL;
}

static void cc_jsmnKeyval_delete(cc_jsmnKeyval_t** _self)
{
	ASSERT(_self);

	cc_jsmnKeyval_t* self = *_self;
	if(self)
	{
		cc_jsmnVal_delete(&self->val);
		FREE(self->key);
		FREE(self);
		*_self = NULL;
	}
}

/***********************************************************
* private printing                                         *
***********************************************************/

static void cc_jsmnVal_printd(cc_jsmnVal_t* self,
                              int depth);
static void cc_jsmnObject_printd(cc_jsmnObject_t* self,
                                 int depth);
static void cc_jsmnArray_printd(cc_jsmnArray_t* self,
                                int depth);

static void cc_jsmn_indent(int depth)
{
	int i;
	for(i = 0; i < depth; ++i)
	{
		printf("\t");
	}
}

static void
cc_jsmnVal_printd(cc_jsmnVal_t* self, int depth)
{
	ASSERT(self);

	if(self->type == CC_JSMN_TYPE_OBJECT)
	{
		cc_jsmn_indent(depth);
		printf("{\n");
		cc_jsmnObject_printd(self->obj, depth + 1);
		cc_jsmn_indent(depth);
		printf("}\n");
	}
	else if(self->type == CC_JSMN_TYPE_ARRAY)
	{
		cc_jsmn_indent(depth);
		printf("[\n");
		cc_jsmnArray_printd(self->array, depth + 1);
		cc_jsmn_indent(depth);
		printf("]\n");
	}
	else
	{
		cc_jsmn_indent(depth);
		printf("%s\n", self->data);
	}
}

static void
cc_jsmnObject_printd(cc_jsmnObject_t* self, int depth)
{
	ASSERT(self);

	cc_listIter_t* iter = cc_list_head(self->list);
	while(iter)
	{
		cc_jsmnKeyval_t* kv;
		kv = (cc_jsmnKeyval_t*) cc_list_peekIter(iter);

		cc_jsmn_indent(depth);
		printf("%s:\n", kv->key);
		cc_jsmnVal_printd(kv->val, depth + 1);

		iter = cc_list_next(iter);
	}
}

static void
cc_jsmnArray_printd(cc_jsmnArray_t* self, int depth)
{
	ASSERT(self);

	cc_listIter_t* iter = cc_list_head(self->list);
	while(iter)
	{
		cc_jsmnVal_t* val;
		val = (cc_jsmnVal_t*) cc_list_peekIter(iter);
		cc_jsmnVal_printd(val, depth);

		iter = cc_list_next(iter);
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

cc_jsmnVal_t*
cc_jsmnVal_new(const char* str, size_t len)
{
	ASSERT(str);

	cc_jsmnWrapper_t* jw = cc_jsmnWrapper_new(str, len);
	if(jw == NULL)
	{
		return NULL;
	}

	cc_jsmnVal_t* self = cc_jsmnVal_wrap(jw);
	if(self == NULL)
	{
		goto fail_val;
	}

	cc_jsmnWrapper_delete(&jw);

	// success
	return self;

	// failure
	fail_val:
		cc_jsmnWrapper_delete(&jw);
	return NULL;
}

cc_jsmnVal_t* cc_jsmnVal_import(const char* fname)
{
	ASSERT(fname);

	FILE* f = fopen(fname, "r");
	if(f == NULL)
	{
		LOGE("invalid %s", fname);
		return NULL;
	}

	// get file size
	if(fseek(f, (long) 0, SEEK_END) == -1)
	{
		LOGE("fseek failed");
		goto fail_size;
	}
	size_t size = ftell(f);

	// rewind to start
	if(fseek(f, 0, SEEK_SET) == -1)
	{
		LOGE("fseek failed");
		goto fail_rewind;
	}

	// allocate buffer
	char* str = (char*) CALLOC(1, size);
	if(str == NULL)
	{
		LOGE("CALLOC failed");
		goto fail_str;
	}

	// read file
	if(fread((void*) str, size, 1, f) != 1)
	{
		LOGE("fread failed");
		goto fail_read;
	}

	cc_jsmnVal_t* self = cc_jsmnVal_new(str, size);
	if(self == NULL)
	{
		goto fail_val;
	}

	FREE(str);
	fclose(f);

	// success
	return self;

	// failure
	fail_val:
	fail_read:
		FREE(str);
	fail_str:
	fail_rewind:
	fail_size:
		fclose(f);
	return NULL;
}

void cc_jsmnVal_delete(cc_jsmnVal_t** _self)
{
	ASSERT(_self);

	cc_jsmnVal_t* self = *_self;
	if(self)
	{
		if(self->type == CC_JSMN_TYPE_OBJECT)
		{
			cc_jsmnObject_delete(&self->obj);
		}
		else if(self->type == CC_JSMN_TYPE_ARRAY)
		{
			cc_jsmnArray_delete(&self->array);
		}
		else
		{
			FREE(self->data);
		}

		FREE(self);
		*_self = NULL;
	}
}

void cc_jsmnVal_print(cc_jsmnVal_t* self)
{
	ASSERT(self);

	cc_jsmnVal_printd(self, 0);
}
