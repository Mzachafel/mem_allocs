#ifndef SIMPLE_ALLOC_H
#define SIMPLE_ALLOC_H

#include <unistd.h>
#include <stdio.h>

void *smp_malloc(unsigned int nbytes);
void smp_free(void *);

#endif
