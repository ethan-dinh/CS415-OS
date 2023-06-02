#include "strheap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * functions for a ref-counted string heap
 *
 * for use in programs that store multiple copies of the same string on the
 * heap
 *
 * a unique string will only be stored once on the heap, and a reference
 * count is kept of the number of times that string has been str_malloc()ed.
 *
 * each str_free() causes the associated reference count to be decremented.
 *
 * when the reference count reaches 0, the string is purged from the heap
 */

/*
 * "duplicate" `string' on the string heap
 * returns the address of the heap allocated string or NULL if malloc() errors
 */
char *str_malloc(char *string) {
    char *p = strdup(string);
    if (p == NULL) {
        fprintf(stderr, "str_malloc() failure\n");
	abort();
    }
    return p;
}

/*
 * "free" `string' on the string heap
 * returns true if free'd, or false if the string was not present on the string
 * heap
 */
bool str_free(char *string) {
    free(string);
    return true;
}
