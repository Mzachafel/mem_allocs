#include <stdlib.h>
#include <stdio.h>

typedef struct _poolalloc {
	void *begin;
	void *end;
	void *free_blocks;
	size_t block_size;
	size_t block_count;
} poolalloc;

poolalloc *pa_create(size_t block_size, size_t block_count)
{
	poolalloc *pa = malloc(sizeof(poolalloc));
	if (!pa)
		return NULL;
	pa->begin = malloc(block_size * block_count);
	if (!pa->begin) {
		free(pa);
		return NULL;
	}
	pa->end = pa->begin + block_size * block_count;
	pa->free_blocks = calloc(1 + block_count/8, sizeof(char));
	if (!pa->free_blocks) {
		free(pa->begin);
		free(pa);
		return NULL;
	}
	pa->block_size = block_size;
	pa->block_count = block_count;
	return pa;
}

void *pa_alloc(poolalloc *pa, size_t block_count)
{
	int start=-1; // beginning of sequence of empty blocks
	int n=0; // how many blocks in sequence currently
	int i; // current block
	// loop through all blocks
	for (i=0; i<pa->block_count; i++) {
		// if block is empty
		if (!(*((char *) (pa->free_blocks + i/8)) & 1 << ( i % 8 ))) {
			if (start == -1)
				start = i;
			n++;
			if (n == block_count)
				break;
		}
		// if block is full
		else {
			// restart search
			start=-1;
			n=0;
		}
	}
	// cant find large enough space
	if (n < block_count)
		return NULL;
	// set to full all blocks
	for (i=start; block_count--; i++)
		*((char *) (pa->free_blocks + i/8)) |= 1 << ( i % 8 );
	// return pointer to first block in sequence
	return pa->begin + start;
}

void pa_free(poolalloc *pa, void *block_ptr, size_t block_count)
{
	int i;
	// bound checking
	if (block_ptr < pa->begin || block_ptr > pa->end ||
	    block_ptr + block_count * pa->block_size > pa->end)
		return;
	// alignment checking
	if ((unsigned long) block_ptr % pa->block_size != 0)
		return;
	// empty bits in free_blocks
	for (i=pa->begin-block_ptr; i<block_count; i++)
		*((char *) (pa->free_blocks + i/8)) &= ~(1 << ( i % 8 ));
}

void pa_destroy(poolalloc *pa)
{
	free(pa->begin);
	free(pa->free_blocks);
	free(pa);
}

void pa_debug(poolalloc *pa)
{
	int i;
	for (i=0; i<pa->block_count; i++)
		if (!(*((char *) (pa->free_blocks + i/8)) & 1 << ( i % 8 )))
			printf("0");
		else
			printf("1");
	printf("\n");
}
