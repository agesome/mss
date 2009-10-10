#include "ghtc.h"
#include "callbacks.h"

void
connect_signals (void)
{
  g_signal_connect (G_OBJECT (w_main), "delete_event",
		    G_CALLBACK (wmain_delete_event), NULL);
  g_signal_connect (G_OBJECT (w_main), "destroy",
		    G_CALLBACK (a_destroy), NULL);

}
