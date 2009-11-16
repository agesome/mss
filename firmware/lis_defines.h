#ifndef LIS_SDO_HIGH
#define LIS_WRITE_ADDR 0x38
#define LIS_READ_ADDR 0x39
#else
#define LIS_WRITE_ADDR 0x3a
#define LIS_READ_ADDR 0x3b
#endif

#define LIS_WHOAMI 0xf
#define LIS_WHOAMI_VALUE 0x73

#define LIS_CR1 0x20
#define LIS_DR 7
#define LIS_PD 6
#define LIS_FS 5
#define LIS_ZEN 2
#define LIS_YEN 1
#define LIS_XEN 0

#define LIS_CR2 0x21
#define LIS_REBOOT 6
#define LIS_FDS 4

#define LIS_CR3 0x22
#define LIS_IHL 7
#define LIS_PP_OD 6
#define LIS_I2CFG2 5
#define LIS_I2CFG1 4
#define LIS_I2CFG0 3
#define LIS_I1CFG2 2
#define LIS_I1CFG1 1
#define LIS_I1CFG0 0

#define LIS_SR 0x27
#define LIS_ZYXDA 3

#define LIS_OX 0x29
#define LIS_OY 0x2b
#define LIS_OZ 0x2d
