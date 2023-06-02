#ifndef _ROW_H_
#define _ROW_H_

#include <stdbool.h>

typedef struct row Row;
const Row *Row_create(char *csv_line);
struct row {
    void *self;
    void (*destroy)(const Row *r);
    bool (*field)(const Row *r, unsigned long index, char **value);
    void (*csvline)(const Row *r, char *value);
    unsigned long (*size)(const Row *r);
};

#endif /* _ROW_H_ */
