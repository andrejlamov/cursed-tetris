#include <stdlib.h>
#include <curses.h>
#include <time.h>

#define WIDTH 10
#define HEIGHT 20
#define CLOCKS_PER_FRAME (0.25*CLOCKS_PER_SEC)

#define NR_OF_NCURSES_COLORS 7

#define TERMINO {T, I, O}
enum termino TERMINO;
const enum termino terminos[] = TERMINO;

enum direction {LEFT, RIGHT, DOWN};

struct pointlist {
  int x;
  int y;
  struct pointlist* tail;
};

struct piece {
  struct pointlist* points;
  char pixel;
  int color;
};

struct piecelist {
  struct piece* p;
  struct piecelist* tail;
};

// Origin must be the first element in xs and ys.
struct piece* create_piece(char pixel, int color, int xs[], int ys[], int length){
  struct piece* new_piece = malloc(sizeof(struct piece*));
  new_piece->color = color;
  new_piece->pixel = pixel;

  struct pointlist* points = NULL;

  // Loop backwards to set origin at the head position.
  for(int i = length-1; i > -1; i--){
    struct pointlist* head = malloc(sizeof(struct pointlist*));
    head->x = xs[i];
    head->y = ys[i];
    head->tail = points;
    points = head;
  }
  new_piece->points = points;
  return new_piece;
}

void translate_piece0(struct piece* piece, int dx, int dy){
  struct pointlist* points = piece->points;
  while(points != NULL){
    points->x += dx;
    points->y += dy;
    points = points->tail;
  }
}

void translate_piece1(struct piece* piece, enum direction dir){
  switch (dir) {
  case LEFT:
    translate_piece0(piece, -1, 0);
    break;
  case RIGHT:
    translate_piece0(piece, 1, 0);
    break;
  case DOWN:
    translate_piece0(piece, 0, 1);
    break;
  default:
    break;
  }
}

void rotate_piece(struct piece* piece){
  struct pointlist* points = piece->points;

  // Get origin to translate back to correct position after rotation.
  int ox = points->x;
  int oy = points->y;

  while(points != NULL){
    int x = points->x;
    int y = points->y;
    points->x = (y-oy) + ox;
    points->y = -(x-ox) + oy;
    points = points->tail;
  }
}

struct piece* create_termino(enum termino term, int color0) {
  int* xs;
  int* ys;
  int length = 4;
  int color = 0;
  switch (term) {
  case T:
    xs = (int[]) {1,0,2,1};
    ys = (int[]) {0,0,0,1};
    color = 0;
    break;
  case I:
    xs = (int[]) {0,0,0,0};
    ys = (int[]) {0,1,2,3};
    color = 1;
    break;
  case O:
    xs = (int[]) {0,1,0,1};
    ys = (int[]) {0,0,1,1};
    color = 2;
    break;
  default:
    return NULL;
    break;
  }
  return create_piece('#', color, xs, ys, length);
}

struct piece* create_random_termino(){
  enum termino term = terminos[rand() % (sizeof(terminos) / sizeof(enum termino))];
  int color = rand() % NR_OF_NCURSES_COLORS;
  return create_termino(term, color);
}

int for_some_point0(int (*prop)(int, int), struct piece* p){
  if(p == NULL || prop == NULL) {
    return 0;
  }

  struct pointlist* points = p->points;

  while(points != NULL){
    int x = points->x;
    int y = points->y;

    if(prop(x,y)){
      return 1;
    }
    points = points->tail;
  }
  return 0;
}

int for_some_point1(int (*prop)(int, int, int, int), struct piece* p0, struct piece* p1){
  if(p0 == NULL || p1 == NULL || prop == NULL) {
    return 0;
  }

  struct pointlist* p0_points = p0->points;

  while(p0_points != NULL){
    int mx = p0_points->x;
    int my = p0_points->y;
    struct pointlist* p1_points = p1->points;
    while(p1_points != NULL) {
      int sx = p1_points->x;
      int sy = p1_points->y;
      if(prop(mx, my, sx, sy)){
        return 1;
      }
      p1_points = p1_points->tail;
    }
    p0_points = p0_points->tail;
  }

  return 0;
}

int for_some_piece(int (*prop)(struct piece*, struct piece*), struct piece* p0, struct piecelist* bottom0){
  if(bottom0 == NULL){
    return 0;
  }

  struct piecelist* bottom = bottom0;
  while(bottom != NULL){
    if(prop(p0, bottom->p)){
      return 1;
    }
    bottom = bottom->tail;
  }

  return 0;
}

int points_overlap(int x0, int y0, int x1, int y1){
  return x0 == x1 && y0 == y1;
}

int y_points_will_overlap(int x0, int y0, int x1, int y1){
  return x0 == x1 && y0 + 1 == y1;
}

int pieces_overlap(struct piece* p0, struct piece* p1){
  return for_some_point1(points_overlap, p0, p1);
}

int pieces_will_overlap(struct piece* p0, struct piece* p1){
  return for_some_point1(y_points_will_overlap, p0, p1);
}

int point_will_hit_y_border(int x, int y){
  return y + 1 == HEIGHT;
}

int point_hit_x_border(int x, int y){
  return x == WIDTH || x == -1;
}

void draw_piece(WINDOW* win, struct piece* piece){
  struct pointlist* points = piece->points;
  while(points != NULL){
    attron(COLOR_PAIR(piece->color));
    mvwaddch(win, points->y, points->x, piece->pixel);
    points = points->tail;
    attroff(COLOR_PAIR(1));
  }
}

void draw(WINDOW* win, struct piece* active, struct piecelist* bottom0){
  wclear(win);
  draw_piece(win, active);
  if(bottom0 != NULL){
    struct piecelist* bottom = bottom0;
    while(bottom != NULL){
      if(bottom->p != NULL){
        draw_piece(win, bottom->p);
      }
      bottom = bottom->tail;
    }
  }
  wrefresh(win);
}


void remove_points(int h, struct piecelist* bottom0){
    struct piecelist* bottom = bottom0;
    while(bottom != NULL){
      struct pointlist* points = bottom->p->points;
      while(points != NULL){
        if(points->y == h && points->tail == NULL){
          bottom->p->points = NULL;
          free(points);
        } else if(points->tail != NULL){
          if(points->tail->y == h){
            struct pointlist* tail = points->tail;
            points->tail = tail->tail;
            free(tail);
            continue;
          } else if(points->y == h){
            struct pointlist* temp = points->tail;
            bottom->p->points = temp;
            free(points);
            points = temp;
            continue;
          }
        }
        points = points->tail;
      }
      bottom = bottom->tail;
    }
    bottom = bottom0;
    while(bottom != NULL){
      struct pointlist* points = bottom->p->points;
      while(points != NULL){
        if(points->y < h){
          points->y += +1;
        }
        points = points->tail;
      }
      bottom = bottom->tail;
    }
}

void remove_full_rows(struct piecelist* bottom0){
  for(int h = HEIGHT; h >= 0; h--){
    int x = WIDTH;
    struct piecelist* bottom = bottom0;
    while(bottom != NULL){
      struct pointlist* points = bottom->p->points;
      while(points != NULL){
        if(points->y == h){
          x--;
          if(x == 0){
            remove_points(h, bottom0);
          }
        }
        points = points->tail;
      }
      bottom = bottom->tail;
    }
  }
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

  struct piecelist* bottom = NULL;

  // Game loop
  clock_t clk_last = clock();

  struct piece* current_piece = create_random_termino();
  translate_piece0(current_piece, WIDTH/2, 0);

  // Inital draw
  draw(stdscr, current_piece, bottom);

  while(true){
    int key = getch();
    switch (key) {
    case KEY_LEFT:
      translate_piece1(current_piece, LEFT);
      if(for_some_piece(pieces_overlap, current_piece, bottom)
         || for_some_point0(point_hit_x_border, current_piece)){
        translate_piece1(current_piece, RIGHT);
        break;
      }
      draw(stdscr, current_piece, bottom);
      break;
    case KEY_RIGHT:
      translate_piece1(current_piece, RIGHT);
      if(for_some_piece(pieces_overlap, current_piece, bottom)
         || for_some_point0(point_hit_x_border,current_piece)){
        translate_piece1(current_piece, LEFT);
        break;
      }
      draw(stdscr, current_piece, bottom);
      break;
    case KEY_DOWN:
      translate_piece1(current_piece, DOWN);
      draw(stdscr, current_piece, bottom);
      break;
    case ' ':
      rotate_piece(current_piece);
      draw(stdscr, current_piece, bottom);
      break;
    default:
      break;
    }

    if(for_some_piece(pieces_will_overlap, current_piece, bottom)
       || for_some_point0(point_will_hit_y_border,current_piece)){

      // Make it a part of the bottom
      struct piecelist* bottom0 = malloc(sizeof(struct piecelist*));
      bottom0->p = current_piece;
      bottom0->tail = bottom;
      bottom = bottom0;
      // New current piece
      current_piece = create_random_termino();
      translate_piece0(current_piece, WIDTH/2, 0);
    }

    remove_full_rows(bottom);

    if(clock() - clk_last >= CLOCKS_PER_FRAME){
      translate_piece1(current_piece, DOWN);
      clk_last = clock();
      draw(stdscr, current_piece, bottom);
    }

  }
  endwin();
  return 0;
}
