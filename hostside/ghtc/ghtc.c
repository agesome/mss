#include "ghtc.h"
#include "signals.h"
#include <gtkdatabox.h>

int
main (int argc, char *argv[])
{

  gtk_init (&argc, &argv);

  /* main window setup, FIXME: move to a separate function when done */
  w_main = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (w_main), "gHTC");
  gtk_widget_show (w_main);

  connect_signals ();
  gtk_main ();

}
