#include "simplealloc.h"

// CHUNK % ALIGN == 0
#define CHUNK 1024
#define ALIGN 4
#define HEADER 4

/* 
Header structure:
|00000000000000000000000000000|00|a|
 a  - free/allocated
 2  - reserved
29  - size of block
*/

// Header macros
#define HGET(x) *((unsigned int *) x) // Get whole block header
#define HISA(x) (*((unsigned int *) x) & 1) // Get 'a' bit value
#define HSIZE(x) (*((unsigned int *) x) & -ALIGN) // Get size of block
#define HNEXT(x) (x + HSIZE(x)) // Get pointer to next block header
#define HSETA(x) (*((unsigned int *) x) |= 1) // Set 'a' bit
#define HUNSETA(x) (*((unsigned int *) x) &= -ALIGN) // Unset 'a' bit
#define HSHPLD(x) (x += HEADER) // Shift pointer to block payload
#define HSHHDR(x) (x -= HEADER) // Shift pointer to block header

static int exists = 0;
static void *heap = NULL;

static int init(void)
{
	if ((heap = sbrk(CHUNK)) == (void *) -1)
		return 0;
		
	HGET(heap) = CHUNK - ALIGN;
	void *term = HNEXT(heap);
	HGET(term) = 0;
	HSETA(term);

	return exists = 1;
}

static void *find(unsigned int nbytes)
{
	// First fit with coalescing
	void *prev, *current;
	for (prev = NULL, current = heap; HSIZE(current); prev = current, current += HSIZE(current))
		if (!HISA(current)) {
			while (!HISA(HNEXT(current)))
				HGET(current) += HSIZE(HNEXT(current));
			if (nbytes <= HSIZE(current))
				return current;
		}

	// Syscall for more memory
	if (sbrk(CHUNK) == (void *) -1)
		return NULL;
	HGET(current) = CHUNK;
	void *term = HNEXT(current);
	HGET(term) = 0;
	HSETA(term);
	HGET(prev) += HSIZE(current);

	return prev;
}

static void place(void *current, unsigned int nbytes)
{
	unsigned int memleft = HSIZE(current) - nbytes;

	void *next = current;
	next += nbytes;

	HGET(current) = nbytes;
	HSETA(current);

	HGET(next) = memleft;
	HUNSETA(next);
}

void *smp_malloc(unsigned int nbytes)
{
	if (!exists && !init() || nbytes == 0)
		return NULL;

	nbytes += HEADER;
	if (nbytes % ALIGN != 0)
		nbytes += ALIGN - nbytes % ALIGN;

	void *ptr = find(nbytes);
	if (!ptr)
		return NULL;

	place(ptr, nbytes);
	HSHPLD(ptr);

	return ptr;
}

void smp_free(void *ptr)
{
	HSHHDR(ptr);
	HUNSETA(ptr);
}

/* ----------DEBUG----------

static void printheader(void *header)
{
	unsigned int temp = HGET(header);
	char bin[33];
	for (int i=31; i>=0; i--, temp/=2)
		bin[i] = temp%2 + '0';
	bin[32] = '\0';
	printf("%s\n", bin);
}

static void printheap(void)
{
	void *current;
	for (current = heap; HSIZE(current); current += HSIZE(current))
		printf("|%d/%d", HSIZE(current), HISA(current));
	printf("|%d/%d|\n", HSIZE(current), HISA(current));
}

int main()
{
	smp_malloc(0);
	printheap();
	int *a = smp_malloc(2 * sizeof(int));
	printheap();
	char *b = smp_malloc(3 * sizeof(char));
	printheap();
	smp_free(a);
	printheap();
	short *c = smp_malloc(2 * sizeof(short));
	printheap();
	int *d = smp_malloc(5 * sizeof(int));
	printheap();
	smp_free(c);
	smp_free(b);
	printheap();
	int *e = smp_malloc(1 * sizeof(int));
	printheap();

	return 0;
}
*/
