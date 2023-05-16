#include "dbf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

//static int step = 1;

/* Print a map of the field, displaying an 'x' for our current position. */
void dbfbug(struct dbf_t dbf, int x, int y, int ti, int tv, char d)
{
	char c;
	int delay = 35;
	static int step = 0;

	init_pair(1, COLOR_RED, -1);
	init_pair(2, COLOR_GREEN, -1);
	init_pair(3, 9, -1);
	init_pair(4, 7, -1);

	erase();
	move(0, 0);

	attron(COLOR_PAIR(2));
	printw("Tape Index:\n");
	printw("Current Cell:\n");
	printw("Direction:\n");
	printw("Coords (x, y):");
	attroff(COLOR_PAIR(2));

	attron(COLOR_PAIR(1));
	attron(A_BOLD);
	mvprintw(0, 17, "%d", ti);
	mvprintw(1, 17, "%d", tv);
	mvprintw(2, 17, "%c", d);
	mvprintw(3, 17, "(%d, %d)", x, y);
	attroff(A_BOLD);
	attroff(COLOR_PAIR(1));

	/* Scrolling window debug, useful for programs larger than 1 page */
	move(4, 0);
	for (int i = (dbf.num_lines > LINES-10) ? y/2 : 0; i < dbf.num_lines; i++) {
		for (int n = 0; n < dbf.line_len; n++) {
			if (i == y && n == x) {
				attron(COLOR_PAIR(3));
				attron(A_BOLD);
				addch('X');
				attroff(COLOR_PAIR(3));
				attroff(A_BOLD);
			}
			else {
				attron(COLOR_PAIR(4));
				addch(dbf.lines[i][n]);
				attroff(COLOR_PAIR(4));
			}
		}
		addch('\n');
	}

	move(LINES-3, 0);
	attron(COLOR_PAIR(2));
	attron(A_BOLD);
	printw("Press `r` to automatically step through.\n");
	printw("Press `r` again to cycle speeds..\n");
	printw("Press any key to continue.\n");
	if (step == 0)
		mvprintw(LINES-1, COLS-15, "Manual stepping");
	else
		mvprintw(LINES-1, COLS-14, "Autostep (%d/4)", step);
	attroff(A_BOLD);
	attroff(COLOR_PAIR(2));
	refresh();

	if (step == 0) {
		timeout(-1);
		c = getch();
		if (c == 'r') {
			step = 1;
		}
	}
	else {
		timeout(delay/step);
		c = getch();
		if (c == 'r') {
			if (step == 4)
				step = 0;
			else
				step += 1;
		}
	}
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

	if (debug) {
		initscr();
		use_default_colors();
		start_color();
		curs_set(0);
	}

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
		if (t == 'd') {
			//debug = !debug;
			if (debug)
				endwin();
			else {
				initscr();
				use_default_colors();
				start_color();
			}

			debug = !debug;
		}
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
	if (debug) {
		curs_set(1);
		endwin();
	}
	DEBUG_PRINT("Done.\n");

	return 0;
}
