#include "ghtc.h"

gboolean
wmain_delete_event (void)
{
  return FALSE;
}

void
a_destroy (void)
{
  gtk_main_quit ();
}
