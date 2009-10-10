#include "ghtc.h"

gboolean
wmain_delete_event (void)
{
  return FALSE;
}

void
w_main_destroy (void)
{
  gtk_main_quit ();
}

void
ghtc_quit (void)
{
  gtk_main_quit ();
}
