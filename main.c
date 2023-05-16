#include "dbf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Print a map of the field, displaying an 'x' for our current position. */
void dbfbug(struct dbf_t dbf, int x, int y, int ti, int tv, char d)
{
	printf("\x1b[2J\x1b[H");
	printf("Tape index:\t%d\n", ti);
	printf("Current cell:\t%d\n", tv);
	printf("Direction:\t%c\n", d);
	printf("XY: %d/%d\n", x, y);

	for (int i = 0; i < dbf.num_lines; i++) {
	/* Scrolling window debug, useful for programs larger than 1 page */
	//for (int i = (y - 15 > 0) ? y - 15 : 0; i < y + 15; i++) {
		for (int n = 0; n < dbf.line_len; n++) {
			if (i == y && n == x)
				putchar('x');
			else
				putchar(dbf.lines[i][n]);
		}
		putchar('\n');
	}

	printf("\nPress any key to continue.\n");
	getchar();
}

int main(int argc, char *argv[])
{
	struct dbf_t dbf;
	int debug;

	if (argc == 1) {
		printf("Usage: ./%s [file] (-d)", argv[0]);
		return 1;
	}

	/* TODO: *actually* check if argv[2] is "-d", currently any
	 * option will enable debug mode. */
	if (argc > 2)
		debug = 1;
	else
		debug = 0;

	/* from file.c
	 * Read from the first argument into a dbf_t struct */
	dbf = read_dbf_from_file(argv[1]);
	if (dbf.error != 0) {
		printf("An error occured. (%d)\n", dbf.error);
		return 1;
	}

	DEBUG_PRINT("Initializing vars\n");
	char tape[TAPE_LEN] = { 0 }; /* TAPE_LEN from dbf.h */
	int pointer = 0;
	int x = 0;
	int y = 0;
	char d = '>';
	char t;

	DEBUG_PRINT("Loop begin.\n");
	for (;;) {
		DEBUG_PRINT("Check bounds\n");
		DEBUG_PRINT("y > dbf.num_lines - 1 || y < 0 || x > dbf.line_len - 1|| x < 0\n");
		DEBUG_PRINT("%d > %d || %d < 0 || %d > %d || %d < 0\n",
			y, dbf.num_lines - 1, y, x, dbf.line_len - 1, x);

		/* We need - 1, because it will attempt to access out of bounds due to arrays
		 * being 0-indexed. This causes a segmentation fault. */
		if (y > dbf.num_lines - 1 || y < 0 || x > dbf.line_len - 1 || x < 0) {
			DEBUG_PRINT("*** Outta bounds:\n");
			DEBUG_PRINT("y: %d\n", y);
			DEBUG_PRINT("x: %d\n", x);
			DEBUG_PRINT("direction: %c\n", d);
			break;
		}

		DEBUG_PRINT("dbf.lines[y][x] = '%c'\n", dbf.lines[y][x]);
		t = dbf.lines[y][x];
		/* Change direction */
		if (t == '^' || t == 'v' || t == '>' || t == '<')
			d = t;
		/* Conditional. If 0, direction becomes ^, otherwise v. */
		if (t == '?') {
			if (tape[pointer] == 0)
				d = '^';
			else
				d = 'v';
		}
		if (t == '}')
			pointer += 1;
		if (t == '{')
			pointer -= 1;
		if (t == '+')
			tape[pointer] += 1;
		if (t == '-')
			tape[pointer] -= 1;
		if (t == '.')
			putchar(tape[pointer]);
		if (t == ',')
			tape[pointer] = getchar();
		if (t == 'd')
			debug = !debug;
		if (t == 'e')
			break;

		/* Move forward one space in whichever direction. */
		if (d == '^')
			y -= 1;
		if (d == 'v')
			y += 1;
		if (d == '>')
			x += 1;
		if (d == '<')
			x -= 1;

		if (debug)
			dbfbug(dbf, x, y, pointer, tape[pointer], d);
	}

	DEBUG_PRINT("Freeing...\n");
	free_dbf(dbf); /* from file.c */
	DEBUG_PRINT("Done.\n");

	return 0;
}
