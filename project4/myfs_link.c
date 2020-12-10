/**
Make a hard link in MY FILE SYSTEM.

CS3113  Andrew H. Fagg

*/
#include <stdio.h>
#include <string.h>

#include "myfs_lib.h"
#include "vdisk.h"

int main(int argc, char** argv) {
	// Fetch the key environment vars
	char cwd[MAX_PATH_LENGTH];
	char disk_name[MAX_PATH_LENGTH];

	myfs_get_environment(cwd, disk_name);

	// Check arguments
	if (argc == 3) {
		// Open the virtual disk
		vdisk_open(disk_name, 0);

		myfs_link(cwd, argv[1], argv[2]);
		// Clean up
		vdisk_close();

	}
	else {
		fprintf(stderr, "Usage: myfs_link <src> <dst>\n");
		return(-1);
	}

	return(0);
}
