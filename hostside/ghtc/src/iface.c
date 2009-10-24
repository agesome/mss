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

#include <usb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define USBDEV_SHARED_VENDOR 0x16C0
#define USBDEV_SHARED_PRODUCT 0x05DC
#define USB_ERROR_NOTFOUND 1
#define USB_ERROR_ACCESS 2
#define USB_ERROR_IO 3
#define VNAME "evgeny217.org.ua"
#define PNAME "HT_217"
#define DEV_REQ_LEN 64
#define DEV_TIMEOUT 1000

typedef struct _ht_data
{
  double t1, t2, t3, t4, t5;
  double h1, h2;
} data;

data r;
double t[5], h[2];

usb_dev_handle *handle;
char *recvBuf;

/* code mostly taken from PowerSwitch, http://www.obdev.at/avrusb/powerswitch.html */
static int
usbGetStringAscii (usb_dev_handle * dev, int index, int langid, char *buf,
		   int buflen)
{
  char buffer[256];
  int rval, i;


  if ((rval =
       usb_control_msg (dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR,
			(USB_DT_STRING << 8) + index, langid, buffer,
			sizeof (buffer), 1000)) < 0)
    return rval;
  if (buffer[1] != USB_DT_STRING)
    return 0;
  if ((unsigned char) buffer[0] < rval)
    rval = (unsigned char) buffer[0];
  rval /= 2;
  /* lossy conversion to ISO Latin1 */
  for (i = 1; i < rval; i++)
    {
      if (i > buflen)		/* destination buffer overflow */
	break;
      buf[i - 1] = buffer[2 * i];
      if (buffer[2 * i + 1] != 0)	/* outside of ISO Latin1 range */
	buf[i - 1] = '?';
    }
  buf[i - 1] = 0;
  return i - 1;
}

static int
usbOpenDevice (usb_dev_handle ** device, int vendor, char *vendorName,
	       int product, char *productName)
{
  struct usb_bus *bus;
  struct usb_device *dev;
  usb_dev_handle *handle = NULL;
  int errorCode = USB_ERROR_NOTFOUND;
  static int didUsbInit = 0;

  if (!didUsbInit)
    {
      didUsbInit = 1;
      usb_init ();
    }
  usb_find_busses ();
  usb_find_devices ();
  for (bus = usb_get_busses (); bus; bus = bus->next)
    {
      for (dev = bus->devices; dev; dev = dev->next)
	{
	  if (dev->descriptor.idVendor == vendor
	      && dev->descriptor.idProduct == product)
	    {
	      char string[256];
	      int len;
	      handle = usb_open (dev);	/* we need to open the device in order to query strings */
	      if (!handle)
		{
		  errorCode = USB_ERROR_ACCESS;
		  fprintf (stderr, "Warning: cannot open USB device: %s\n",
			   usb_strerror ());
		  continue;
		}
	      if (vendorName == NULL && productName == NULL)
		{		/* name does not matter */
		  break;
		}
	      /* now check whether the names match: */
	      len =
		usbGetStringAscii (handle, dev->descriptor.iManufacturer,
				   0x0409, string, sizeof (string));
	      if (len < 0)
		{
		  errorCode = USB_ERROR_IO;
		  fprintf (stderr,
			   "Warning: cannot query manufacturer for device: %s\n",
			   usb_strerror ());
		}
	      else
		{
		  errorCode = USB_ERROR_NOTFOUND;
		  /* fprintf(stderr, "seen device from vendor ->%s<-\n", string); */
		  if (strcmp (string, vendorName) == 0)
		    {
		      len =
			usbGetStringAscii (handle, dev->descriptor.iProduct,
					   0x0409, string, sizeof (string));
		      if (len < 0)
			{
			  errorCode = USB_ERROR_IO;
			  fprintf (stderr,
				   "Warning: cannot query product for device: %s\n",
				   usb_strerror ());
			}
		      else
			{
			  errorCode = USB_ERROR_NOTFOUND;
			  /* fprintf(stderr, "seen product ->%s<-\n", string); */
			  if (strcmp (string, productName) == 0)
			    break;
			}
		    }
		}
	      usb_close (handle);
	      handle = NULL;
	    }
	}
      if (handle)
	break;
    }
  if (handle != NULL)
    {
      errorCode = 0;
      *device = handle;
    }
  return errorCode;
}

/* end of PowerSwitch code */

int
usbInit (void)
{
  /* if (usbOpenDevice */
  /*     (&handle, USBDEV_SHARED_VENDOR, VNAME, USBDEV_SHARED_PRODUCT, PNAME)) */
  /*   return 1; */
  /* return 0; */
  int i;
  for(i = 0; i <= 4; i++)
    t[i] = 0;
  for(i = 0; i <= 1; i++)
    h[i] = 0;
  return 1;
}

data *
usbData (void)
{
  
  /* recvBuf = (char *) malloc (DEV_REQ_LEN); */
  /* /\* transfer problems sometimes occur, we check for them so we do not process garbage; return NULL on error *\/ */
  /* if (usb_control_msg (handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,	// bRequestType */
  /* 		       0,	// bRequest */
  /* 		       0,	// wValue */
  /* 		       0,	// wIndex */
  /* 		       recvBuf,	// pointer to destination buffer */
  /* 		       DEV_REQ_LEN,	// wLength */
  /* 		       DEV_TIMEOUT) < 0) */
  /*   return NULL; */
  /* return recvBuf; */
  r.h1++;
  r.h2 = r.h1+2;
  r.t1++;
  r.t2 = r.t1++;
  r.t3 = r.t2++;
  r.t4 = r.t3++;
  r.t5 = r.t4++;
  return &r;
}
