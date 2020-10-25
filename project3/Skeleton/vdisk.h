/**
   We use a file as a virtual disk.

   The file is partitioned into fixed-sized blocks

   This API provides 4 functions:
1. Open virtual disk
2. Close virtual disk
3. Write a block of data to the disk
4. Read a block of data from the disk
*/

#ifndef VDISK_H

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

typedef unsigned short BLOCK_REFERENCE;

// Size of a single block in bytes
#define BLOCK_SIZE 256 

// Maximum number of blocks on a disk
#define MAX_BLOCKS 1948


int vdisk_open(char *virtual_disk_name, int truncate_flag);
int vdisk_close();
int vdisk_read_block(BLOCK_REFERENCE block_ref, void *block);
int vdisk_write_block(BLOCK_REFERENCE block_ref, void *block);

#endif
