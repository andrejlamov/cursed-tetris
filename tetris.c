#include <stdlib.h>
#include <curses.h>
#include <time.h>

#define WIDTH 20
#define HEIGHT 20
#define CLOCKS_PER_FRAME (0.25*CLOCKS_PER_SEC)

#define NR_OF_NCURSES_COLORS 7

#define NR_OF_TERMINOS 3
char termino_sign[] = {'T', 'I', 'O'};

enum direction {LEFT, RIGHT, DOWN};

struct pointlist {
  int x;
  int y;
  struct pointlist* next;
};

struct piece {
  struct pointlist* pointlist;
  char pixel;
  int color;
};

struct bottomlist {
  struct piece* piece;
  struct bottomlist* next;
};

// Origin must be the first element in xs and ys.
struct piece* create_piece(char pixel, int color, int xs[], int ys[], int length){
  struct piece* p = malloc(sizeof(struct piece*));
  p->color = color;
  p->pixel = pixel;

  struct pointlist* pts = NULL;

  // Loop backwards to set origin at the head position.
  for(int i = length-1; i > -1; i--){
    struct pointlist* head = malloc(sizeof(struct pointlist*));
    head->x = xs[i];
    head->y = ys[i];
    head->next = pts;
    pts = head;
  }
  p->pointlist = pts;
  return p;
}

void translate_piece(struct piece* piece, enum  direction dir){
  int x = 0;
  int y = 0;
  switch (dir) {
  case LEFT:
    x = -1;
    break;
  case RIGHT:
    x = 1;
    break;
  case DOWN:
    y = 1;
    break;
  default:
    break;
  }

  struct pointlist* pointlist = piece->pointlist;
  while(pointlist != NULL){
    pointlist->x += x;
    pointlist->y += y;
    pointlist = pointlist->next;
  }
}

void rotate_piece(struct piece* piece){
  struct pointlist* pointlist = piece->pointlist;

  // Get origin to translate back to correct position after rotation.
  int ox = pointlist->x;
  int oy = pointlist->y;

  while(pointlist != NULL){
    int x = pointlist->x;
    int y = pointlist->y;
    pointlist->x = (y-oy) + ox;
    pointlist->y = -(x-ox) + oy;
    pointlist = pointlist->next;
  }
}

struct piece* create_termino(char sign, int color) {
  int* xs;
  int* ys;
  int length = 4;

  switch (sign) {
  case 'T':
    xs = (int[]) {1,0,2,1};
    ys = (int[]) {0,0,0,1};
    break;
  case 'I':
    xs = (int[]) {0,0,0,0};
    ys = (int[]) {0,1,2,3};
    break;
  case 'O':
    xs = (int[]) {0,1,0,1};
    ys = (int[]) {0,0,1,1};
    break;
  default:
    return NULL;
    break;
  }
  return create_piece('#', color, xs, ys, length);
}

struct piece* create_random_termino(){
  char sign = termino_sign[rand() % NR_OF_TERMINOS];
  int color = rand() % NR_OF_NCURSES_COLORS;
  return create_termino(sign, color);
}


int pieces_collide(struct piece* higher_piece, struct piece* lower_piece){
  if(higher_piece == NULL || lower_piece == NULL) {
    return 0;
  }

  struct pointlist* higher_pointlist = higher_piece->pointlist;

  // For each point in the higher piece, test if it collides with some point in the lower piece.
  while(higher_pointlist != NULL){
    int hx = higher_pointlist->x;
    int hy = higher_pointlist->y;
    struct pointlist* lower_pointlist = lower_piece->pointlist;
    while(lower_pointlist != NULL) {
      int lx = lower_pointlist->x;
      int ly = lower_pointlist->y;
      if(hy + 1 == ly && lx == hx){
        return 1;
      }
      lower_pointlist = lower_pointlist->next;
    }
    higher_pointlist = higher_pointlist->next;
  }

  return 0;
}

int collision_occured(struct piece* piece, struct bottomlist* bottom){

  // Collision with some piece
  if(bottom != NULL){
    struct bottomlist* bot = bottom;
    while(bot != NULL){
      if(pieces_collide(piece, bot->piece)){
        return 1;
      }
      bot = bot->next;
    }
  }

  // Or did the ground stop us...
  struct pointlist* pointlist = piece->pointlist;
  while(pointlist != NULL){
    int y = pointlist->y;
    if(y + 1 == HEIGHT){
      return 1;
    }
    pointlist = pointlist->next;
  }
  return 0;
}

void draw_piece(WINDOW* win, struct piece* piece){
  struct pointlist* pointlist = piece->pointlist;
  while(pointlist != NULL){
    attron(COLOR_PAIR(piece->color));
    mvwaddch(win, pointlist->y, pointlist->x, piece->pixel);
    pointlist = pointlist->next;
  }
}

void draw(WINDOW* win, struct piece* active, struct bottomlist* bottom){
  wclear(win);
  draw_piece(win, active);
  refresh();
  if(bottom != NULL){
    struct bottomlist* bot = bottom;
    while(bot != NULL){
      if(bot->piece != NULL){
        draw_piece(win, bot->piece);
        refresh();
      }
      bot = bot->next;
    }
  }
  wrefresh(win);
}

int main(int argc, char** argv){
  srand(time(NULL));

  initscr();
  // Init colors
  start_color();
  for(int c = 0; c < 8; c++) {
    init_pair(c, c, COLOR_BLACK);
  };

  cbreak();
  timeout(0);
  keypad(stdscr, TRUE);

  struct bottomlist* bottom = NULL;malloc(sizeof(struct bottomlist*));

  // Game loop
  clock_t clk_last = clock();

  struct piece* current_piece = create_random_termino();
  // Inital draw
  draw(stdscr, current_piece, bottom);

  while(true){
    if(collision_occured(current_piece, bottom)){
      struct bottomlist* bottom0 = malloc(sizeof(struct bottomlist*));
      bottom0->piece = current_piece;
      bottom0->next = bottom;
      bottom = bottom0;
      current_piece = create_random_termino();
    }

    int key = getch();
    switch (key) {
    case KEY_LEFT:
      translate_piece(current_piece, LEFT);
      draw(stdscr, current_piece, bottom);
      break;
    case KEY_RIGHT:
      translate_piece(current_piece, RIGHT);
      draw(stdscr, current_piece, bottom);
      break;
    case KEY_DOWN:
      translate_piece(current_piece, DOWN);
      draw(stdscr, current_piece, bottom);
      break;
    case ' ':
      rotate_piece(current_piece);
      draw(stdscr, current_piece, bottom);
      break;
    default:
      break;
    }
    if(clock() - clk_last >= CLOCKS_PER_FRAME){
      translate_piece(current_piece, DOWN);
      clk_last = clock();
      draw(stdscr, current_piece, bottom);
    }

  }
  endwin();
  return 0;
}
