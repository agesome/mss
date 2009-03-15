#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <iface.h>
#include <htc.h>
#include <time.h>
#define MAXTS 3
#define MAXHS 2

const char welc[] = "HT217 interface";
extern char *recvBuffer;
double tData[MAXTS], hData[MAXHS];
WINDOW *msgs;

void
parseRecv(void){
  sscanf(recvBuffer, "%2.1f %2.1f %2.1f %2.1f %2.1f", &tData[0], &tData[1], &tData[2], &hData[0], &hData[1]);
}


/*print str in statustext box*/
void sText(const char *str){
  int y, ycmp, x;
  getyx(msgs, y, x);
  
  if(strlen(str) + x > COLS - 2)
    wmove(msgs, ++y, 2);
  if(y >= 9){
    scroll(msgs);
    wmove(msgs, --y, 2);
  }
  
  wprintw(msgs, str);
  
  getyx(msgs, ycmp, x);
  if(y < ycmp)
    wmove(msgs, ycmp, 2);
  box(msgs, 0, 0);
  getyx(msgs, y, x);
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
  msgs = newwin(10, COLS, LINES - 10, 0);
  //keypad(msgs, TRUE);
  scrollok(msgs, TRUE);
  idlok(msgs, TRUE);
  clearok(msgs, TRUE);
  wsetscrreg(msgs, 1, 8); 
  box(msgs, 0, 0);
  wmove(msgs, 1, 2);
  wrefresh(msgs);
  
  /*welcome message*/
  sText("This is HT217 interface version " VERSION ". Press F1 for help.\n");
}


int
main(int argc, char *argv[]){
  int btn;
  clock_t execT;

  init();
  usbInit();

  while(1){
    refresh();
    btn = getch();
    if(btn == 'q')
      break;
    else if(btn == KEY_F(1))
      sText(helpText);
  }
  
  endwin();
  return 0;
}
