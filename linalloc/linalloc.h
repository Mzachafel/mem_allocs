#ifndef LINALLOC_H
#define LINALLOC_H

typedef struct _linalloc linalloc;

linalloc *la_create(unsigned int size);
void *la_alloc(linalloc *la, unsigned int size);
void la_free(linalloc *la);
void la_destroy(linalloc *la);

#endif
