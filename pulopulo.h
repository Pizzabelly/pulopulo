#include <stdbool.h>
#include <wctype.h>
#include <wchar.h>

/* board size constants */
#define BOARD_X 6
#define BOARD_Y 12
#define MAX_PUYOS BOARD_X * BOARD_Y

/* characters to build the playboard with */
static char TOP_LEFT = '/';
static char TOP_RIGHT =  '\\';
static char BOTTOM_LEFT = '\\';
static char BOTTOM_RIGHT = '/';
static char VERTICAL = '|';
static char HORIZONTAL = '-';
const wchar_t* PUYO_CHAR = L"\x25EF"; // unicode circle

/* possible colors of each puyo */
typedef enum {
    RED,
    GREEN,
    BLUE,
    PURPLE,
    YELLOW
} color_t;

/* enum for the direction of movement and parent puyos */
typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    INVALID
} direction;

/* pair of x and y values */
typedef struct {
    int x;
    int y;
} point;

/* type for a single puyo */
typedef struct {
    point pos;
    bool active;
    color_t color;
} puyo;

/* struct for the game instance */
struct game {
    /* frame counter */
    int frame;

    /* main array of puyos containing all puyos on the board */
    int puyo_count;
    puyo puyos[MAX_PUYOS];

    /* list of indexs where puyos were matched */
    int removable_count;
    int removable[MAX_PUYOS];

    /* index of current 2 puyos */
    int anchor;
    int child;

    /* previous rotation offset and info for rotation */
    point prev_offset;
    bool x_offset_switch;
};
