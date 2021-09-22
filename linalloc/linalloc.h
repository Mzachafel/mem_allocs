#ifndef LINALLOC_H
#define LINALLOC_H

struct linalloc *la_create(unsigned int size, unsigned int align);
void *la_allocate(struct linalloc *la, unsigned int size);
void la_free(struct linalloc *la);
void la_destroy(struct linalloc *la);
void la_debug(struct linalloc *la);

#endif
