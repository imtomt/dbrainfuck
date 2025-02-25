#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbf.h"

/*
 * Initialize a new dbf struct.
 */
struct dbf_t init_dbf()
{
	struct dbf_t dbf;
	dbf.num_lines = 0;
	dbf.line_len = 0;
	dbf.lines = NULL;
	dbf.status = SUCCESS;

	return dbf;
}

/*
 * Return the number of lines from a file given as argument */
int count_lines(FILE *fp)
{
	int lines = 0;
	int ch = 0;

	for (ch = getc(fp); ch != EOF; ch = getc(fp)) {
		if (ch == '\n')
			lines += 1;
	}

	rewind(fp);
	return lines;
}

/* dbf.lines gets alloced and realloced, so we need to
 * free everything. */
int free_dbf(struct dbf_t dbf)
{
	for (int i = 0; i < dbf.num_lines; i++)
		free(dbf.lines[i]);
	free(dbf.lines);

	return 0;
}

/* Read from a file, given as a string argument, and process for dbf.
 * Dynamically allocate a double pointer (array of strings) to accomodate
 * exactly the right amount of memory, regardless of file size.
 *
 * We then pad every line with trailing spaces, so each line is the same
 * length as the longest line. That's necessary to move up and down
 * through lines. */
struct dbf_t read_dbf_from_file(char *file)
{
	struct dbf_t result = init_dbf();
	FILE *fp = fopen(file, "r");
	if (fp == NULL) {
		DEBUG_PRINT("*** Could not open file %s.\n", file);
		result.status = FILE_ERROR;
		return result;
	}

	int num_lines = count_lines(fp);
	/* Allocate enough space for the number of lines. Each line
	 * will be malloc'ed to fit the line length later. */
	char **lines = (char **)malloc(num_lines * sizeof(char *));

	if (lines == NULL) {
		DEBUG_PRINT("*** Could not allocate memory.\n");
		result.status = MEM_ERROR;
		return result;
	}

	char buf[1024];
	int len = 0;
	int maxlen = 0;

	DEBUG_PRINT("Reading lines\n");

	/* Read each line of the file to get the longest line length, so we can
	   pad each line in the future. */
	for (int i = 0; i < num_lines; i++) {
		if (fgets(buf, sizeof(buf), fp) != NULL) {
			len = strlen(buf);
			if (len > maxlen)
				maxlen = len;
		}
		else if (feof(fp)) {
			break;
		}
		else {
			DEBUG_PRINT("*** File error\n");
			result.status = FILE_ERROR;
			return result;
		}
	}

	// Rewind the file to start at the beginning
	fseek(fp, 0, SEEK_SET);

	/* Pad each line to the same length, allocating just enough memory. */
	for (int i = 0; i < num_lines; i++) {
		if (fgets(buf, sizeof(buf), fp) != NULL) {
			len = strlen(buf);

			/* Replace newlines with NULL, we don't want \n in our
			   actual tape. */
			if (buf[len-1] == '\n')
				buf[len-1] = ' ';

			lines[i] = (char *)malloc((maxlen + 1) * sizeof(char));
			if (lines == NULL) {
				DEBUG_PRINT("*** Could not allocate memory.\n");
				result.status = MEM_ERROR;
				return result;
			}

			strncpy(lines[i], buf, len + 1);

			/* Pad the lines to all the same length, using spaces */
			for (int c = len; c < maxlen; c++)
				lines[i][c] = ' ';

			lines[i][maxlen] = '\0';
		}
		else {
			DEBUG_PRINT("*** Unexpected EOF\n");
			result.status = FILE_ERROR;
			return result;
		}
	}

	fclose(fp);

	/* Populate the dbf struct */
	result.num_lines = num_lines;
	result.line_len = maxlen;
	result.lines = lines;

	return result;
}
