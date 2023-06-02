#include "row.h"
#include "strheap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct row_data {
    char **theFields;
    unsigned long nFields;
}RowData;

#define MAXFIELDS 250

static RowData *split(char *csvline) {
    char *fields[MAXFIELDS], **theFields, *token;
    unsigned long i = 0, j;
    char *theLine = strdup(csvline);
    char *line = theLine;
    RowData *rd = (RowData *)malloc(sizeof(RowData));

    while ((token = strtok_r(line, ",", &line)) != NULL)
        fields[i++] = str_malloc(token);
    free(theLine);
    rd->nFields = i;
    theFields = (char **)malloc(i * sizeof(char *));
    for (j = 0; j < i; j++)
        theFields[j] = fields[j];
    rd->theFields = theFields;
    return rd;
}

static void r_destroy(const Row *r) {
    RowData *rd = (RowData *)r->self;
    unsigned long i;
    for (i = 0; i < rd->nFields; i++)
        str_free(rd->theFields[i]);
    free(rd->theFields);
    free(rd);
    free((void *)r);
}

static bool r_field(const Row *r, unsigned long index, char **value) {
    RowData *rd = (RowData *)r->self;
    if (index >= rd->nFields)
        return false;
    *value = rd->theFields[index];
    return true;
}

static void r_csvline(const Row *r, char *buffer) {
    RowData *rd = (RowData *)r->self;
    unsigned long i;
    char *p = buffer;
    p += sprintf(p, "%s", rd->theFields[0]);
    for (i = 1; i < rd->nFields; i++)
        p += sprintf(p, ",%s", rd->theFields[i]);
}

static unsigned long r_size(const Row *r) {
    RowData *rd = (RowData *)r->self;
    return rd->nFields;
}

static Row template = {NULL, r_destroy, r_field, r_csvline, r_size};

const Row *Row_create(char *csv_line) {
    Row *r;

    if ((r = (Row *)malloc(sizeof(Row))) != NULL) {
        RowData *rd = split(csv_line);
	*r = template;
	r->self = rd;
    }
    return r;
}
