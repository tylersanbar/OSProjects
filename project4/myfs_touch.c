/**
   If the specified file does not exist, then create it.

   Author: CS 3113

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

	if (argc != 2) {
		fprintf(stderr, "Usage: myfs_touch <file name>\n");
	}
	else {
		// Open the virtual disk
		vdisk_open(disk_name, 0);

		// Open the file
		MYFILE* fp = myfs_fopen(cwd, argv[1], "a");

		if (fp != NULL) {
			myfs_fclose(fp);
		}
		else {
			fprintf(stderr, "Error opening file.\n");
		}
		// Clean up
		vdisk_close();
	}
	return(0);
}
