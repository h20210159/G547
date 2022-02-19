#ifndef ADC8DEV_H
#define ADC8DEV_H

#include <linux/ioctl.h>

#define MAGIC_NUM 241


#define CHANNELSELECTION _IOR(MAGIC_NUM, 1, int *)


#define ALSELECTION _IOR(MAGIC_NUM, 2, char *)

#define DEVICE_FILE_NAME "/dev/adc-dev"


#endif
