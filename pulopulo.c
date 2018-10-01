#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <ncurses.h>
#include <assert.h>
#include <locale.h>

#include "pulopulo.h"

#define PROJECT_NAME "pulopulo"

/* initialize the game elements */
void init_game(struct game* g) {
    g->frame = 0;
    g->puyo_count = 0;
    g->removable_count = 0;
    g->x_offset_switch = false;
    g->prev_offset.x = 0; g->prev_offset.y = 0;
}

/* draw the puyos on the board. multiply x pos by 2 for better spacing */
void print_puyos(struct game* g) {
    for (int i=0; i < g->puyo_count; i++) {
        attron(COLOR_PAIR(g->puyos[i].color));
        mvaddwstr(g->puyos[i].pos.y, g->puyos[i].pos.x * 2, PUYO_CHAR);
        attroff(COLOR_PAIR(g->puyos[i].color));
    }
    move(BOARD_Y, BOARD_X * 2);
}

/* print board characters in shape of a puyo board */
void print_drawable(struct game* g) {
    for (int y = 0; y < BOARD_Y + 1; y++) {
        for (int x = 0; x < BOARD_X * 2 + 1; x++) {
            if (y == 0) {
                if (x == 0) mvaddch(y, x, TOP_LEFT);
                else if (x == BOARD_X * 2) mvaddch(y, x, TOP_RIGHT);
                else mvaddch(y, x, HORIZONTAL);
            } else if (y == BOARD_Y) {
                if (x == 0) mvaddch(y, x, BOTTOM_LEFT);
                else if (x == BOARD_X * 2) mvaddch(y, x, BOTTOM_RIGHT);
                else mvaddch(y, x, HORIZONTAL);
            } else {
                if (x == 0) mvaddch(y, x, VERTICAL);
                else if (x == BOARD_X * 2) mvaddch(y, x, VERTICAL);
            }
        }
    }
}

/* get x and y offset based on direction */
point get_offset(direction dir) {
    point p;
    switch(dir) {
        case UP:
            p.x = 0; p.y = -1;
            break;
        case DOWN:
            p.x = 0; p.y = 1;
            break;
        case LEFT:
            p.x = -1; p.y = 0;
            break;
        case RIGHT:
            p.x = 1; p.y = 0;
            break;
        case INVALID:
            break; 
    }
    return p;
}

/* return random color out of the 5 colors a puyo can be */
color_t random_color() {
    int i = rand() % 5;
    switch (i) {
        case 0: return RED; break;
        case 1: return GREEN; break;
        case 2: return BLUE; break;
        case 3: return PURPLE; break;
        case 4: return YELLOW; break;
    }
    return RED;
}

/* spawn new puyos */
void new_puyos(struct game* g) {
    puyo anchor;
    anchor.pos.x = 4; anchor.pos.y = 1;
    anchor.active = true;
    anchor.color = random_color();
    g->puyos[g->puyo_count] = anchor;
    g->anchor = g->puyo_count;
    g->puyo_count++;

    puyo child;
    child.pos.x = 4; child.pos.y = 2;
    child.active = true;
    child.color = random_color();
    g->puyos[g->puyo_count] = child;
    g->child = g->puyo_count;
    g->puyo_count++;

    g->prev_offset.x = -1; g->prev_offset.y = 1;
    g->x_offset_switch = false;
}

/* return the index of the puyo or -1 if there is none */
int space_occupied(struct game* g, int x, int y) {
    if (x == 0 || x == BOARD_X || y == BOARD_Y) return MAX_PUYOS + 1;
    for (int p = 0; p < g->puyo_count; p++) {
        if (g->puyos[p].pos.x == x && g->puyos[p].pos.y == y && !g->puyos[p].active) {
            return p;
        }
    }
    return -1;
}

/* recurse through every puyo and check for matching colors */
void match_puyos(struct game* g, struct puyo_buffer* b, int p) {
    direction dir[4] = {UP, DOWN, LEFT, RIGHT};
    for (int d = 0; d < 4; d++) {
        point offset = get_offset(dir[d]);
        int next = space_occupied(g, g->puyos[p].pos.x + offset.x, g->puyos[p].pos.y + offset.y);
        if (next < 0 || next == MAX_PUYOS + 1) continue;
        if (g->puyos[p].color == g->puyos[next].color) {
            if (b->count == 0) {
                b->buffer[b->count] = p;
                b->count++;
            }
            bool prev_match = false;
            for (int a = 0; a < b->count; a++) {
                if (b->buffer[a] == next) prev_match = true;
            }
            if (!prev_match) {
                b->buffer[b->count] = next;
                b->count++;
                match_puyos(g, b, next);
            }
        }
    }
}

void clear_puyos(struct game* g) {
    for (int p = 0; p < g->puyo_count; p++) {
        struct puyo_buffer b;
        b.count = 0;
        match_puyos(g, &b, p);

        if (b.count >= 4) {
            for (int r = 0; r < b.count; r++) {
                printf("%i\n", b.buffer[r]);
                for (int i = b.buffer[r]; i < g->puyo_count - 1; i++) {
                    g->puyos[i] = g->puyos[i + 1];
                }
                g->puyo_count--;
            }
        }
    }
}

/* check if any puyo will fail to move ahead of time */
bool buffer_move(struct game* g, point offset, bool include_anchor) {
    for (int p = 0; p < g->puyo_count; p++) {
        if (!include_anchor && p == g->anchor) continue;
        if (!g->puyos[p].active) continue;
        if (space_occupied(g, g->puyos[p].pos.x + offset.x, g->puyos[p].pos.y + offset.y) >= 0) {
            if (offset.y == 1 && offset.x == 0) g->puyos[p].active = false;
            else return false;
        }
    }
    return true;
}

/* check for any puyos on the board are active */
bool any_active_puyos(struct game* g) {
    for (int p = 0; p < g->puyo_count; p++) {
        if (g->puyos[p].active) return true;
    }
    return false;
}

/* update all active puyos in the given diretion */
bool update_puyo_pos(struct game* g, puyo* p, point offset) {
    if (space_occupied(g, p->pos.x + offset.x, p->pos.y + offset.y) < 0) {
        p->pos.x += offset.x;
        p->pos.y += offset.y;
        return false;
    }
    return true;
}

/* called each frame and on every input to change the board acordingling */
void update_board(struct game* g, point offset) {
    if (!buffer_move(g, offset, true)) return;
    for (int p = 0; p < g->puyo_count; p++) {
        if (g->puyos[p].active) {
            update_puyo_pos(g, &g->puyos[p], offset);
        }
    }
}

/* rotate active puyos acording to the anchor */
void rotate_puyos(struct game* g, bool cw) {
    point offset;
    if (g->x_offset_switch && cw) {
        offset.x = g->prev_offset.x * -1;
        offset.y = g->prev_offset.y;
    } else {
        offset.x = g->prev_offset.x;
        offset.y = g->prev_offset.y * -1;
    }
    if (buffer_move(g, offset, false)) {
        g->puyos[g->child].pos.x += offset.x;
        g->puyos[g->child].pos.y += offset.y;
        g->x_offset_switch = !g->x_offset_switch;
        g->prev_offset = offset;
    }
}

/* curses stuff */
void curses_init() {
    setlocale(LC_CTYPE, "");
    initscr();
    noecho();
    timeout(32);

    /* map puyo color type to ncurses colors */
    start_color();
    init_pair(RED, COLOR_RED, COLOR_BLACK);
    init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(PURPLE, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
}

int main(void) {
    /* init curses screen and stuff */
    curses_init();

    /* random number generator for the colors */
    srand((unsigned) time(NULL));

    /* main game instance */
    struct game g;

    /* initialize game */
    init_game(&g);
    new_puyos(&g);

    char c; // input character
    /* main game loop */
    while(true) {
    
        /* handle inputs */
        c = getch();
        if (c == 'q') {
            break;
        } else if (c == 'u') {
            rotate_puyos(&g, true);
        } else if (c == 'a') {
            update_board(&g, get_offset(LEFT));
        } else if (c == 'd') {
            update_board(&g, get_offset(RIGHT));
        } else if (c == 's') {
            update_board(&g, get_offset(DOWN));
        }

        /* no matter if there was an input or not, update the board for this frame */ 
        erase();
        print_drawable(&g);
        print_puyos(&g);

        /* increment the framecount on each "frame" */
        g.frame += 1;

        /* for every about second move the active pieces down */
        if (g.frame % 30 == 0) {
            update_board(&g, get_offset(DOWN));
        }
        if (!any_active_puyos(&g)) new_puyos(&g);
        clear_puyos(&g);
    }

    /* quit cleanly */
    endwin();
    return 0;
}
