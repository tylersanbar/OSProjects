/**
Remove a directory from MY File System.

CS3113

Author: Andrew H. Fagg (CS3113)

Usage: myfs_rmd <dirname>

Algorithm:
1. Read the current working directory and disk name from the environment vars
2. Open the virtual disk
3. Remove the specified dirname
4. Close the virtual disk


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
	if (argc == 2) {
		// Open the virtual disk
		vdisk_open(disk_name, 0);

		myfs_rmd(cwd, argv[1]);
		// Clean up
		vdisk_close();

	}
	else {
		fprintf(stderr, "Usage: myfs_rmd <directory name>\n");
	}

}
