#include <stdlib.h>

typedef struct _linalloc {
	void *start;
	void *current;
	void *end;
} linalloc;

linalloc *la_create(size_t size)
{
	if (!size)
		return NULL;
	linalloc *la = malloc(sizeof(linalloc));
	if (!la)
		return NULL;
	la->start = malloc(size);
	if (!la->start) {
		free(la);
		return NULL;
	}
	la->current = la->start;
	la->end = la->start + size;
	return la;
}

void *la_alloc(linalloc *la, size_t size)
{
	if (!size || la->current + size > la->end)
		return NULL;
	void *ret = la->current;
	la->current += size;
	return ret;
}

void la_free(linalloc *la)
{
	la->current = la->start;
}

void la_destroy(linalloc *la)
{
	free(la->start);
	free(la);
}
