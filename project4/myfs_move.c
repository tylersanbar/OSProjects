#include <stdio.h>
#include <string.h>

#include "myfs_lib.h"
#include "vdisk.h"

#define BUF_SIZE 500

int main(int argc, char** argv) {
	// Fetch the key environment vars
	char cwd[MAX_PATH_LENGTH];
	char disk_name[MAX_PATH_LENGTH];

	unsigned char buf[BUF_SIZE];

	myfs_get_environment(cwd, disk_name);

	// Open the virtual disk
	vdisk_open(disk_name, 0);
	if (argc != 3) {
		fprintf(stderr, "Usage: myfs_move <SRC> <DEST>\n");
	}
	else {
		if (myfs_move(cwd, argv[1], argv[2]) < 0) {
			exit(-1);
		}
	}

	// Clean up
	vdisk_close();

	return(0);
}
