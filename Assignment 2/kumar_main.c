
#include <linux/string.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/genhd.h> 
#include <linux/blkdev.h> 
#include <linux/hdreg.h> 
#include <linux/errno.h>
#include <linux/vmalloc.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>


#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

#define SECTOR_SIZE 512
#define MBR_SIZE SECTOR_SIZE
#define MBR_DISK_SIGNATURE_OFFSET 440
#define MBR_DISK_SIGNATURE_SIZE 4
#define PARTITION_TABLE_OFFSET 446
#define PARTITION_ENTRY_SIZE 16 
#define PARTITION_TABLE_SIZE 64 
#define MBR_SIGNATURE_OFFSET 510
#define MBR_SIGNATURE_SIZE 2
#define MBR_SIGNATURE 0xAA55
#define BR_SIZE SECTOR_SIZE
#define BR_SIGNATURE_OFFSET 510
#define BR_SIGNATURE_SIZE 2
#define BR_SIGNATURE 0xAA55

#define DOF_FIRST_MINOR 0
#define DOF_MINOR_CNT 16

#define DOF_SECTOR_SIZE 512
#define DOF_DEVICE_SIZE 1024 



static u_int dof_major = 0;  




typedef struct
{
	unsigned char boot_type; 
	unsigned char start_head;
	unsigned char start_sec:6;
	unsigned char start_cyl_hi:2;
	unsigned char start_cyl;
	unsigned char part_type;
	unsigned char end_head;
	unsigned char end_sec:6;
	unsigned char end_cyl_hi:2;
	unsigned char end_cyl;
	unsigned int abs_start_sec;
	unsigned int sec_in_part;
} PartEntry;

typedef PartEntry PartTable[4]; 

static PartTable def_part_table =
{
	{
		boot_type: 0x00,
		start_head: 0x00,
		start_sec: 0x2,
		start_cyl: 0x00,
		part_type: 0x83,
		end_head: 0x00,
		end_sec: 0x20,
		end_cyl: 0x09,
		abs_start_sec: 0x00000001,
		sec_in_part: 0x0000013F
	},
	{
		boot_type: 0x00,
		start_head: 0x00,
		start_sec: 0x1,
		start_cyl: 0x14,
		part_type: 0x83,
		end_head: 0x00,
		end_sec: 0x20,
		end_cyl: 0x1F,
		abs_start_sec: 0x00000280,
		sec_in_part: 0x00000180
	},
         
};

static void copy_mbr(u8 *disk)
{
struct file *dof_file;
loff_t pos1=510;
loff_t pos2=446;
char data='A';

	memset(disk, 0x0, MBR_SIZE);
	*(unsigned long *)(disk + MBR_DISK_SIGNATURE_OFFSET) = 0x36E5756D;
	memcpy(disk + PARTITION_TABLE_OFFSET, &def_part_table, PARTITION_TABLE_SIZE);
	*(unsigned short *)(disk + MBR_SIGNATURE_OFFSET) = MBR_SIGNATURE;


	dof_file = filp_open("/etc/DiskOnFile.txt",  O_RDWR|O_CREAT, 0666);

       kernel_write(dof_file,&data,MBR_SIGNATURE_SIZE, &pos1);
       printk(KERN_ALERT "signature done");
       kernel_write(dof_file,&def_part_table,PARTITION_TABLE_SIZE, &pos2);
printk(KERN_ALERT "part table done");
   
	filp_close(dof_file,NULL); 
        
}


int dofdevice_init(void)
{
	dev_data = vmalloc(DOF_DEVICE_SIZE * DOF_SECTOR_SIZE);
	if (dev_data == NULL)
		return -ENOMEM;
	
	copy_mbr(dev_data);
	return DOF_DEVICE_SIZE;
}



void dofdevice_cleanup(void)
{
	vfree(dev_data);
}

void dofdevice_write(sector_t sector_off, u8 *buffer, unsigned int sectors)
{
struct file *dof_file;
loff_t pos;
		pos=(loff_t)sector_off*DOF_SECTOR_SIZE;
	memcpy(dev_data + sector_off * DOF_SECTOR_SIZE, buffer,
		sectors * DOF_SECTOR_SIZE);
		
		

        

	

	dof_file = filp_open("/etc/DiskOnFile.txt",  O_WRONLY, 0666);

 	kernel_write(dof_file,buffer,sectors * DOF_SECTOR_SIZE,&pos);

	
	filp_close(dof_file,NULL); 
}



void dofdevice_read(sector_t sector_off, u8 *buffer, unsigned int sectors)
{
	struct file *dof_file;
loff_t pos;
		pos=(loff_t)sector_off*DOF_SECTOR_SIZE;
	memcpy(buffer, dev_data + sector_off * DOF_SECTOR_SIZE,
		sectors * DOF_SECTOR_SIZE);
		

	
        dof_file = filp_open("/etc/DiskOnFile.txt",  O_RDONLY, 0666);

	kernel_read(dof_file,buffer, sector_off*DOF_SECTOR_SIZE, &pos);

 	
	filp_close(dof_file,NULL);
        
}


static struct dof_device
{
	
	unsigned int size;
	
	spinlock_t lock;

	struct request_queue *dof_queue;
	
	struct gendisk *dof_disk;
} dof_dev;



static int dof_open(struct block_device *bdev, fmode_t mode)
{
	unsigned unit = iminor(bdev->bd_inode);

	printk(KERN_INFO "dof: Device is opened\n");
	printk(KERN_INFO "dof: Inode number is %d\n", unit);

	if (unit > DOF_MINOR_CNT)
		return -ENODEV;
	return 0;
}



#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
static int dof_close(struct gendisk *disk, fmode_t mode)
{
	printk(KERN_INFO "dof: Device is closed\n");
	return 0;
}
#else
static void dof_close(struct gendisk *disk, fmode_t mode)
{
	printk(KERN_INFO "dof: Device is closed\n");
}
#endif



static int dof_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	geo->heads = 1;
	geo->cylinders = 32;
	geo->sectors = 32;
	geo->start = 0;
	return 0;
}


static int dof_transfer(struct request *req)
{

	int dir = rq_data_dir(req);
	sector_t start_sector = blk_rq_pos(req);
	unsigned int sector_cnt = blk_rq_sectors(req);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0))
#define BV_PAGE(bv) ((bv)->bv_page)
#define BV_OFFSET(bv) ((bv)->bv_offset)
#define BV_LEN(bv) ((bv)->bv_len)
	struct bio_vec *bv;
#else
#define BV_PAGE(bv) ((bv).bv_page)
#define BV_OFFSET(bv) ((bv).bv_offset)
#define BV_LEN(bv) ((bv).bv_len)
	struct bio_vec bv;
#endif
	struct req_iterator iter;

	sector_t sector_offset;
	unsigned int sectors;
	u8 *buffer;

	int ret = 0;

	sector_offset = 0;
	rq_for_each_segment(bv, req, iter)
	{
		buffer = page_address(BV_PAGE(bv)) + BV_OFFSET(bv);
		if (BV_LEN(bv) % DOF_SECTOR_SIZE != 0)
		{
			printk(KERN_ERR "dof: Should never happen: "
				"bio size (%d) is not a multiple of DOF_SECTOR_SIZE (%d).\n"
				"This may lead to data truncation.\n",
				BV_LEN(bv), DOF_SECTOR_SIZE);
			ret = -EIO;
		}
		sectors = BV_LEN(bv) / DOF_SECTOR_SIZE;
		printk(KERN_DEBUG "dof: Start Sector: %llu, Sector Offset: %llu; Buffer: %p; Length: %u sectors\n",
			(unsigned long long)(start_sector), (unsigned long long)(sector_offset), buffer, sectors);
		if (dir == WRITE) 
		{
			dofdevice_write(start_sector + sector_offset, buffer, sectors);
		}
		else 
		{
			dofdevice_read(start_sector + sector_offset, buffer, sectors);
		}
		sector_offset += sectors;
	}
	if (sector_offset != sector_cnt)
	{
		printk(KERN_ERR "dof: bio info doesn't match with the request info");
		ret = -EIO;
	}

	return ret;
}

static void dof_request(struct request_queue *q)
{
	struct request *req;
	int ret;

	while ((req = blk_fetch_request(q)) != NULL)
	{
		ret = dof_transfer(req);
		__blk_end_request_all(req, ret);
	}
}

static struct block_device_operations dof_fops =
{
	.owner = THIS_MODULE,
	.open = dof_open,
	.release = dof_close,
	.getgeo = dof_getgeo,
};
	
static int __init dof_init(void)
{

	int ret;

	if ((ret = dofdevice_init()) < 0)
	{
		return ret;
	}
	dof_dev.size = ret;


	dof_major = register_blkdev(dof_major, "dof");
	if (dof_major <= 0)
	{
		printk(KERN_ERR "dof: Unable to get Major Number\n");
		dofdevice_cleanup();
		return -EBUSY;
	}

	spin_lock_init(&dof_dev.lock);
	dof_dev.dof_queue = blk_init_queue(dof_request, &dof_dev.lock);
	if (dof_dev.dof_queue == NULL)
	{
		printk(KERN_ERR "dof: blk_init_queue failure\n");
		unregister_blkdev(dof_major, "dof");
		dofdevice_cleanup();
		return -ENOMEM;
	}
	
	dof_dev.dof_disk = alloc_disk(DOF_MINOR_CNT);
	if (!dof_dev.dof_disk)
	{
		printk(KERN_ERR "dof: alloc_disk failure\n");
		blk_cleanup_queue(dof_dev.dof_queue);
		unregister_blkdev(dof_major, "dof");
		dofdevice_cleanup();
		return -ENOMEM;
	}

 	
	dof_dev.dof_disk->major = dof_major;
  	
	dof_dev.dof_disk->first_minor = DOF_FIRST_MINOR;
 	
	dof_dev.dof_disk->fops = &dof_fops;

	dof_dev.dof_disk->private_data = &dof_dev;
	dof_dev.dof_disk->queue = dof_dev.dof_queue;
	
	sprintf(dof_dev.dof_disk->disk_name, "dof");

	set_capacity(dof_dev.dof_disk, dof_dev.size);
	add_disk(dof_dev.dof_disk);
	printk(KERN_INFO "dof: DOF Block driver initialised (%d sectors; %d bytes)\n",
		dof_dev.size, dof_dev.size * DOF_SECTOR_SIZE);

	return 0;
}


static void __exit dof_cleanup(void)
{       


	del_gendisk(dof_dev.dof_disk);
	put_disk(dof_dev.dof_disk);
	blk_cleanup_queue(dof_dev.dof_queue);
	unregister_blkdev(dof_major, "dof");
	dofdevice_cleanup();
}

module_init(dof_init);
module_exit(dof_cleanup);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kumar Gaurav <h20210159@pilani.bits-pilani.ac.in>");
MODULE_DESCRIPTION("Assigment 2 Disk on File");
MODULE_ALIAS_BLOCKDEV_MAJOR(dof_major);


