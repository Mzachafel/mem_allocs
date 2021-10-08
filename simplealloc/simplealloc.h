#ifndef SIMPLE_ALLOC_H
#define SIMPLE_ALLOC_H

#include <unistd.h>

void *smp_malloc(unsigned int nbytes);
void smp_free(void *);

#endif
