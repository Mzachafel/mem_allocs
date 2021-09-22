#include <stdlib.h>

struct linalloc {
	void *start;
	size_t offset;
	size_t end;
};

struct linalloc *la_create(size_t size)
{
	struct linalloc *la = malloc(sizeof(struct linalloc));
	la->start = malloc(size);
	la->offset = 0;
	la->end = size;
	return la;
}

void *la_allocate(struct linalloc *la, size_t size)
{
	if (la->offset + size > la->end)
		return NULL;
	void *ret = la->free;
	la->free += size;
	return ret;
}

void la_free(struct linalloc *la)
{
	la->offset = 0;
}

void la_destroy(struct linalloc *la)
{
	free(la->start);
	free(la);
}
