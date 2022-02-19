#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>              
#include <unistd.h>             
#include <stdint.h>
#include <sys/ioctl.h>          
#include "adc8dev.h"


uint16_t data;


int ioctl_sel_channel(int file_desc, int channel)                   // ioctl call               
{
    int m, SEL_CONV;
    m = ioctl(file_desc, CHANNELSELECTION, channel);
    if (m < 0) 
	{
        printf("ioctl_sel_channel failed:%d\n", m);
        exit(-1);
	}
    return 0;
}

int ioctl_sel_alignment(int file_desc, char align)                           // ioctl call
{
    int m;
    m = ioctl(file_desc, ALSELECTION, align);
    if (m < 0) {
        printf("ioctl_sel_alignment failed:%d\n", m);
        exit(-1);
    }
    return 0;
}

int ioctl_sel_conv(int file_desc, int conv)                     // ioctl call
{
	int ret_val, SEL_CONV;
	ret_val = ioctl(file_desc, SEL_CONV, conv);
	if (ret_val < 0)
{
	printf("IOCTL alignment failed:%d\n", ret_val);
	exit(-1);
	}
	return 0;
}	
 
int main()
{
    int file_desc;
    int channel;
    char align;
    int conv;
    file_desc = open(DEVICE_FILE_NAME, 0);
	if (file_desc < 0) 
	{
        	printf("Can't open device file: %s\n", DEVICE_FILE_NAME);
        	exit(-1);
	}
		printf("Enter the Channel(0-7):");
		scanf("%d", &channel);
		printf("Enter the Alignment(r or l):");
		scanf(" %c", &align);
	if(channel <0 || channel>7 || (align != 'r' && align != 'l'))
	{
		printf("Invalid Selection of Channel or Alignment\n");
		exit(-1);
	}
		printf("Enter type of conversion(0-1):");
		scanf("%d", &conv);
		                                                                                  //  all functions defined above main function is called here 
		ioctl_sel_channel(file_desc,channel);
		ioctl_sel_alignment(file_desc,align);
		if(read(file_desc,&data,sizeof(data)))
	{
		if(align == 'l')                                                          // allignment and conversion modes covered from line 76 to line 94
	{
		printf("left alligned number %u \n", data);
		printf("original number %u \n", data/64);
	}
	else
	{
		printf("right allignment and original number is:%u ", data);
        	
	}
	if(conv==0)
	{ printf("one shot execution");
	}
	else
	{ for(int i=0;i<50;i++)
	  { printf("\nData read user is : %u -", data);
	  }
	 } 
	}
		close(file_desc);
		return 0;
}
