#include <unistd.h>
#include <stdio.h>

// MINIMAL BLOCK SIZE
// HEADER + PREV + NEXT + FOOTER
// 8 + 8 + 8 + 8 = 32

// MINIMAL PAYLOAD = PREV + NEXT = 16

// CHUNK % ALIGN == 0
#define CHUNK 4096
#define ALIGN 8
#define BLOCK 32
#define SECTION 8

/* 
Block structure:
|61|2|a| - header
|//64//| - previous free block
|//64//| - next free block
|61|2|a| - footer

|/63/|a| - last block

 a  - free/allocated
 3  - reserved
60  - size of block
64  - size of pointer
*/

		// Block macros
	// Header
#define HGET(x)   (*((unsigned long *)x))            // Get value
#define HPUT(x,v) (HGET(x)=v)                        // Put value
#define HISA(x)   (HGET(x)&1)       		     // Get 'a' bit value
#define HSIZE(x)  (HGET(x)&-ALIGN)  		     // Get size of block
#define HSETA(x)  (HGET(x)|=1)      		     // Set 'a' bit
#define HCLRA(x)  (HGET(x)&=-ALIGN)                  // Clear 'a' bit
	// Previous block
#define PGET(x)   (*((void **)(x+SECTION)))  	     // Get value
#define PPUT(x,v) (PGET(x)=v)                        // Put value
	// Next block
#define NGET(x)   (*((void **)(x+2*SECTION)))  	     // Get value
#define NPUT(x,v) (NGET(x)=v)                        // Put value
	// Footer
// Forward usage
#define FFGET(x)   (*((unsigned long *)(x+HSIZE(x)))) // Get value 
#define FFPUT(x,v) (FFGET(x)=v) 		      // Put value
#define FFISA(x)   (FFGET(x)&1) 		      // Get 'a' bit value
#define FFSIZE(x)  (FFGET(x)&-ALIGN) 		      // Get size of block
#define FFSETA(x)  (FFGET(x)|=1) 		      // Set 'a' bit
#define FFCLRA(x)  (FFGET(x)&=-ALIGN) 		      // Clear 'a' bit
// Backward Usage
#define FBGET(x)   (*((unsigned long *)(x-SECTION)))
#define FBPUT(x,v)
#define FBISA(x)   (FBGET(x)&1)
#define FBSIZE(x)  (FBGET(x)&-ALIGN)
	// Actions
#define NEXT(x) (x=x+HSIZE(x)+SECTION)                // Go to next block
#define PREV(x) (x=x-FBSIZE(x)-SECTION)		      // Go to previous block
#define TOPLD(x) (x=x+SECTION)			      // Shift pointer to payload
#define TOHDR(x) (x=x-SECTION)			      // Shift pointer to header


static int exists = 0;
static void *begin = NULL;
static void *end = NULL;
static void *fl = NULL;

static void cls(void *ptr)
{
	unsigned long size;
	void *tmp, *prev, *next;

	// link prev fb with next fb
	prev = PGET(ptr);
	next = NGET(ptr);
	if (prev != NULL) NPUT(prev,next);
	if (next != NULL) PPUT(next,prev);

	// get new size and set fl
	size = HSIZE(ptr) + SECTION;
	tmp = ptr;
	PREV(ptr);
	if (fl == tmp) fl = ptr;
	size += HSIZE(ptr);	

	// put values in new free block
	HPUT(ptr,size);
	HCLRA(ptr);
	FFPUT(ptr,size);
	FFCLRA(ptr);
}

static void coalesce(void *ptr)
{
	unsigned long size;
	void *tmp, *prev, *next;
 
	// left block
	if (ptr > begin && !FBISA(ptr))
		cls(ptr);

	tmp = ptr;
	NEXT(ptr); 

	// right block
	if (ptr < end && !HISA(ptr))
		cls(ptr);
}

static int expand(unsigned int nbytes)
{
	if (sbrk(nbytes+2*SECTION) == (void *) -1)
		return 0;

	// create new free block
	HPUT(end,nbytes);
	HCLRA(end);
	PPUT(end,NULL);
	NPUT(end,fl);
	if (fl) PPUT(fl,end);
	FFPUT(end,nbytes);
	FFCLRA(end);
	fl = end;

	// set new null block
	NEXT(end);
	HPUT(end,0);
	HSETA(end);

	coalesce(fl);

	return 1;
}

static int init(void)
{
	if ((end = begin = sbrk(SECTION)) == (void *) -1)
		return 0;

	// create new null block
	HPUT(end,0);
	HSETA(end);

	if (!expand(CHUNK))
		return 0;

	return exists = 1;
}

static void *find(unsigned long nbytes)
{
	void *tmp;

	// First fit
	for (tmp=fl; tmp!=NULL; tmp=NGET(tmp))
		if (HSIZE(tmp) >= nbytes)
			return tmp;

	// Syscall for more memory
	if (!expand(nbytes))
		return NULL;
	return fl;
}

static void place(void *ptr, unsigned long nbytes)
{
	unsigned long memleft;
	void *tmp, *prev, *next;

	prev = PGET(ptr);
	next = NGET(ptr);

	memleft = HSIZE(ptr) - nbytes - SECTION;

	if (memleft < 2*SECTION) { // just use the whole block
		// this block is in free list so connect prev fb with next
		if (prev != NULL) NPUT(prev,next);
		if (next != NULL) PPUT(next,prev);
		// fl must be set to another block in list
		if (ptr == fl)
			fl = next;
		// just set block to alloced
		HSETA(ptr);
		FFSETA(ptr);
	} else { // separate new free block
		// set new size for this block (header and footer)
		HPUT(ptr,nbytes);
		HSETA(ptr);
		FFPUT(ptr,nbytes);
		FFSETA(ptr);
		// get to new fb and set fl
		tmp = ptr;
		NEXT(ptr);
		if (tmp == fl)
			fl = ptr;
		// set new sizes and pointers for it
		HPUT(ptr,memleft);
		HCLRA(ptr);
		PPUT(ptr,prev);
		NPUT(ptr,next);
		FFPUT(ptr,memleft);
		FFCLRA(ptr);
	}
}

void *efl_malloc(unsigned int nbytes)
{
	void *ptr;

	if (!exists && !init() || nbytes == 0)
		return NULL;
	
	if (nbytes < 2*SECTION)
		nbytes = 2*SECTION;
	if (nbytes % ALIGN != 0)
		nbytes += ALIGN - nbytes % ALIGN;

	ptr = find(nbytes);
	if (!ptr)
		return NULL;

	place(ptr, nbytes);
	TOPLD(ptr);

	printf("Allocated %d bytes\n", nbytes); //DEBUG

	return ptr;
}

void efl_free(void *ptr)
{
	// switch to header
	TOHDR(ptr);
	// add to fl
	HCLRA(ptr);
	PPUT(ptr,NULL);
	NPUT(ptr,fl);
	if (fl) PPUT(fl,ptr);
	FFCLRA(ptr);
	fl = ptr;
	
	// coalesce
	coalesce(fl);

	//printf("Freed %d bytes\n", HSIZE(ptr)); DEBUG
}

/* ----------DEBUG---------- */

static void printblock(void *ptr)
{
	if (!ptr) return;
	printf("%p=|%ld/%ld|%p|%p|%ld/%ld|\n", ptr, HSIZE(ptr), HISA(ptr), PGET(ptr),
		                 	       NGET(ptr), FFSIZE(ptr), FFISA(ptr));
}

static void printfreelist(void)
{
	void *tmp;
	for (tmp=fl; tmp!=NULL; tmp=NGET(tmp))
		printblock(tmp);
}

static void printheap(void)
{
	void *curr;
	for (curr = begin; HSIZE(curr); NEXT(curr))
		printf("%p=|%ld/%ld|%ld/%ld|\n", curr, HSIZE(curr), HISA(curr), FFSIZE(curr), FFISA(curr));
	printf("%p=|%ld/%ld|\n", curr, HSIZE(curr), HISA(curr));
}

static void datacheck(long *a, long *b)
{
	for (int i=0; i<20; i++)
		if (a[i] != b[i]) {
			printf("%d - data error\n", i);
			return;
		}
	printf("no data error\n");
}

int main()
{
	//init();
	//printheap();
	//printf("\n");
	
	// integrity test
	long base[20] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
	long *test = efl_malloc(20*sizeof(long));
	for (int i=0; i<20; i++)
		test[i] = base[i];
	datacheck(base, test);
	printheap();
	printf("\n");

	char *str;
	int *arr;
	str = efl_malloc(200);
	arr = efl_malloc(30*sizeof(int));
	datacheck(base, test);
	printheap();
	printf("\n");

	efl_free(str);
	datacheck(base, test);
	printheap();
	printf("\n");

	efl_free(arr);
	datacheck(base, test);
	printheap();
	printf("\n");

	return 0;
}
