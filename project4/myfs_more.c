/**
Print the contents of a file in the OU File System.

CS3113 Andrew H. Fagg

*/

#include <stdio.h>
#include <string.h>

#include "myfs_lib.h"
#include "vdisk.h"

#define BUF_SIZE 1000

int main(int argc, char** argv) {
	// Fetch the key environment vars
	char cwd[MAX_PATH_LENGTH];
	char disk_name[MAX_PATH_LENGTH];

	myfs_get_environment(cwd, disk_name);

	// Open the virtual disk
	vdisk_open(disk_name, 0);

	// Check parameters
	if (argc != 2) {
		fprintf(stderr, "Usage: myfs_more <file name>\n");
	}
	else {
		MYFILE* fp = myfs_fopen(cwd, argv[1], "r");
		unsigned char buf[BUF_SIZE];
		if (fp != NULL) {
			// Successfully opened the file for reading
			int n;
			// Loop until the contents of the file are all printed to STDOUT
			while ((n = myfs_fread(fp, buf, BUF_SIZE)) != 0) {
				// Compiler generates warnings unless we receive the return
				//   value and do something with it
				int ret = write(1, buf, n);
				++ret;
			}

			// Clean up
			myfs_fclose(fp);
		}
	}

	// Clean up
	vdisk_close();

	return(0);
}
