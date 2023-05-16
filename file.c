#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbf.h"

/* Return the number of lines from a file given as argument */
int count_lines(FILE *fp)
{
	int lines = 0;
	int ch = 0;

	while (!feof(fp)) {
		ch = fgetc(fp);
		if (ch == '\n')
			lines++;
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
	struct dbf_t result = dbf_default;
	FILE *fp = fopen(file, "r");
	if (fp == NULL) {
		DEBUG_PRINT("Error 1\n");
		result.error = 1;
		return result;
	}

	int num_lines = count_lines(fp);
	/* Allocate enough space for the number of lines. Each line
	 * will be malloc'ed to fit the line length later. */
	char **lines = (char **)malloc(num_lines * sizeof(char *));

	if (lines == NULL) {
		DEBUG_PRINT("Error 2\n");
		result.error = 2;
		return result;
	}

	char buf[1024];
	int len = 0;
	int maxlen = 0;

	DEBUG_PRINT("Reading lines\n");
	/* Read each line from the file into an array */
	for (int i = 0; i < num_lines; i++) {
		/* first it gets read into buf, to determine the length of the line
		 * so enough space can be allocated. */
		if (fgets(buf, sizeof(buf), fp) != NULL) {
			DEBUG_PRINT("Not null.\n");
			len = strlen(buf);

			/* Remove newline if present */
			if (buf[len-1] == '\n') {
				DEBUG_PRINT("Newline (buf[%d-1] == '\\n')\n", len);
				buf[len-1] = '\0';
			}

			DEBUG_PRINT("malloc(%d);\n", (len+1)*sizeof(char));
			/* allocate proper amount of space per line */
			lines[i] = (char *)malloc((len + 1) * sizeof(char));
			if (lines[i] == NULL) {
				DEBUG_PRINT("Error 2\n");
				result.error = 2;
				return result;
			}
			strcpy(lines[i], buf);

			/* Keep track of maximum line length */
			if (len > maxlen)
				maxlen = len;
		}
	}
	fclose(fp);

	/* Pad all lines to be same length */
	DEBUG_PRINT("Padding lines\n");
	for (int i = 0; i < num_lines; i++) {
		DEBUG_PRINT("len = %d\n", strlen(lines[i]));
		len = strlen(lines[i]);

		DEBUG_PRINT("%d < %d? %s\n", len, maxlen, (len < maxlen) ? "true" : "false");
		/* the current line length is shorter than the longest line... */
		if (len < maxlen) {
			/* ...realloc enough space to be filled with spaces. */
			lines[i] = realloc(lines[i], maxlen * sizeof(char));
			if (lines[i] == NULL) {
				DEBUG_PRINT("Error 2\n");
				result.error = 2;
				return result;
			}
		}

		/* add spaces to the end of the line, until the max length */
		for (int c = len; c < maxlen; c++)
			lines[i][c] = ' ';

		DEBUG_PRINT("Done.\n");
		lines[i][maxlen] = '\0';
	}

	/* Populate the dbf struct */
	result.num_lines = num_lines;
	result.line_len = maxlen;
	result.lines = lines;

	DEBUG_PRINT("All set.\n");
	return result;
}
