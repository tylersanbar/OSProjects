#include <stdio.h>
#include <string.h>

#include "myfs_lib.h" 
#include "vdisk.h"

#define BUF_SIZE 100

int main(int argc, char** argv) {
	// Fetch the key environment vars
	char cwd[MAX_PATH_LENGTH];
	char disk_name[MAX_PATH_LENGTH];

	myfs_get_environment(cwd, disk_name);

	// Open the virtual disk
	vdisk_open(disk_name, 0);
	if (argc == 1) {
		fprintf(stderr, "Usage: myfs_create <file name>\n");
	}
	else {
		MYFILE* fp = myfs_fopen(cwd, argv[1], "w");
		unsigned char buf[BUF_SIZE];
		if (fp != NULL) {
			int n;
			while ((n = read(0, buf, BUF_SIZE)) != 0) {
				myfs_fwrite(fp, buf, n);
			}

			myfs_fclose(fp);
		}
	}

	// Clean up
	vdisk_close();

	return(0);
}
