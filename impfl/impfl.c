#include <unistd.h>

// CHUNK % ALIGN == 0
#define CHUNK 4096
#define ALIGN 8
#define HEADER 4

/* 
Header structure:
|00000000000000000000000000000|00|a|
 a  - free/allocated
 2  - reserved
29  - size of block
*/

// Header macros
#define HGET(x) *((unsigned int *) x) // Get block header
#define HPUT(x,val) (*((unsigned int *) x) = val) // Put value in block header
#define HADD(x,val) (*((unsigned int *) x) += val) // Add value to block header
#define HISA(x) (*((unsigned int *) x) & 1) // Get 'a' bit value
#define HSIZE(x) (*((unsigned int *) x) & -ALIGN) // Get size of block
#define HNEXT(x) (x + HSIZE(x)) // Get pointer to next block header
#define HSETA(x) (*((unsigned int *) x) |= 1) // Set 'a' bit
#define HUNSETA(x) (*((unsigned int *) x) &= -ALIGN) // Unset 'a' bit
#define HSHPLD(x) (x += HEADER) // Shift pointer to block payload
#define HSHHDR(x) (x -= HEADER) // Shift pointer to block header

static int exists = 0;
static void *heap = NULL;

static int expand(void *ptr, unsigned int nbytes)
{
	if (sbrk(nbytes) == (void *) -1)
		return 0;
		
	HPUT(ptr,nbytes);
	void *term = HNEXT(ptr);
	HPUT(term,0);
	HSETA(term);

	return 1;
}

static int init(void)
{
	if ((heap = sbrk(HEADER)) == (void *) -1)
		return 0;
	HPUT(heap,0);
	HSETA(heap);

	if (!expand(heap, CHUNK))
		return 0;

	return exists = 1;
}

static void *find(unsigned int nbytes)
{
	// First fit with coalescing
	void *prev, *current;
	for (prev = NULL, current = heap; HSIZE(current); prev = current, current += HSIZE(current))
		if (!HISA(current)) {
			while (!HISA(HNEXT(current)))
				HADD(current,HSIZE(HNEXT(current)));
			if (nbytes <= HSIZE(current))
				return current;
		}

	// Syscall for more memory
	if (!HISA(prev))
		nbytes -= HSIZE(prev);
	nbytes = CHUNK > nbytes ? CHUNK : nbytes;
	if (!expand(current, nbytes))
		return NULL;

	if (!HISA(prev)) {
		HADD(prev,HSIZE(current));
		return prev;
	} else {
		return current;
	}
}

static void place(void *current, unsigned int nbytes)
{
	unsigned int memleft = HSIZE(current) - nbytes;

	void *next = current;
	next += nbytes;

	HPUT(current,nbytes);
	HSETA(current);

	HPUT(next,memleft);
	if (!HSIZE(next))
		HSETA(next);
}

void *ifl_malloc(unsigned int nbytes)
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

	// printf("Allocated %d bytes\n", nbytes); DEBUG

	return ptr;
}

void ifl_free(void *ptr)
{
	HSHHDR(ptr);
	HUNSETA(ptr);

	// printf("Freed %d bytes\n", HSIZE(ptr)); DEBUG
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
	printf("Init\n");
	printheap();
	int *a = smp_malloc(2 * sizeof(int));
	printheap();
	char *b = smp_malloc(3 * sizeof(char));
	printheap();
	smp_free(a);
	printheap();
	short *c = smp_malloc(2 * sizeof(short));
	printheap();
	int *d = smp_malloc(3 * sizeof(int));
	printheap();
	smp_free(c);
	smp_free(b);
	printheap();
	int *e = smp_malloc(1 * sizeof(int));
	printheap();
	int *f = smp_malloc(20 * sizeof(int));
	printheap();

	return 0;
}
*/
