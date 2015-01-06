#include <stdlib.h>
#include <curses.h>
#include <time.h>

#define WIDTH 20
#define HEIGHT 20
#define FRAME_DURATION (0.25*CLOCKS_PER_SEC)

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

struct piece* create_piece(char sign, int color, int xs[], int ys[], int length){
  struct piece* p = malloc(sizeof(struct piece*));
  p->color = 1;
  p->sign = sign;

  struct pointlist* pts = NULL;

  for(int i = 0; i < length; i++){
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

      if(box->pointlist->y < HEIGHT){
        translate_piece(box, DOWN);
      }

      clk_last = clock();
      draw(stdscr, box, NULL);
    }

  }
  endwin();
  return 0;
}
