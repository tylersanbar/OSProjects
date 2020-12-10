#include "myfs_lib.h"
#include "myfs_lib_support.h"
#include "vdisk.h"
#include "string.h"

// Yes ... a global variable
int debug = 1;

/**
 Read the MYFS_PWD, MYFS_DISK environment
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

int myfs_format_disk(char* virtual_disk_name, int n_blocks)
{
    // Attach to the virtual disk
    if (vdisk_open(virtual_disk_name, 1) != 0) {
        return(-1);
    }
    BLOCK block;
    //Zero out block----------------------------
    memset(&block, 0, sizeof(block));
    //Set next block to unallocated
    block.next = UNALLOCATED_BLOCK;
    //Loop through disk and write out to all blocks
    for (int i = 0; i < n_blocks; i++) {
        vdisk_write_block(i, &block);
    }
    //Set volume block--------------------------
    //Allocation table byte 0 = 0000 0111 = 7
    block.content.volume.block_allocation_table[0] = 7;
    //3 Blocks allocated, Root Index, Root Directory, Volume
    block.content.volume.n_allocated_blocks = 3;
    //1 Index node that points to directory
    block.content.volume.n_allocated_index_nodes = 1;
    //Set n_blocks
    block.content.volume.n_blocks = n_blocks;
    //Write out volume block
    if (vdisk_write_block(VOLUME_BLOCK_REFERENCE, &block) != 0) {
        return (-1);
    }
    //Set directory block------------------------
    //Set Index Node and Directory Block
    INDEX_NODE dirNode;
    myfs_init_directory_block_and_index_node(&dirNode, &block, ROOT_DIRECTORY_BLOCK, 0, 1);
    //Write Directory Block
    if (vdisk_write_block(ROOT_DIRECTORY_BLOCK, &block) != 0) {
        return (-1);
    }
    //Set index node block--------------------------
    //Clear out index node block
    myfs_clear_all_index_node_entries(&block);
    //Set root directory index node as entry in index node block
    block.content.index_nodes.index_node[ROOT_DIRECTORY_INDEX_NODE] = dirNode;
    //Write index node block
    if (vdisk_write_block(ROOT_INDEX_NODE_BLOCK, &block) != 0) {
        return (-1);
    }
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

int myfs_list(char* cwd, char* path)
{
    if (debug) {
        fprintf(stderr, "Listing directories with cwd = %s and path = %s.\n", cwd, path);
    }
    INDEX_NODE_REFERENCE parent_Ref;
    INDEX_NODE_REFERENCE new_Ref;
    INDEX_NODE new;
    char newPath[MAX_PATH_LENGTH];
    char local_name[MAX_PATH_LENGTH];
    //Assign path to modifiable string
    strcpy(newPath, path);
    //If no arguments provided for List, prepend cwd to path
    if (strcmp(newPath, "") == 0) strcpy(newPath, cwd);
    //Get index node references and local name
    int pathReturn = myfs_path_to_index_node(cwd, newPath, &parent_Ref, &new_Ref, local_name);
    if (pathReturn != 0) {
        if (pathReturn == -1) {
            fprintf(stderr, "File or directory does not exist.\n");
            return (-1);
        }
        fprintf(stderr, "Error finding path to file or directory.\n");
        return(-3);
    }
    if (debug) {
        fprintf(stderr,"Entry found at index node %d, with name %s.\n", (int)new_Ref, local_name);
    }
    //Check if index node is a directory, if not, print name
    if (myfs_read_index_node_by_reference(new_Ref, &new) != 0) {
        fprintf(stderr, "Unable to read index node.\n");
    }
    if (new.type != T_DIRECTORY) {
        printf("%s\n", local_name);
        return(0);
    }
    //Add "/" to local name
    strcat(local_name,"/");
    //Make array of strings with max number of directory entries
    char contents[MAX_ENTRIES_PER_DIRECTORY][MAX_PATH_LENGTH];
    //Add local name to contents
    strcpy(contents[0], local_name);
    //Next concents index
    int numOfElements = 1;
    //Initialize directory block and reference
    BLOCK directoryBlock;
    BLOCK_REFERENCE bRef = new.content;

    //Loop through linked list of directory blocks
    while (bRef != UNALLOCATED_BLOCK) {
        //Read directory block from index node
        if (vdisk_read_block(bRef, &directoryBlock) != 0) {
            return(-1);
        }
        if (debug) {
            fprintf(stderr, "Searching directory at index reference %d for entries.\n", (int)bRef);
        }
        //Next block we will check
        bRef = directoryBlock.next;
        //Check each entry in directory
        for (int i = 0; i < MAX_ENTRIES_PER_DIRECTORY; i++) {
            //Get reference for index node
            new_Ref = directoryBlock.content.directory.entry[i].index_node_reference;
            //If entry is valid, add to list
            if (new_Ref != UNALLOCATED_INDEX_NODE) {
                //Get name of entry
                strncpy(local_name, directoryBlock.content.directory.entry[i].name, MAX_PATH_LENGTH);
                //Get entry index node to check if it is a directory
                myfs_read_index_node_by_reference(new_Ref, &new);
                //If it is, add "/"
                if(new.type == T_DIRECTORY) strcat(local_name,"/");
                //Add name to list of contents
                strcpy(contents[numOfElements++],local_name);
                //If we've found all the entires, exit loop
                if (numOfElements == new.size) {
                    bRef = UNALLOCATED_BLOCK;
                    continue;
                }
            }
        }
        
    }
    //Sort list of names
    qsort( contents, numOfElements, MAX_PATH_LENGTH, cmpstr);
    //Print the sorted list of names
    for (int i = 0; i < numOfElements; i++) {
        printf("%s\n",contents[i]);
    }
    return(0);
}

//Provided function wrapper for cmpstr found on webpage
//https://www.benjaminwuethrich.dev/2015-03-07-sorting-strings-in-c-with-qsort.html
int cmpstr(const void* a, const void* b) {
    const char* aa = (const char*)a;
    const char* bb = (const char*)b;
    return strcmp(aa, bb);
}


///////////////////////////////////
/**
 * Make a new directory
 *
 * To be successful:
 *  - the parent must exist and be a directory
 *  - the new must not exist
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
    BLOCK volume;
    INDEX_NODE_REFERENCE parentIndexRef;
    INDEX_NODE_REFERENCE newIndexRef;
    INDEX_NODE parentIndexNode;
    INDEX_NODE newIndexNode;
    char local_name[MAX_PATH_LENGTH];
    BLOCK_REFERENCE lastRef;
    BLOCK lastBlock;
    int entryIndex;
    DIRECTORY_ENTRY newEntry;
    BLOCK newDirBlock;
    BLOCK_REFERENCE newDirRef;
    //Read volume block into memory
    if (vdisk_read_block(VOLUME_BLOCK_REFERENCE, &volume) != 0) {
        fprintf(stderr, "Unable to read volume block");
        return (-4);
    }
    //-----------------------INDEX NODES----------------
    //Get index references and local name from path
    if (myfs_path_to_index_node(cwd, path, &parentIndexRef, &newIndexRef, local_name) != -1) {
        fprintf(stderr, "Invalid path.\n");
        return(-1);
    }
    //Get parent index node from reference
    if (myfs_read_index_node_by_reference(parentIndexRef, &parentIndexNode) != 0) {
        fprintf(stderr, "Error reading parent index node.\n");
        return(-2);
    }
    //Check if parent is a directory
    if (parentIndexNode.type != T_DIRECTORY) {
        fprintf(stderr, "Parent is not a directory\n");
        return(-3);
    }
    //Assign new an index node reference
    newIndexRef = myfs_find_index_node_hole(&lastBlock, &lastRef);
    //If none are available, allocate new index node block
    if (newIndexRef == UNALLOCATED_INDEX_NODE) {
        //new will refer to the next index node after the last one found
        newIndexRef = lastRef++;
        //Append the new index node block to the last index node block
        if (myfs_append_new_block_to_existing_block(&volume, &lastBlock, &lastRef) != 0) {
            fprintf(stderr, "Unable to append new index node block");
            return(-1);
        }
        //Write new index node block to disk
        if (vdisk_write_block(lastRef, &lastBlock) < 0) {
            fprintf(stderr, "Unable to write new index node block.");
            exit(-1);
        }
    }
    //Increase number of allocated index nodes in volume block by 1
    volume.content.volume.n_allocated_index_nodes++;
    //-------------------PARENT DIRECTORY---------------------
    //Initialize new directory entry
    newEntry.index_node_reference = newIndexRef;
    strncpy(newEntry.name, local_name, FILE_NAME_SIZE);
    //Find directory hole in parent
    //If 0, append new directory block
    if (myfs_find_directory_hole(parentIndexNode.content, &lastBlock, &lastRef, &entryIndex) == 0) {
        //Append new directory block
        if (myfs_append_new_block_to_existing_block(&volume, &lastBlock, &lastRef) != 0) {
            fprintf(stderr, "Unable to append new directory block.");
            return(-1);
        }
        //Entry index will be first entry in new directory block
        entryIndex = 0;
    }
    //Increase parent index node size by 1 and write back to disk
    parentIndexNode.size++;
    myfs_write_index_node_by_reference(parentIndexRef, &parentIndexNode);
    //Assign new directory entry to directory entry array in parent
    lastBlock.content.directory.entry[entryIndex] = newEntry;
    //Write parent directory block to disk
    if (vdisk_write_block(lastRef, &lastBlock) != 0) {
        fprintf(stderr, "Unable to write parent directory block");
        return(-1);
    }
    //------------NEW DIRECTORY BLOCK---------------------------------
    //Allocate new block for new directory
    newDirRef = myfs_allocate_new_block(&volume);
    if (newDirRef == UNALLOCATED_BLOCK) {
        fprintf(stderr, "Unable to allocate new block for new directory");
        return(-1);
    }
    //Initialize new directory block and new index node
    myfs_init_directory_block_and_index_node(&newIndexNode, &newDirBlock, newDirRef, newIndexRef, parentIndexRef);
    //Set newDirBlock next ref to Unallocated
    newDirBlock.next = UNALLOCATED_BLOCK;
    //Write new directory block to disk
    if (vdisk_write_block(newDirRef, &newDirBlock) != 0) {
        fprintf(stderr, "Unable to write new directory block.\n");
        return (-5);
    }
    //Write new index node to disk
    myfs_write_index_node_by_reference(newIndexRef, &newIndexNode);
    //Write volume block back to disk
    if (vdisk_write_block(VOLUME_BLOCK_REFERENCE, &volume) != 0) {
        fprintf(stderr, "Unable to write volume block.\n");
        return (-5);
    }
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
    BLOCK volume;
    INDEX_NODE_REFERENCE parentIndexRef;
    INDEX_NODE_REFERENCE removeIndexRef;
    char local_name[MAX_PATH_LENGTH];
    INDEX_NODE removeIndexNode;
    INDEX_NODE parentIndexNode;
    //Read volume block into memory
    if (vdisk_read_block(VOLUME_BLOCK_REFERENCE, &volume) != 0) {
        fprintf(stderr, "Unable to read volume block");
        return (-4);
    }
    //Get index nodes for parent and directory to remove
    myfs_path_to_index_node(cwd, path, &parentIndexRef, &removeIndexRef, local_name);
    //If it doesn't exist, error
    if (removeIndexRef == UNALLOCATED_BLOCK) {
        fprintf(stderr, "Directory does not exist.\n");
        return -1;
    }
    //If invalid name, error
    if (!(strcmp(local_name, "/") & strcmp(local_name, ".") & strcmp(local_name, ".."))) {
        fprintf(stderr, "Can not remove directory named %s\n", local_name);
        return -2;
    }
    //Read removed index node
    myfs_read_index_node_by_reference(removeIndexRef, &removeIndexNode);
    //If not empty, error
    if (removeIndexNode.size > 2) {
        fprintf(stderr, "Directory not empty, can not remove.\n");
        return -2;
    }
    //Deallocate directory block we are removing
    if (myfs_deallocate_blocks(&volume, removeIndexNode.content) != 0) {
        fprintf(stderr, "Unable to deallocate block for removal request.\n");
        return -1;
    }
    //Set removed index node to unused
    myfs_set_index_node(&removeIndexNode, T_UNUSED, 0, UNALLOCATED_BLOCK, 0);
    //Write removed index node to disk
    if (myfs_write_index_node_by_reference(removeIndexRef, &removeIndexNode) != 0) {
        fprintf(stderr, "Unable to write removed index node.\n");
        return -1;
    }
    //Decrement number of allocated index nodes in volume block
    volume.content.volume.n_allocated_index_nodes--;
    //Read parent index node
    if(myfs_read_index_node_by_reference(parentIndexRef, &parentIndexNode) != 0) {
        fprintf(stderr, "Unable to read parent index node.\n");
        return -1;
    }
    //Remove entry from parents directory list
    if (myfs_remove_directory_entry(parentIndexNode.content, local_name) != 0) {
        fprintf(stderr, "Unable to remove directory entry from parent directory list.\n");
        return -1;
    }
    //Decrement size of parent index node
    parentIndexNode.size--;
    //Write parent index node to disk
    if (myfs_write_index_node_by_reference(parentIndexRef, &parentIndexNode) != 0) {
        fprintf(stderr, "Unable to write parent index node.\n");
        return -1;
    }
    if (vdisk_write_block(VOLUME_BLOCK_REFERENCE, &volume) != 0) {
        fprintf(stderr, "Unable to write volume block");
        return (-5);
    }
}

