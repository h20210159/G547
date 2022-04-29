####################################################################    ASSIGNMENT 2 DISK ON FILE BY KUMAR GAURAV (2021H1400159P)#########################################################

To test the working of the Disk On File(DOF) Block Device Driver. Follow the steps given below:

1) $make 
This will create the object file main.ko

2) $sudo insmod main.ko 
This will add the kernel object file cretaed in step 1 to the kernel module

3) $lsmod
Check is the block_driver module is present in the kernel

4) $ls /dev
Check is the disk file dof and its partitions dof1 and dof2 are created succesfully

5) $sudo chmod 777 /dev/dof  
   $sudo chmod 777 /dev/dof1  
   $sudo chmod 777 /dev/dof2

Change the permission of the disk file and its partitions 

6) $lsblk 

Displays the disk file and its partitions, their size and type. Here the size of the dof file can be checked whether it is 512KB or not 

7) $dmesg
view the kernel log and see the comments 
ls
8) $ fdisk -l /dev/dof
Additionally this cmmand can be used to see the partitions and ther starting and ending sectors, number of blocks and ID. 

9) cat > /dev/dof1 
After this command type " THIS IS GAURAV" and then press ctrl+C. This will write this sentence on dof1

10) cat > /dev/dof2 
After this command type " THIS IS GAURAV'S FRIEND" and then press ctrl+C. This will write this sentence on dof2

11) sudo dd if=/dev/dof of=diskdetails.txt
This will print the disk details including the MBR on the newly created file diskdetails.txt. We can also check if the 
data we wrote in dof1 and dof2 appears here. 

12) sudo dd if=/dev/dof1 of=partition1.txt
This will print the data on dof1 including the sentence we wrote in step (8) on the newly created file partition1.txt

13) sudo dd if=/dev/dof2 of=partition2.txt
This will print the data on dof2 including the sentence we wrote in step (9) on the newly created file partition9.txt






