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
#define DEV_REQ_LEN 36
#define DEV_TIMEOUT 1000

char *recvBuffer;
usb_dev_handle *handle;


/* code mostly taken from PowerSwitch, http://www.obdev.at/avrusb/powerswitch.html */
static int  usbGetStringAscii(usb_dev_handle *dev, int index, int langid, char *buf, int buflen)
{
  char    buffer[256];
  int     rval, i;
  
  
  if((rval = usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, langid, buffer, sizeof(buffer), 1000)) < 0)
    return rval;
  if(buffer[1] != USB_DT_STRING)
    return 0;
  if((unsigned char)buffer[0] < rval)
    rval = (unsigned char)buffer[0];
  rval /= 2;
  /* lossy conversion to ISO Latin1 */
  for(i=1;i<rval;i++){
    if(i > buflen)  /* destination buffer overflow */
      break;
    buf[i-1] = buffer[2 * i];
    if(buffer[2 * i + 1] != 0)  /* outside of ISO Latin1 range */
      buf[i-1] = '?';
  }
  buf[i-1] = 0;
  return i-1;
}

static int usbOpenDevice(usb_dev_handle **device, int vendor, char *vendorName, int product, char *productName)
{
  struct usb_bus      *bus;
  struct usb_device   *dev;
  usb_dev_handle      *handle = NULL;
  int                 errorCode = USB_ERROR_NOTFOUND;
  static int          didUsbInit = 0;
  char tmpBuf[256];
  
  if(!didUsbInit){
    didUsbInit = 1;
    usb_init();
  }
  usb_find_busses();
  usb_find_devices();
  for(bus=usb_get_busses(); bus; bus=bus->next){
    for(dev=bus->devices; dev; dev=dev->next){
      if(dev->descriptor.idVendor == vendor && dev->descriptor.idProduct == product){
	char    string[256];
	int     len;
	handle = usb_open(dev); /* we need to open the device in order to query strings */
	if(!handle){
	  errorCode = USB_ERROR_ACCESS;
	  sprintf(tmpBuf, "Warning: cannot open USB device: %s\n", usb_strerror());
	  sText(tmpBuf);
	  continue;
	}
	if(vendorName == NULL && productName == NULL){  /* name does not matter */
	  break;
	}
	/* now check whether the names match: */
	len = usbGetStringAscii(handle, dev->descriptor.iManufacturer, 0x0409, string, sizeof(string));
	if(len < 0){
	  errorCode = USB_ERROR_IO;
	  sprintf(tmpBuf, "Warning: cannot query manufacturer for device: %s\n", usb_strerror());
	  sText(tmpBuf);
	}else{
	  errorCode = USB_ERROR_NOTFOUND;
	  /* fprintf(stderr, "seen device from vendor ->%s<-\n", string); */
	  if(strcmp(string, vendorName) == 0){
	    len = usbGetStringAscii(handle, dev->descriptor.iProduct, 0x0409, string, sizeof(string));
	    if(len < 0){
	      errorCode = USB_ERROR_IO;
	      sprintf(tmpBuf, "Warning: cannot query product for device: %s\n", usb_strerror());
	      sText(tmpBuf);
	    }else{
	      errorCode = USB_ERROR_NOTFOUND;
	      /* fprintf(stderr, "seen product ->%s<-\n", string); */
	      if(strcmp(string, productName) == 0)
		break;
	    }
	  }
	}
	usb_close(handle);
	handle = NULL;
      }
    }
    if(handle)
      break;
  }
  if(handle != NULL){
    errorCode = 0;
    *device = handle;
  }
  return errorCode;
}
/* end of PowerSwitch code */

int
usbInit(void){
  
  if(usbOpenDevice(&handle, USBDEV_SHARED_VENDOR, VNAME, USBDEV_SHARED_PRODUCT, PNAME)){
    sText("Warning: Cannot open device\n");
    return 1;
  }
  
  /*usb_set_configuration(handle, 1);
    usb_claim_interface(handle, 0);*/
  return 0;
}

char
*usbData(void){
  recvBuffer = malloc(DEV_REQ_LEN);
  
  if( usb_control_msg(
		      handle,
		      USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE, // bRequestType
		      2, // bRequest
		      0, // wValue
		      0, // wIndex
		      recvBuffer, // pointer to destination buffer
		      DEV_REQ_LEN, // wLength
		      DEV_TIMEOUT
		      ) != DEV_REQ_LEN)
    sText("Warning, recieved more/less data than expected\n");
  return recvBuffer;
}

	

	
