/**
Make a directory in MY File System.

Author: Andrew H. Fagg (CS3113)

Usage: myfs_mkd <dirname>

Algorithm:
1. Read the current working directory and disk name from the environment vars
2. Open the virtual disk
3. Create the specified dirname
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
		// Expected number of args

		// Open the virtual disk
		vdisk_open(disk_name, 0);

		// Make the specified directory
		int ret = myfs_mkd(cwd, argv[1]);
		if (ret != 0) {
			fprintf(stderr, "Error (%d)\n", ret);
		}

		// Clean up
		vdisk_close();

	}
	else {
		// Wrong number of parameters
		fprintf(stderr, "Usage: myfs_mkd <dirname>\n");
	}
}
