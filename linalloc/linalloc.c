#include <stdlib.h>
#include <stdio.h>

struct linalloc {
	void *start;
	void *free;
	void *end;
	int align;
};

struct linalloc *la_create(size_t size, size_t align)
{
	struct linalloc *la = malloc(sizeof(struct linalloc));
	la->start = malloc(size);
	la->free = la->start;
	la->end = la->start+size;
	la->align = align;
	return la;
}

void *la_allocate(struct linalloc *la, size_t size)
{
	size += size % la->align;
	if (la->free + size <= la->end) {
		void *ret = la->free;
		la->free += size;
		return ret;
	}
	return NULL;
}

void la_free(struct linalloc *la)
{
	la->free = la->start;
}

void la_destroy(struct linalloc *la)
{
	free(la->start);
	free(la);
}

void la_debug(struct linalloc *la)
{
	printf("start: %p\nend:   %p\nfree:  %p\n", la->start, la->end, la->free);
}
