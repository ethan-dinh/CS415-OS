#include "hashset.h"  /* the .h file does NOT reside in /usr/local/include/ADTs */
#include <stdlib.h>
#include <stdio.h>

/* any other includes needed by your code */
#define UNUSED __attribute__((unused))
#define TRIGGER 100

typedef struct node {
	struct node *next;
    void *value;	
} Node;

typedef struct s_data {
    /* definitions of the data members of self */
	long (*hash)(void *, long N);
	int (*cmp)(void *, void *);
	long size;
	long capacity;
	long changes;
	double load;
	double loadFactor;
	double increment;
	Node **buckets;
	void (*freeV)(void *v);
} SData;

/*
 * important - remove UNUSED attributed in signatures when you flesh out the
 * methods
 */

/* Helper function to remove all of the values and free their memory */
static void purge(SData *sd) {
	// Traverses the pointer array and frees each value
	for (int i = 0; i < sd->capacity; i++) {
		Node *p, *q;
		p = sd -> buckets[i];

		// Traverses the linked list
		while (p != NULL) {
			sd -> freeV(p->value);
			q = p -> next;
			free(p);
			p = q;
		} sd -> buckets[i] = NULL;
	}
}

static void s_destroy(const Set *s) {
    /* implement the destroy() method */
	SData *sd = (SData *) s->self;
	purge(sd);
	free(sd->buckets);
	free(sd);
	free((void *)s);
}

static void s_clear(const Set *s) {
    /* implement the clear() method */
	SData *sd = (SData *) s->self;
	purge(sd);
	sd->size = 0;
	sd->load = 0.0;
	sd->changes = 0;
}

/* Helper function to resize the map array */
static void resize(SData *sd) {
	long N;
	Node *p, *q, **array;
	long i, j;

	// Doubles the capacity of the map array
	N = 2 * sd->capacity;
	array = (Node **)malloc(N * sizeof(Node *));
	if (array == NULL)
		return;

	// Sets all of the values of the array to NULL
	for (j = 0; j < N; j++)
		array[j] = NULL; 

	// Copies the original array to the new array
	// and reindexes the stored nodes
	for (i = 0; i < sd->capacity; i++) {
		for (p = sd->buckets[i]; p != NULL; p = q) {
			q = p -> next;
			j = sd->hash(p->value, N);
			p->next = array[j];
			array[j] = p;
		}
	}

	free(sd->buckets);
	sd->buckets = array;
	sd->capacity = N;
	sd->load /= 2.0;
	sd->changes = 0;
	sd->increment = 1.0 / (double) N;
}

/* Helper function to return the node with the given value */
static Node* find_node(SData *sd, void *member) {
	Node *p;

	// Hashes the value and stores the value as in index
	long i = sd->hash(member, sd->capacity);
	for (p = sd->buckets[i]; p != NULL; p = p->next) {
		// Checks to which node in the linked list has the same value
		if (sd->cmp(p->value, member) == 0) {
			return p;
		}
	} return NULL;
}

static bool s_add(const Set *s, void *member) {
    /* implement the add() method */
	SData *sd = (SData *) s->self;
	Node *p = (Node *)malloc(sizeof(Node));
	
	bool status = (p != NULL);
	bool in_bucket = (find_node(sd, member) != NULL);

	if (sd->changes > TRIGGER) {
		sd->changes = 0;
		if (sd->load > sd->loadFactor)
			resize(sd);
	}

	if (!in_bucket && status) {
		long i = sd->hash(member, sd->capacity);	
		p->value = member;
		p->next = sd->buckets[i];
		sd->buckets[i] = p;
		sd->size++;
		sd->load += sd->increment;
		sd->changes++;
	} else {
		free(p);
		status = false;
	}

	return status;	
}

static bool s_contains(const Set *s, void *member) {
    /* implement the contains() method */
	SData *sd = (SData *) s->self;
	return (find_node(sd, member) != NULL);
}

static bool s_isEmpty(const Set *s) {
    /* implement the isEmpty() method */
	SData *sd = (SData *) s->self;
    return (sd->size == 0L);
}

static bool s_remove(const Set *s, void *member) {
    /* implement the remove() method */
    SData *sd = (SData *) s->self;

	// Determines the node that the member belongs to
	Node *entry = find_node(sd, member);

	// Determine the index of the input value using the hash fxn
	long i = sd->hash(member, sd->capacity);
	bool status = (entry != NULL);

	if (status) {
		Node *p, *c;
		// Traverses the pointer to the target value
		for (p = NULL, c = sd->buckets[i]; c != entry; p = c, c = c->next)
			;

		// Checks if the member is the first node
		if (p == NULL)
			sd->buckets[i] = entry->next;
		else
			p->next = entry->next;
		
		sd->size--;
		sd->load -= sd->increment;
		sd->changes++;

		// Clears the node from the heap
		sd->freeV(entry->value);
		free(entry);
	}
	return status;
}

static long s_size(const Set *s) {
    /* implement the size() method */
	SData *sd = (SData *) s->self;
    return sd->size;
}

/* Helper function to create value array from map */
static void **values(SData *sd) {
	void **tmp = NULL;
	if (sd->size > 0L) {
		// Declare the total size of the new malloced array
		size_t nbytes = sd->size * sizeof(void *);
		tmp = (void **)malloc(nbytes);
		if (tmp != NULL) {
			long i, n = 0L;
			for (i = 0L; i < sd->capacity; i++) {
				// Copies the nodes and all its subsequent children
				Node *p = sd->buckets[i];
				while (p != NULL) {
					tmp[n++] = p->value;
					p = p -> next;
				}
			}
		}
	}
	return tmp;
}

static void **s_toArray(UNUSED const Set *s, UNUSED long *len) {
    /* implement the toArray() method */
	SData *sd = (SData *) s->self;
	void **tmp = values(sd);

	if (tmp != NULL)
		*len = sd->size;
	return tmp;
}

static const Iterator *s_itCreate(UNUSED const Set *s) {
    /* implement the itCreate() method */
    SData *sd = (SData *) s->self;
	const Iterator *it = NULL;
	void **tmp = (void **)values(sd);

	if (tmp != NULL) {
		// Uses the array and the array size to create the iterator
		it = Iterator_create(sd->size, tmp);
		if (it == NULL)
			free(tmp);
	}
	return it;
}

static const Set *s_create(const Set *s);

static UNUSED Set template = {
    NULL, s_destroy, s_clear, s_add, s_contains, s_isEmpty, s_remove,
    s_size, s_toArray, s_itCreate
};

/*
 * Helper Function to create a new Set dispatch table
 */

static const Set *newSet(void (*freeValue)(void*), int (*cmpFxn)(void*, void*), long capacity,
						 double loadFactor, long (*hashFxn)(void *m, long N)) {
	
	Set *s = (Set *)malloc(sizeof(Set));
	long N;
	double lf;
	Node **array;

	if (s != NULL) {
		SData *sd = (SData *)malloc(sizeof(SData));
		if (sd != NULL) {
			N = ((capacity > 0) ? capacity : DEFAULT_SET_CAPACITY);
			lf = ((loadFactor > 0.000001) ? loadFactor : DEFAULT_LOAD_FACTOR);
			array = (Node **)malloc(N * sizeof(Node *));
			if (array != NULL) {
				sd -> capacity = N; sd -> size = 0L; sd -> changes = 0L;
				sd -> loadFactor = lf; sd -> load = 0.0;
				sd -> increment = 1.0 / (double) N;
				sd -> hash = hashFxn; sd -> cmp = cmpFxn;
				sd -> freeV = freeValue;
				sd -> buckets = array;

				for (int i = 0; i < N; i++) {
					array[i] = NULL;
				} 

				*s = template;
				s -> self = sd;
			} else {
				free(sd); free(s); s = NULL;
			}
		} else {
			free(s); s = NULL;
		}	
	} 
	return s;
}

static const Set *s_create(const Set *s) {
	SData *sd = (SData *) s->self;
	return newSet(sd->freeV, sd->cmp, sd->capacity, sd->loadFactor, sd->hash);
}

const Set *HashSet(void (*freeValue)(void*), int (*cmpFxn)(void*, void*), long capacity, 
				   double loadFactor, long (*hashFxn)(void *m, long N)) {
    
	/* construct a Set instance and return to the caller */
    return newSet(freeValue, cmpFxn, capacity, loadFactor, hashFxn);
}
