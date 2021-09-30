#ifndef POOLALLOC_H
#define POOLALLOC_H

typedef struct _poolalloc poolalloc;

poolalloc *pa_create(unsigned int block_size, unsigned int block_count);
void *pa_alloc(poolalloc *pa, unsigned int block_count);
void pa_free(poolalloc *pa, void *block_ptr, unsigned int block_count);
void pa_destroy(poolalloc *pa);
void pa_debug(poolalloc *pa);

#endif
