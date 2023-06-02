#include "table.h"
#include <stdlib.h>

#define DEFAULT_CAPACITY 16UL

typedef struct tbl_data {
    unsigned long size;
    unsigned long capacity;
    Row **rows;
    void (*freeValue)(void *x);
} TblData;

static void t_destroy(const Table *t) {
    TblData *self = (TblData *)t->self;
    unsigned long i;

    for (i = 0UL; i < self->size; i++)
        self->freeValue(self->rows[i]);
    free(self->rows);
    free(self);
    free((void *)t);
}

static bool t_append(const Table *t, const Row *r) {
    TblData *self = (TblData *)t->self;
    bool response;
    unsigned long index = self->size;
    if (index >= self->capacity) {	/* resize the rows array */
        unsigned long newcap = 2 * self->capacity;
	Row **newrows = (Row **)realloc(self->rows, newcap*sizeof(Row *));
	if (newrows != NULL) {
            self->capacity = newcap;
	    self->rows = newrows;
	}
    }
    if (index < self->capacity) {
        self->rows[index] = (Row *)r;
	self->size++;
        response = true;
    } else
        response = false;
    return response;
}

static bool t_retrieve(const Table *t, unsigned long n, Row **r) {
    TblData *self = (TblData *)t->self;
    bool response;
    if (n < self->size) {
        response = true;
	*r = self->rows[n];
    } else
        response = false;
    return response;
}

static bool t_remove(const Table *t, unsigned long n) {
    TblData *self = (TblData *)t->self;
    bool response;
    if (n < self->size) {
        unsigned long i;
	Row *r = self->rows[n];
	self->freeValue(r);
        for (i = n + 1; i < self->size; i++)
            self->rows[i-1] = self->rows[i];
	self->size--;
	response = true;
    } else
        response = false;
    return response;
}

static long t_size(const Table *t) {
    TblData *self = (TblData *)t->self;
    return (long)self->size;
}

static Row **copyArray(TblData *self) {
    Row **tmp = (Row**) malloc(self->size * sizeof(Row *));
    unsigned long i;

    if (tmp != NULL)
        for (i = 0UL; i < self->size; i++)
            tmp[i] = self->rows[i];
    return tmp;
}

static Row **t_toArray(const Table *t, unsigned long *len) {
    TblData *self = (TblData *)t->self;
    Row **theArray = copyArray(self);
    if (theArray != NULL)
        *len = self->size;
    return theArray;
}

static const Iterator *t_itCreate(const Table *t) {
    TblData *self = (TblData *)t->self;
    Row **theArray = copyArray(self);
    const Iterator *it;
    
    if (theArray != NULL) {
        it = Iterator_create(self->size, (void **)theArray);
	if (it == NULL)
            free(theArray);
    }
    return it;
}

static Table template = {
    NULL, t_destroy, t_append, t_retrieve, t_remove,
    t_size, t_toArray, t_itCreate
};

const Table *Table_create(void (freeValue(void *x))) {
    Table *t = (Table *)malloc(sizeof(Table));

    if (t != NULL) {
        TblData *td = (TblData *)malloc(sizeof(TblData));
	if (td != NULL) {
            Row **rows = (Row **)malloc(DEFAULT_CAPACITY * sizeof(Row *));
	    if (rows != NULL) {
                td->size = 0UL;
		td->capacity = DEFAULT_CAPACITY;
		td->rows = rows;
		td->freeValue = freeValue;
		*t = template;
		t->self = td;
	    } else {
                free(td);
		free(t);
		t = NULL;
	    }
	} else {
            free(t);
	    t = NULL;
	}
    }
    return t;
}
