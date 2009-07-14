#include "ghtc.h"

gboolean
wmain_destroy_event (void)
{
  return FALSE;
}

void
a_destroy (void)
{
  gtk_main_quit ();
}
