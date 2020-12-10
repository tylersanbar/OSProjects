/**

List a file or directory in MY File System.

Author: Andrew H. Fagg (CS3113)

Usage: myfs_list [<name>]

Algorithm:
1. Read the current working directory and disk name from the environment vars
2. Open the virtual disk
3. Use the specified file or directroy; Use the root directory if
none are specified
4. Generate the listing for the specified entity
5. Close the virtual disk


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

  // Open the virtual disk
  vdisk_open(disk_name, 0);
  if (debug) {
      fprintf(stderr, "Calling myfs_list with %d arguments.", argc);
  }
  // How list is called depends on the number of arguments
  if(argc == 1) {
    myfs_list(cwd, "");
  }else if(argc == 2){
    myfs_list(cwd, argv[1]);
  }else{
    fprintf(stderr, "Usage: myfs_list [<name>]\n");
  }

  // Clean up
  vdisk_close();
}
