/* Copyright (C) 2009 Evgeny Grablyk <evgeny.grablyk@gmail.com>

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

#include "ghtc.h"
#include "signals.h"

#define WM_TITLE "gHTC"

void
w_main_setup(void)
{
  /* main window setup, FIXME: move to a separate function when done */
  w_main = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (w_main), WM_TITLE);
  vbox_main = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (w_main), vbox_main);

  /* boxes for everything */
  hbox_menu = gtk_hbox_new (TRUE, 5);
  hbox_data = gtk_hbox_new (TRUE, 5);
  hbox_graphs = gtk_hbox_new (TRUE, 5);
  hbox_status = gtk_hbox_new (TRUE, 5);

  /* packing */
  gtk_box_pack_start (GTK_BOX (vbox_main), hbox_menu, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_main), hbox_data, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_main), hbox_graphs, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_main), hbox_status, FALSE, FALSE, 0);
}

void
mainmenu_setup(void)
{
  menubar = gtk_menu_bar_new ();
  gtk_container_add (GTK_CONTAINER (hbox_menu), GTK_WIDGET (menubar));
  
  mainmenu = gtk_menu_item_new_with_label ("Main");
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), mainmenu);

  mainmenu_contents = gtk_menu_new ();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(mainmenu), mainmenu_contents);
  
  mainmenu_quit = gtk_menu_item_new_with_label ("Quit");
  gtk_menu_shell_append (GTK_MENU_SHELL (mainmenu_contents), mainmenu_quit);
}

int
main (int argc, char *argv[])
{

  gtk_init (&argc, &argv);

  w_main_setup ();
  mainmenu_setup ();
  
  gtk_widget_show_all (w_main);
  connect_signals ();
  gtk_main ();

}
