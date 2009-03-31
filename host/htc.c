#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <iface.h>
#include <time.h>
#include <math.h>
#define MAXTS 3
#define MAXHS 2

const char welc[] = "HT217 interface";
const char helpText[] = "F1: This help text. q: quit.\n";

extern char *recvBuf;
float tData[MAXTS], hData[MAXHS];
WINDOW *msgs, *data;

void
parseRecv(char *buf){
  sscanf(buf, "%f %f %f %f %f", &tData[0], &tData[1], &tData[2], &hData[0], &hData[1]);
}

void
updateData(void){
  wclear(data);
  box(data, 0, 0);
  mvwprintw(data, 0, 3, "Data");
  mvwprintw(data, trunc(LINES / 6 / 2), 2, "T 1: %2.1f; T 2: %2.1f; T 3: %2.1f; Fi 1: %2.1f; Fi 2: %2.1f;", tData[0], tData[1], tData[2], hData[0], hData[1]);
  wrefresh(data);
}

/*print str in statustext box*/
void sText(const char *str){
  int y, ycmp, x;
  
  /*border checks*/
  getyx(msgs, y, x);
  if(strlen(str) + x >= COLS - 1)
    wmove(msgs, ++y, 2);
  if(y >= trunc(LINES / 4) - 2){
    scroll(msgs);

    wmove(msgs, --y, 2);
  }
  wprintw(msgs, str);
  /* if there was a newline, go to a free line*/
  getyx(msgs, ycmp, x);
  if(y < ycmp)
    wmove(msgs, ycmp, 2);
  getyx(msgs, y, x);
  box(msgs, 0, 0);
  mvwprintw(msgs, 0, 3, "Status");
  wmove(msgs, y, x);
  wrefresh(msgs);
}

void init(void){
  /*stdscr part*/
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  mvprintw(0, COLS/2-strlen(welc), welc);
  refresh();
  
  /*message box part*/
  msgs = newwin(trunc(LINES / 4), COLS, trunc(LINES - LINES / 4), 0);
  scrollok(msgs, TRUE);
  idlok(msgs, TRUE);
  clearok(msgs, TRUE);
  wsetscrreg(msgs, 1, trunc(LINES / 4) - 2); 
  box(msgs, 0, 0);
  wmove(msgs, 1, 2);
  wrefresh(msgs);

  /* data box */
  data = newwin(trunc(LINES / 5), COLS, trunc(LINES / 5), 0);
  box(data, 0, 0);
  wrefresh(data);
  
  /*welcome message*/
  sText("This is HT217 interface version " VERSION ". Press F1 for help.\n");
}


int
main(int argc, char *argv[]){
  int btn;
  clock_t execT, delta;
  char tmp[256];
  
  execT = clock() / CLOCKS_PER_SEC;
  init();
  if(usbInit()){
    endwin();
    fprintf(stderr, "Problems opening device, exiting.\n");
    return 1;
  }
  
  while(1){
    if((delta = clock() / CLOCKS_PER_SEC) - execT >= 1){
      execT = clock() / CLOCKS_PER_SEC;
      parseRecv(usbData());
      updateData();
      refresh();
    }
/*     btn = getch(); */
/*     if(btn == 'q') */
/*       break; */
/*     else if(btn == KEY_F(1)) */
/*       sText(helpText); */
  }
  
  endwin();
  return 0;
}
