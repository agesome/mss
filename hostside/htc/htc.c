/*  Copyright 2009 Evgeny Grablyk <evgeny.grablyk@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include "iface.h"
#include <time.h>
#include <math.h>
#define MAXTS 3
#define MAXHS 2

char logfile[] = "Logfile.txt";
const char welc[] = "HT217 interface";

extern char *recvBuf;
float tData[MAXTS], hData[MAXHS];
WINDOW *msgs, *data;
FILE *lg;

void
parseRecv (void)
{
  sscanf (recvBuf, "%f %f %f %f %f", &tData[0], &tData[1], &tData[2],
	  &hData[0], &hData[1]);
  fprintf (lg, "%s\n", recvBuf);
}

void
updateData (void)
{
  wclear (data);
  box (data, 0, 0);
  mvwprintw (data, 0, 3, "Data");
  mvwprintw (data, trunc (LINES / 6 / 2), 2,
	     "T 1: %2.1f; T 2: %2.1f; T 3: %2.1f; Fi 1: %2.1f; Fi 2: %2.1f;",
	     tData[0], tData[1], tData[2], hData[0], hData[1]);
  wrefresh (data);
}

/*print str in statustext box*/
void
sText (const char *str)
{
  int y, ycmp, x;

  /*border checks */
  getyx (msgs, y, x);
  if (strlen (str) + x >= (unsigned int) COLS - 1)
    wmove (msgs, ++y, 2);
  if (y >= trunc (LINES / 4) - 2)
    {
      scroll (msgs);

      wmove (msgs, --y, 2);
    }
  wprintw (msgs, str);
  /* if there was a newline, go to a free line */
  getyx (msgs, ycmp, x);
  if (y < ycmp)
    wmove (msgs, ycmp, 2);
  getyx (msgs, y, x);
  box (msgs, 0, 0);
  mvwprintw (msgs, 0, 3, "Status");
  wmove (msgs, y, x);
  wrefresh (msgs);
}

void
init (void)
{
  /*stdscr part */
  initscr ();
  cbreak ();
  noecho ();
  keypad (stdscr, TRUE);
  mvprintw (0, COLS / 2 - strlen (welc), welc);
  refresh ();

  /*message box part */
  msgs = newwin (trunc (LINES / 4), COLS, trunc (LINES - LINES / 4), 0);
  scrollok (msgs, TRUE);
  idlok (msgs, TRUE);
  clearok (msgs, TRUE);
  wsetscrreg (msgs, 1, trunc (LINES / 4) - 2);
  box (msgs, 0, 0);
  wmove (msgs, 1, 2);
  wrefresh (msgs);

  /* data box */
  data = newwin (trunc (LINES / 5), COLS, trunc (LINES / 5), 0);
  box (data, 0, 0);
  wrefresh (data);

  /*welcome message */
  sText ("This is HT217 interface version " VERSION ".\n");

  /*   log file */
  lg = fopen (logfile, "a");
}


int
main (void)
{
  clock_t execT, delta;

  execT = clock () / CLOCKS_PER_SEC;
  init ();
  if (usbInit ())
    {
      endwin ();
      fprintf (stderr, "Problems opening device, exiting.\n");
      return 1;
    }

  while (1)
    if ((delta = clock () / CLOCKS_PER_SEC) - execT >= 1)
      {
	execT = clock () / CLOCKS_PER_SEC;
	if (!(usbData () == NULL))
	  {
	    parseRecv ();
	    updateData ();
	    refresh ();
	  }
      }

  fclose (lg);
  endwin ();
  return 0;
}
