#include <stdlib.h>
#include <curses.h>
#include <time.h>

#define WIDTH 20
#define HEIGHT 20
#define FRAME_DURATION (0.25*CLOCKS_PER_SEC)

enum direction {LEFT, RIGHT, DOWN};

struct matrix {
  int length;
  int* xs;
  int* ys;
};

struct pointvector {
  struct point* pt;
  struct pointvector* next;
};

struct point {
  int x;
  int y;
};

struct piece {
  struct matrix* matrix;
  char sign;
  int color;
};

struct bottom {
  struct piece* piece;
  struct list* next;
};


void draw_piece(WINDOW* win, struct piece* piece){
  int i;
  for(i = 0; i < piece->matrix->length; i++){
    attron(COLOR_PAIR(piece->color));
    int x = piece->matrix->xs[i];
    int y = piece->matrix->ys[i];
    mvwaddch(win, y,x, piece->sign);
    refresh();
  }
}

void draw(WINDOW* win, struct piece* active, struct bottom* rest){
  wclear(win);
  draw_piece(win, active);
  wrefresh(win);
}

struct piece* create_piece(char sign, int color, int xs[], int ys[], int length){
  struct piece* p = malloc(sizeof(struct piece*));
  p->color = 1;
  p->sign = sign;
  p->matrix = malloc(sizeof(struct matrix*));
  struct matrix* matrix = p->matrix;
  matrix->length = length;
  matrix->xs = calloc(length, sizeof(int));
  matrix->ys = calloc(length, sizeof(int));

  for(int i = 0; i < length; i++){
    matrix->xs[i] = xs[i];
    matrix->ys[i] = ys[i];
  }
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

  int i;
  for(i = 0; i < piece->matrix->length; i++) {
    piece->matrix->ys[i] += y;
    piece->matrix->xs[i] += x;
  }
}

int main(int argc, char** argv){
  struct piece* box = create_piece('#', 1, (int[]) {0,1,0,1}, (int[]) {0,0,1,1}, 4);

  initscr();
  // init colors
  start_color();
  int c;
  for(c = 0; c < 8; c++) {
    init_pair(c, c, COLOR_BLACK);
  };

  cbreak();
  timeout(0);
  keypad(stdscr, TRUE);

  // game loop
  clock_t clk_last = clock();

  // inital draw
  draw(stdscr, box, NULL);

  while(true){

    int key = getch();
    switch (key) {
    case KEY_LEFT:
      translate_piece(box, LEFT);
      draw(stdscr, box, NULL);
      break;
    case KEY_RIGHT:
      translate_piece(box, RIGHT);
      draw(stdscr, box, NULL);
      break;
    case KEY_DOWN:
      translate_piece(box, DOWN);
      draw(stdscr, box, NULL);
    default:
      break;
    }

    if(clock() - clk_last >= FRAME_DURATION){

      if(box->matrix->ys[1] < HEIGHT){
        translate_piece(box, DOWN);
      }

      clk_last = clock();
      draw(stdscr, box, NULL);
    }

  }
  endwin();
  return 0;
}
