/**
   Format MY File System disk with a specified number of blocks

   Author: Andrew H. Fagg (CS3113)

Usage: myfs_format [<num blocks>]

Algorithm:
1. If argument is specified, use it for the number of blocks
1b. Specified number of blocks must be between 10 and MAX_BLOCKS
2. Read the current working directory and disk name from the environment vars
3. Format the disk with the specified number of blocks

*/


#include <stdio.h>
#include <string.h>
#include "myfs_lib.h"

int main(int argc, char **argv)
{
  // Default is to use the maximum number of blocks
  int n_blocks = 32;

  // Parse the arguments
  if(argc == 2) {
    if(sscanf(argv[1], "%d", &n_blocks) != 1) {
      fprintf(stderr, "Usage: myfs_format [<number of blocks>]\n");
      exit(-1);
    }else{
      if(n_blocks < 10 || n_blocks > MAX_BLOCKS) {
	fprintf(stderr, "number of blocks must be between 10 and %d\n", MAX_BLOCKS);
	exit(-1);
      }
    }
  }else if(argc > 2) {
    fprintf(stderr, "Usage: myfs_format [<number of blocks>]\n");
    exit(-1);
  }
  
  // Get the environmental variables
  char cwd[MAX_PATH_LENGTH];
  char disk_name[MAX_PATH_LENGTH];
  myfs_get_environment(cwd, disk_name);

  // Format the disk
  myfs_format_disk(disk_name, n_blocks);

  return(0);

}
