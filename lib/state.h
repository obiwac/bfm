#pragma once

#include <stdlib.h>
// XXX separate file?

typedef struct {
	char* msg;

	char const* file;
	char const* func;
	size_t line;
} bfm_err_t;

typedef void* (*bfm_alloc_t) (size_t size);
typedef void (*bfm_free_t) (void* ptr);

typedef struct {
	bfm_err_t err;

	bfm_alloc_t alloc;
	bfm_free_t free;
} bfm_state_t;

int bfm_state_create(bfm_state_t* state);
int bfm_state_destroy(bfm_state_t* state);

int bfm_set_alloc(bfm_state_t* state, bfm_alloc_t alloc);
int bfm_set_free(bfm_state_t* state, bfm_alloc_t free);

int bfm_err_print(bfm_state_t* state);
