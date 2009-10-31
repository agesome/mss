require 'mkmf'
extension_name = 'HTLLUSB'
find_header "libusb.h", "/usr/include/libusb-1.0/"
have_library "usb-1.0", "libusb_init"
create_makefile extension_name 
