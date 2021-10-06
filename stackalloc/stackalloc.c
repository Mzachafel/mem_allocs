#include "stackalloc.h"

struct _stackalloc {
	void *start;
	void *current;
	void *end;
};

stackalloc *sa_create(size_t size)
{
	if (!size)
		return NULL;
	stackalloc *sa = malloc(sizeof(stackalloc));
	if (!sa)
		return NULL;
	sa->start = malloc(size);
	if (!sa->start) {
		free(sa);
		return NULL;
	}
	sa->current = sa->start;
	sa->end = sa->start + size;
	return sa;
}

void *sa_alloc(stackalloc *sa, size_t size)
{
	if (!size || sa->current + size + 1 > sa->end)
		return NULL;
	void *ret = sa->current;
	sa->current += size;
	*((char *) sa->current) = size;
	sa->current++;
	return ret;
}

void sa_free(stackalloc *sa)
{
	sa->current--;
	sa->current -= *((char *) sa->current);
}

void sa_destroy(stackalloc *sa)
{
	free(sa->start);
	free(sa);
}

void sa_debug(stackalloc *sa)
{
	printf("%p\n", sa->current);
}
