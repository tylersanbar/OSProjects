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
		fprintf(stderr, "Usage: myfs_copy <SRC> <DEST>\n");
	}
	else {
		MYFILE* fps = myfs_fopen(cwd, argv[1], "r");
		if (fps == NULL) {
			fprintf(stderr, "Error opening source file.\n");
			exit(-1);
		}

		MYFILE* fpd = myfs_fopen(cwd, argv[2], "w");
		if (fpd == NULL) {
			fprintf(stderr, "Error opening destination file.\n");
			exit(-1);
		}

		int n;
		while ((n = myfs_fread(fps, buf, BUF_SIZE)) != 0) {
			myfs_fwrite(fpd, buf, n);
		}

		myfs_fclose(fps);
		myfs_fclose(fpd);
	}

	// Clean up
	vdisk_close();

	return(0);
}
