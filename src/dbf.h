#ifndef DBF_H
#define DBF_H

#include <stdio.h>

#define TAPE_LEN 30000 // change this to increase/decrase tape length

typedef enum {
    SUCCESS,
    ERROR
} Status;

struct dbf_t {
    int num_lines;
    int line_len;
    char **lines;
    Status status;
};

int count_lines(FILE *);
int free_dbf(struct dbf_t);
struct dbf_t read_dbf_from_file(char *);

#endif
