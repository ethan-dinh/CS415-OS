#ifndef _TABLE_H_
#define _TABLE_H_

#include "row.h"
#include "ADTs/iterator.h"

typedef struct table Table;
const Table *Table_create(void (freeValue(void *x)));
struct table {
    void *self;
    void (*destroy)(const Table *t);
    bool (*append)(const Table *t, const Row *r);
    bool (*retrieve)(const Table *t, unsigned long n, Row **r);
    bool (*remove)(const Table *t, unsigned long n);
    long (*size)(const Table *t);
    Row **(*toArray)(const Table *t, unsigned long *len);
    const Iterator *(*itCreate)(const Table *t);
};

#endif /* _TABLE_H_ */
