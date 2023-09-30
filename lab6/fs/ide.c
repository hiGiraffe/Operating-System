/*
 * operations on IDE disk.
 */

#include "serv.h"
#include <drivers/dev_disk.h>
#include <lib.h>
#include <mmu.h>

// Overview:
//  read data from IDE disk. First issue a read request through
//  disk register and then copy data from disk buffer
//  (512 bytes, a sector) to destination array.
//
// Parameters:
//  diskno: disk number.磁盘号
//  secno: start sector number.起始扇区号
//  dst: destination for data read from IDE disk.从IDE硬盘读取数据的目的地
//  nsecs: the number of sectors to read.要读取的扇区数
//
// Post-Condition:
//  Panic if any error occurs. (you may want to use 'panic_on')
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_OPERATION_READ',1
//  'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS', 'DEV_DISK_BUFFER'
// BY2SECT - Bytes per disk sector
void ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	u_int read_tag = 0;
	u_int status = 0;
	u_int cur_offset;
	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		
		/* Exercise 5.3: Your code here. (1/2) */
		cur_offset = begin + off;
		//设置磁盘号
		if(syscall_write_dev((u_int)&diskno,0x13000010, 4) != 0){
			user_panic("write in 0x13000010 fail!\n");
		}
		//设置偏移量
		if (syscall_write_dev((u_int)&cur_offset, 0x13000000, 4) != 0) {
			user_panic("write in 0x13000000 failed!\n");
		}
		//设置读还是写
		if (syscall_write_dev((u_int)&read_tag, 0x13000020, 4) != 0) {
			user_panic("write in 0x13000020 failed!\n");
		}

		//read
		//读上一次操作的状态返回值
		if (syscall_read_dev(&status, 0x13000030, 4) != 0) {
			user_panic("read in 0x13000030 failed1!\n");
		}
		//如果没读成功
		if(status == 0){
			user_panic("read in 0x13000030 failed2!\n");
		}
		//从缓存读数据
		if (syscall_read_dev((u_int)(dst + off), 0x13004000, 0x200) != 0) {
			user_panic("read in 0x13004000 failed!\n");
		}
	}
}
// Overview:
//  write data to IDE disk.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  src: the source data to write into IDE disk.
//  nsecs: the number of sectors to write.
//
// Post-Condition:
//  Panic if any error occurs.
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_BUFFER',
//  'DEV_DISK_OPERATION_WRITE', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS'
void ide_write(u_int diskno, u_int secno, void *src, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	u_int write_tag = 1;
	u_int status = 0;
	u_int cur_offset;
	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		/* Exercise 5.3: Your code here. (2/2) */
		cur_offset = begin + off;
		//设置磁盘号
		if(syscall_write_dev((u_int)&diskno,0x13000010, 4) != 0){
			user_panic("write in 0x13000010 fail!\n");
		}
		//设置偏移量
		if (syscall_write_dev((u_int)&cur_offset, 0x13000000, 4) != 0) {
			user_panic("write in 0x13000000 failed!\n");
		}
		//写数据到缓冲
		if (syscall_write_dev((u_int)(src + off), 0x13004000, 0x200) != 0) {
			user_panic("write in 0x13004000 failed!\n");
		}
		//设置写操作
		if (syscall_write_dev((u_int)&write_tag, 0x13000020, 4) != 0) {
			user_panic("write in 0x13000020 failed!\n");
		}
		//获取上一次操作的状态返回值
		if (syscall_read_dev(&status, 0x13000030, 4) != 0) {
			user_panic("read in 0x13000030 failed1!\n");
		}
		//没写成功
		if(status == 0){
			user_panic("read in 0x13000030 failed2!\n");
		}
	}
}
