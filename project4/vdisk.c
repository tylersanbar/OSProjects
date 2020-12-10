#include "vdisk.h"
/*
 * Virtual disk implementation.
 *
 * Author: Andrew H. Fagg (CS3113)
 *
 * The disk is implemented on top of a file.  Access provided by this
 * library is on a block-by-block basis
 */

 // Debug flag: set if you want sections of code to display debugging info
 //#define debug 0
extern int debug;

// File descriptor for virtual disk.  Private to this file
// Yes, global variables are generally a bad idea...

int vdisk_fd = 0;

/**
 * Open the virtual disk
 *
 * @param virtual_disk_name Name of the file containing the virtual disk
 * @return 0 on success; < 0 on error
 *
 */
int vdisk_open(char* virtual_disk_name, int truncate_flag)
{
	if (vdisk_fd != 0) {
		fprintf(stderr, "A disk is already opened\n");
		return(-1);
	};

	// Are we truncating the file?
	if (truncate_flag)
		truncate_flag = O_TRUNC;

	// Open file
	int fd = open(virtual_disk_name, O_RDWR | O_CREAT | truncate_flag,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	// Check code
	if (fd <= 0) {
		fprintf(stderr, "Unable to open virtual disk (%s)\n", virtual_disk_name);
		return(-1);
	};

	// Remember the fd in the global variable
	vdisk_fd = fd;
	return(0);
};

/**
 * Close the virtual disk
 *
 * @return 0 on success; <0 for an error
 */
int vdisk_close()
{
	// Must be initialized to clos it
	if (vdisk_fd == 0) {
		fprintf(stderr, "vdisk_close(): disk not initialized\n");
		exit(-1);
	};

	// Close the file
	close(vdisk_fd);

	// Mark as closed
	vdisk_fd = 0;
	return(0);
}

/**
 *  Read a disk block into the provided buffer
 *
 * @param block_ref Index of the block that is to be loaded
 * @param block Pointer to the buffer that the read block will be placed into
 * @return 0 on success; <0 on error
 *
 */
int vdisk_read_block(BLOCK_REFERENCE block_ref, void* block)
{
	if (debug)
		fprintf(stderr, "##Reading block %d\n", block_ref);

	// Make sure that the disk is initialized
	if (vdisk_fd == 0) {
		fprintf(stderr, "vdisk_read_block(): disk not initialized\n");
		exit(-1);
	};

	// Make sure that we have a valid block request
	if (block_ref >= MAX_BLOCKS) {
		fprintf(stderr, "vdisk_read_block(): bad block_ref(%d)\n", block_ref);
		return(-2);
	}

	// Lsek to the correct point in the file
	if (lseek(vdisk_fd, block_ref * BLOCK_SIZE, SEEK_SET) < 0) {
		fprintf(stderr, "vdisk_read_block(): seek failed\n");
		return(-3);
	}

	// Read the block
	if (read(vdisk_fd, block, BLOCK_SIZE) != BLOCK_SIZE) {
		fprintf(stderr, "vdisk_read_block(): read failed\n");
		return(-4);
	}

	// Success
	return(0);
}

/**
 *  Write a disk block to the virtual disk
 *
 * @param block_ref Index to the block to be written
 * @param block Memory in which the block is currently stored
 *
 */
int vdisk_write_block(BLOCK_REFERENCE block_ref, void* block)
{
	if (debug)
		fprintf(stderr, "##Writing block %d\n", block_ref);

	// File open?
	if (vdisk_fd == 0) {
		fprintf(stderr, "vdisk_write_block(): disk not initialized\n");
		exit(-1);
	};

	// Is it a valid block request?
	if (block_ref >= MAX_BLOCKS) {
		fprintf(stderr, "vdisk_write_block(): bad block_ref(%d)\n", block_ref);
		return(-2);
	}

	// Move to the beginning of the block
	if (lseek(vdisk_fd, block_ref * BLOCK_SIZE, SEEK_SET) < 0) {
		fprintf(stderr, "vdisk_write_block(): seek failed\n");
		return(-3);
	}

	// Read the block
	if (write(vdisk_fd, block, BLOCK_SIZE) != BLOCK_SIZE) {
		fprintf(stderr, "vdisk_write_block(): read failed\n");
		return(-4);
	}

	// Success
	return(0);
}
