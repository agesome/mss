#include "ghtc.h"
#include "callbacks.h"

void
connect_signals (void)
{
  g_signal_connect (G_OBJECT (w_main), "delete_event",
		    G_CALLBACK (wmain_delete_event), NULL);
  g_signal_connect (G_OBJECT (w_main), "destroy",
		    G_CALLBACK (w_main_destroy), NULL);

  g_signal_connect_swapped (G_OBJECT (mainmenu_quit), "activate",
			    G_CALLBACK (ghtc_quit), 
			    NULL);
  
}
