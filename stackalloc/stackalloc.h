#ifndef STACKALLOC_H
#define STACKALLOC_H

#include <stdlib.h>
#include <stdio.h>

typedef struct _stackalloc stackalloc;

stackalloc *sa_create(size_t size);
void *sa_alloc(stackalloc *sa, size_t size);
void sa_free(stackalloc *sa);
void sa_destroy(stackalloc *sa);
void sa_debug(stackalloc *sa);

#endif
