#include <stdlib.h>
#include <curses.h>
#include <time.h>

#define WIDTH 20
#define HEIGHT 20
#define CLOCKS_PER_FRAME (0.25*CLOCKS_PER_SEC)

enum direction {LEFT, RIGHT, DOWN};

struct pointlist {
  int x;
  int y;
  struct pointlist* next;
};

struct piece {
  struct pointlist* pointlist;
  char sign;
  int color;
};

struct bottomlist {
  struct piece* piece;
  struct bottomlist* next;
};


void draw_piece(WINDOW* win, struct piece* piece){
  struct pointlist* pointlist = piece->pointlist;
  while(pointlist != NULL){
    attron(COLOR_PAIR(piece->color));
    mvwaddch(win, pointlist->y, pointlist->x, piece->sign);
    refresh();
    pointlist = pointlist->next;
  }
}

void draw(WINDOW* win, struct piece* active, struct bottomlist* rest){
  wclear(win);
  draw_piece(win, active);
  wrefresh(win);
}

// Origin must be the first element in xs and ys.
struct piece* create_piece(char sign, int color, int xs[], int ys[], int length){
  struct piece* p = malloc(sizeof(struct piece*));
  p->color = 1;
  p->sign = sign;

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

int main(int argc, char** argv){
  struct piece* box = create_piece('#', 1, (int[]) {0,1,2,3}, (int[]) {0,0,0,0}, 4);
  struct piece* the_T_piece = create_piece('#', 1, (int[]) {1,0,2,1}, (int[]) {0,0,0,1}, 4);
  struct piece* current_piece = the_T_piece;

  initscr();
  // Init colors
  start_color();
  int c;
  for(c = 0; c < 8; c++) {
    init_pair(c, c, COLOR_BLACK);
  };

  cbreak();
  timeout(0);
  keypad(stdscr, TRUE);

  // Game loop
  clock_t clk_last = clock();

  // Inital draw
  draw(stdscr, current_piece, NULL);

  while(true){

    int key = getch();
    switch (key) {
    case KEY_LEFT:
      translate_piece(current_piece, LEFT);
      draw(stdscr, current_piece, NULL);
      break;
    case KEY_RIGHT:
      translate_piece(current_piece, RIGHT);
      draw(stdscr, current_piece, NULL);
      break;
    case KEY_DOWN:
      translate_piece(current_piece, DOWN);
      draw(stdscr, current_piece, NULL);
    case ' ':
      rotate_piece(current_piece);
      draw(stdscr, current_piece, NULL);
      break;
    default:
      break;
    }

    if(clock() - clk_last >= CLOCKS_PER_FRAME){

      if(current_piece->pointlist->y < HEIGHT){
        translate_piece(current_piece, DOWN);
      }

      clk_last = clock();
      draw(stdscr, current_piece, NULL);
    }

  }
  endwin();
  return 0;
}
