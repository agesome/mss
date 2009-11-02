#include <stdio.h>
#include <stdlib.h>
#include <libusb.h>
#include <ruby.h>

#define USB_VID 0x16C0
#define USB_PID 0x05DC

VALUE HTLLUSB = Qnil;

libusb_context *ctx;
libusb_device_handle *dh;
char data_buffer[256];

void Init_HTLLUSB (void);
VALUE htllusb_initialize (VALUE, VALUE);
VALUE htllusb_fetch_data (void);
VALUE htllusb_deinit (void);

char *
usb_decode_error (int code)
{
  switch (code)
    {
    case LIBUSB_ERROR_IO:
      return "Input/output error.";
    case LIBUSB_ERROR_INVALID_PARAM:
      return "Invalid parameter";
    case LIBUSB_ERROR_ACCESS:
      return "Access denied.";
    case LIBUSB_ERROR_NO_DEVICE:
      return "No such device.";
    case LIBUSB_ERROR_NOT_FOUND:
      return "Entity not found.";
    case LIBUSB_ERROR_BUSY:
      return "Resource busy.";
    case LIBUSB_ERROR_TIMEOUT:
      return "Operation timed out.";
    case LIBUSB_ERROR_OVERFLOW:
      return "Overflow.";
    case LIBUSB_ERROR_PIPE:
      return "Pipe error.";
    case LIBUSB_ERROR_INTERRUPTED:
      return "System call interrupted";
    case LIBUSB_ERROR_NO_MEM:
      return "Insufficient memory.";
    case LIBUSB_ERROR_NOT_SUPPORTED:
      return "Operation not supported.";
    case LIBUSB_ERROR_OTHER:
      return "Unknown error";
    default:
      return NULL;
    }
}

int
usb_init (int dlevel)
{
  int rc;

  rc = libusb_init (&ctx);
  if (rc)
    {
      fprintf (stderr, usb_decode_error (rc));
      return 1;
    }

  if (dlevel >= 0 && dlevel <= 3)
    libusb_set_debug (ctx, dlevel);
  else
    fprintf (stderr,
	     "Warning: Tried to set abnormal debug level. Ingoring.\n");

  return 0;
}

int
usb_open_dev (void)
{
  dh = libusb_open_device_with_vid_pid (ctx, USB_VID, USB_PID);

  if (dh == NULL)
    return 1;
  return 0;
}

VALUE
htllusb_deinit (void)
{
  libusb_close (dh);
  libusb_exit (ctx);
  return Qnil;
}

VALUE
htllusb_fetch_data (void)
{
  int rc;

  rc = libusb_control_transfer (dh,
				LIBUSB_REQUEST_TYPE_VENDOR |
				LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN,
				0, 0, 0, (unsigned char *) data_buffer, 256,
				1000);
  if (usb_decode_error (rc) == NULL)
    return rb_str_new2 (data_buffer);
  if (rc == LIBUSB_ERROR_NO_DEVICE)
    rb_raise (rb_eEOFError, usb_decode_error (rc));
  rb_raise (rb_eIOError, usb_decode_error (rc));
}

VALUE
htllusb_initialize (VALUE self, VALUE dl)
{
  int rc;

  rc = usb_init (NUM2INT (dl));
  if (rc)
    rb_raise (rb_eException, "Failed to initialize LibUSB");
  rc = usb_open_dev ();
  if (rc)
    rb_raise (rb_eException, "Failed to open device");
  return self;
}

void
Init_HTLLUSB (void)
{
  HTLLUSB = rb_define_class ("HTLLUSB", rb_cObject);
  rb_define_method (HTLLUSB, "initialize", htllusb_initialize, 1);
  rb_define_method (HTLLUSB, "fetch_data", htllusb_fetch_data, 0);
  rb_define_method (HTLLUSB, "close", htllusb_deinit, 0);

}
