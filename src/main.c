#include "dbf.h"

#include <err.h>
#include <ctype.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>

#define MAX_STEP 6

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
    printw("Coords (x, y):\n");
    printw("Output:");
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(1));
    attron(A_BOLD);
    mvprintw(0, 17, "%d", ti);
    mvprintw(1, 17, "%d", tv);
    mvprintw(2, 17, "%c", d);
    mvprintw(3, 17, "(%d, %d)", x, y);

    /* If we try to print normally while in debug mode with ncurses enabled,
     * it fucks up the formatting, and skews everything. Unfortunately we
     * have to handle '.' differently if we are in debug mode. */
    if (dbf.lines[y][x] == '.') {
        // print the character representation of the cell if able...
        if (isprint(tv))
            mvprintw(4, 17, "'%c'", tv);
        // ... otherwise print the hex value
        else
            mvprintw(4, 17, "0x%X", tv);
    }
    else
        mvprintw(4, 17, "(none)");

    attroff(A_BOLD);
    attroff(COLOR_PAIR(1));

    /* Scrolling window debug, useful for programs larger than 1 page */
    move(5, 0);
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
        mvprintw(LINES-1, COLS-14, "Autostep (%d/%d)", step, MAX_STEP);
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
            if (step == MAX_STEP)
                step = 0;
            else
                step += 1;
        }
    }
}

void usage(const char *name)
{
    fprintf(stderr, "Usage: %s [-d] file", name);
}

int main(int argc, char *argv[])
{
    struct dbf_t dbf;
    int debug = 0;
    int ch;

    if (argc == 1) {
        usage(argv[0]);
        return 1;
    }

    while ((ch = getopt(argc, argv, "hd")) != -1) {
        switch (ch) {
        case 'h':
            usage(argv[0]);
            return 0;
        case 'd':
            debug = 1;
            break;
        case '?':
        default:
            usage(argv[0]);
            return 1;
        }
    }

    if (argc == optind) {
        usage(argv[0]);
        return 1;
    }

    /* from file.c
     * Read from the first argument into a dbf_t struct */
    dbf = read_dbf_from_file(argv[optind]);
    if (dbf.status != SUCCESS) {
        printf("An error occured.\nError code: %d\n", dbf.status);
        free_dbf(dbf);
        return 1;
    }

    char tape[TAPE_LEN] = { 0 }; /* TAPE_LEN from dbf.h */
    int pointer = 0;
    int x = 0;
    int y = 0;
    char direction = '>';
    char t;

    /* Set up ncurses if the debug flag is passed on command line. */
    if (debug) {
        initscr();
        use_default_colors();
        start_color();
        curs_set(0);
    }

    /* Main logic behind directional brainfuck :3 */
    for (; y >= 0 && y < dbf.num_lines && x >= 0 && x < dbf.line_len;) {
        t = dbf.lines[y][x];

        /* 'e' allows the dbf program to specify an exit point, which is
         * pretty cool. */
        if (t == 'e')
            break;

        /* Interpret directional brainfuck :3 */
        switch (t) {
        case '^':
        case 'v':
        case '>':
        case '<':
            direction = t;
            break;
            /* Conditional. If the current cell is 0, direction becomes,
             * otherwise v. */
        case '?':
            if (tape[pointer] == 0)
                direction = '^';
            else
                direction = 'v';
            break;
        case '}':
            if (pointer >= TAPE_LEN - 1)
                pointer = 0;
            else
                pointer += 1;
            break;
        case '{':
            if (pointer == 0)
                pointer = TAPE_LEN - 1;
            else
                pointer -= 1;
            break;
        case '+':
            tape[pointer] += 1;
            break;
        case '-':
            tape[pointer] -= 1;
            break;
        case '.':
            /* Printing while in debug mode screws up the whole
               debug view, offsetting the rest of the screen, so
               dbfbug() will handle printing while in debug mode */
            if (!debug)
                putchar(tape[pointer]);
            break;
        case ',':
            if (t == ',')
                tape[pointer] = getchar();
            break;

        case 'd':
            /* Toggle debug mode. If we're currently in debug mode, a 'd'
             * in the file will disable it, and vice versa. We should set
             * up and end ncurses as needed. */
            if (debug)
                endwin();
            else {
                initscr();
                use_default_colors();
                start_color();
            }

            debug = !debug;
            break;
        default:
            break;
        }

        /* Move forward one space in whichever direction. */
        if (direction == '^')
            y -= 1;
        else if (direction == 'v')
            y += 1;
        else if (direction == '>')
            x += 1;
        else if (direction == '<')
            x -= 1;
        else {
            warnx("unexpected direction: %c (0x%x)",
                  direction, direction);
            break;
        }

        /* We need - 1, because it will attempt to access out of bounds
         * due to arrays being 0-indexed. This causes a segfault. */
        if (y > dbf.num_lines - 1 || y < 0 ||
            x > dbf.line_len - 1 || x < 0)
        {
            warnx("out of bounds: %dx%d, dir: %c", x, y, direction);
            break;
        }

        if (debug)
            dbfbug(dbf, x, y, pointer, tape[pointer], direction);
    }

    free_dbf(dbf); /* from file.c */
    /* End ncurses shit */
    if (debug) {
        curs_set(1);
        endwin();
    }

    return 0;
}
