#include "myfs_lib.h"
#include "myfs_lib_support.h"
#include "vdisk.h"
#include "string.h"

// Yes ... a global variable
int debug = 0;

/**
 Read the MYFS_CWD, MYFS_DISK environment
 variables copy their values into cwd, disk_name an pipe_name_base.  If these
 environment variables are not set, then reasonable defaults are
 given.

 @param cwd String buffer in which to place the MYFS current working directory.
 @param disk_name String buffer in which to place file name of the virtual disk.

 PROVIDED
 */
void myfs_get_environment(char *cwd, char *disk_name)
{
  // Current working directory for the OUFS
  char *str = getenv("MYFS_CWD");
  if(str == NULL) {
    // Provide default
    strcpy(cwd, "/");
  }else{
    // Exists
    strncpy(cwd, str, MAX_PATH_LENGTH-1);
    cwd[MAX_PATH_LENGTH-1] = 0;
  }

  // Virtual disk location
  str = getenv("MYFS_DISK");
  if(str == NULL) {
    // Default
    strcpy(disk_name, "vdisk");
  }else{
    // Exists: copy
    strncpy(disk_name, str, MAX_PATH_LENGTH-1);
    disk_name[MAX_PATH_LENGTH-1] = 0;
  }
}

/**
 * Completely format the virtual disk (including creation of the space).
 *
 * NOTE: this function attaches to the virtual disk at the beginning and
 *  detaches after the format is complete.
 *
 * - Zero out all blocks on the disk.
 * - Initialize the volume block: mark index_node 0 as allocated and initialize
 *    the linked list of free blocks
 * - Initialize the ROOT_INDEX_NODE_BLOCK
 * - Initialize root directory index_node 
 * - Initialize the root directory in block ROOT_DIRECTORY_BLOCK
 *
 * @return 0 if no errors
 *         -x if an error has occurred.
 *
 * 
 */

int myfs_format_disk(char  *virtual_disk_name, int n_blocks)
{
  // Attach to the virtual disk
  if(vdisk_open(virtual_disk_name, 1) != 0) {
    return(-1);
  }

  BLOCK block;

  // Zero out block
  memset(&block, 0, BLOCK_SIZE);
  //Look throught each block and set it to zero block
  for (int i = 0; i < n_blocks; i++) {
      if(vdisk_write_block( i, &block) < 0)
  }
  // :

  // Done
  vdisk_close();
 
  return(0);
}


/**
 * Print out the specified file (if it exists) or the contents of the 
 *   specified directory (if it exists)
 *
 * If a directory is listed, then the valid contents are printed in sorted order
 *   (as defined by strcmp()), one per line.  We know that a directory entry is
 *   valid if the index_node_reference is not UNALLOCATED_INDEX_NODE.
 *   Hint: qsort() will do to sort for you.  You just have to provide a compareTo()-like 
 *   function (just like in Java!)
 *   Note: if an entry is a directory itself, then its name must be followed by "/".
 *        This added character is included in the sorting process
 *
 * @param cwd Absolute path representing the current working directory
 * @param path Absolute or relative path to the file/directory
 * @return 0 if success
 *         -x if error
 *
 * NOT PROVIDED
 */

int myfs_list(char *cwd, char *path)
{
}


///////////////////////////////////
/**
 * Make a new directory
 *
 * To be successful:
 *  - the parent must exist and be a directory
 *  - the child must not exist
 * Blocks that need to be allocated (in order):
 *  - (possibly) a new index node block, if there no index nodes available
 *  - (possibly) a new directory block, if the parent directory has no open entries available
 *  - Directory block for the new directory
 *
 * @param cwd Absolute path representing the current working directory
 * @param path Absolute or relative path to the file/directory
 * @return 0 if success
 *         -x if error
 *
 * 
 */
int myfs_mkd(char *cwd, char *path)
{
}


/**
 * Remove a directory
 *
 * To be successul:
 *  - The directory must exist and must be empty
 *  - The directory must not be . or ..
 *  - The directory must not be /
 *
 * @param cwd Absolute path representing the current working directory
 * @param path Absolute or relative path to the file/directory
 * @return 0 if success
 *         -x if error
 *
 * 
 */
int myfs_rmd(char *cwd, char *path)
{
}

