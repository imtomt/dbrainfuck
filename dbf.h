#ifndef DBF_H
#define DBF_H

#include <stdio.h>

#define TAPE_LEN 4096 // change this to increase/decrase tape length
#define dbf_default { 0, 0, NULL, 0 }
#define DEBUG 0

#if defined(DEBUG) && DEBUG > 0
 #define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
    __FILE__, __LINE__, __func__, ##args)
#else
 #define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif


struct dbf_t {
	int num_lines;
	int line_len;
	char **lines;
	int error;	/* 0 = no error, 1 = file error, 2 = memory error */
};

int count_lines(FILE *);
int free_dbf(struct dbf_t);
struct dbf_t read_dbf_from_file(char *);

#endif
