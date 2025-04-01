#ifndef DBF_H
#define DBF_H

#include <stdio.h>

#define TAPE_LEN 30000 // change this to increase/decrase tape length
                       //#define dbf_default { 0, 0, NULL, 0 }
#define DEBUG 0

#if defined(DEBUG) && DEBUG > 0
#define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
        __FILE__, __LINE__, __func__, ##args)
#else
#define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

typedef enum {
    SUCCESS,
    FILE_ERROR,
    MEM_ERROR,
    OTHER_ERROR
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
