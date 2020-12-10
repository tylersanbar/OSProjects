#include <stdio.h>
#include <stdlib.h>
#include "vdisk.h"
#include "myfs_lib_support.h"

/**
 * Set all of the properties of an index node
 *
 * @param index_node   (OUT) Pointer to the index node structure to be initialized
 * @param type         (IN)  Type of index node
 * @param n_references (IN)  Number of references to this index node
 *          (when first created, will always be 1)
 * @param content      (IN) Block reference to the block that contains the information within this index node
 * @param size         (IN) Size of the inode (# of directory entries or size of file in bytes)
 *
 * PROVIDED
 */

void myfs_set_index_node(INDEX_NODE *index_node, INDEX_NODE_TYPE type, int n_references,
			 BLOCK_REFERENCE content, int size)
{
  index_node->type = type;
  index_node->references = n_references;
  index_node->content = content;
  index_node->size = size;
}



/**
 *  Given an index node reference, read the index node from the virtual disk.
 *
 *  @param r_index_node (IN) reference 
 *  @param index_node   (OUT) Pointer to an index node memory structure.  This structure will be
 *                filled in before return)
 *  @return 0 = successfully loaded the index node
 *         -1 = an error has occurred
 *
 * PROVIDED
 */
int myfs_read_index_node_by_reference(INDEX_NODE_REFERENCE r_index_node, INDEX_NODE *index_node)
{
  if(debug)
    fprintf(stderr, "\tDEBUG: Reading index node %d\n", r_index_node);

  // Find the address of the index node

  // Relative block
  BLOCK_REFERENCE r_block = r_index_node / N_INDEX_NODES_PER_BLOCK;

  // elemnt within the block
  int element = (r_index_node % N_INDEX_NODES_PER_BLOCK);

  // Block that contins the index node: read it first
  BLOCK b;

  int ref = ROOT_INDEX_NODE_BLOCK;

  // Walk down the linked list of index nodes block to find the right one
  for(int i = 0; i < r_block && ref != UNALLOCATED_BLOCK; ++i) {
    // Get the disk block
    if(vdisk_read_block(ref, &b) != 0) {
      return(-1);
    }
    // Fetch the next block
    ref = b.next;
  }

  // Check that we have a valid block
  if(ref == UNALLOCATED_BLOCK) {
    return(-1);
  }
  
  // Read the block that contains the index node
  if(vdisk_read_block(ref, &b) != 0) {
    return(-1);
  }

  // Copy the contents of the index node into the return structure
  //  (yes, we can do this with entire structs!)
  *index_node = b.content.index_nodes.index_node[element];

  // Success
  return(0);
}


/**
 * Write a single index node to the disk
 *
 * @param r_index_node  (IN) Index node reference index
 * @param index_node    (IN) Pointer to an index node structure to write
 * @return 0 if success
 *         -x if error
 *
 * 
 */
int myfs_write_index_node_by_reference(INDEX_NODE_REFERENCE r_index_node, INDEX_NODE *index_node)
{
  if(debug)
    fprintf(stderr, "\tDEBUG: Writing index node %d\n", r_index_node);

  // Find the address of the index node

  // Relative block
  BLOCK_REFERENCE r_block = r_index_node / N_INDEX_NODES_PER_BLOCK;

  // elemnt within the block
  int element = (r_index_node % N_INDEX_NODES_PER_BLOCK);

  // Block that contins the index node: read it first
  BLOCK b;

  int ref = ROOT_INDEX_NODE_BLOCK;

  // Walk down the linked list of index nodes block to find the right one
  for (int i = 0; i < r_block && ref != UNALLOCATED_BLOCK; ++i) {
      // Get the disk block
      if (vdisk_read_block(ref, &b) != 0) {
          return(-1);
      }
      // Fetch the next block
      ref = b.next;
  }

  // Check that we have a valid block
  if (ref == UNALLOCATED_BLOCK) {
      return(-1);
  }

  // Read the block that contains the index node
  if (vdisk_read_block(ref, &b) != 0) {
      return(-1);
  }

  // Copy the contents of index node pointer into the read index node array
  memcpy(&b.content.index_nodes.index_node[element], index_node, sizeof(INDEX_NODE));

  //Write the modified block back to the disk
  if (vdisk_write_block(ref, &b) != 0) {
      return(-1);
  }
  // Success
  return(0);
}



/**
 *  Initialize an index node and a directory block structure as a new directory.
 *  - Index node points to directory block (self_block_reference)
 *  - Index node size = 2 (for . and ..)
 *  - Direcory block: add entries . (self_index_node_reference and .. (parent_index_node_reference)
 *  -- Set all other entries to UNALLOCATED_INDEX_NODE
 *
 * @param index_node (OUT) Pointer to index node structure to initialize
 * @param block      (OUT) Pointer to block structure to initialize as a directory block
 * @param self_block_reference        (IN) The block reference to the new directory block
 * @param self_index_node_reference   (IN) The index node reference to the new index node
 * @param parent_index_node_reference (IN) The index node reference to the parent index node
 *
 */
void myfs_init_directory_block_and_index_node(INDEX_NODE *index_node, BLOCK *block,
				    BLOCK_REFERENCE self_block_reference,
				    INDEX_NODE_REFERENCE self_index_node_reference,
				    INDEX_NODE_REFERENCE parent_index_node_reference)
{
    //Initialize index node
    myfs_set_index_node(index_node, T_DIRECTORY, 1, self_block_reference, 2);
    //Set block as directory with cleared entries
    myfs_clear_all_directory_entries(block);
    //Add self entry "."
    block->content.directory.entry[0].index_node_reference = self_index_node_reference;
    strncpy(block->content.directory.entry[0].name, ".", FILE_NAME_SIZE);
    //Add parent entry ".."
    block->content.directory.entry[1].index_node_reference = parent_index_node_reference;
    strncpy(block->content.directory.entry[1].name, "..", FILE_NAME_SIZE);
}

/**
   Clear all of the entries in a directory block.
   - Set names to the empty string
   - Set index_node_reference to UNALLOCATED_INDEX_NODE

   @param directory_block (OUT) Pointer to an existing block that is to be cleared out

 */
void myfs_clear_all_directory_entries(BLOCK *directory_block)
{
    for (int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++) {
        directory_block->content.directory.entry[i].name[0] = 0;
        directory_block->content.directory.entry[i].index_node_reference = UNALLOCATED_INDEX_NODE;
    }
    
}


/**
   Initialize a new index node block.  Set all entires:
   - type to T_UNUSED
   - content to UNALLOCATED_BLOCK

   @param index_node_block (OUT) Pointer to a block that will be filled in
 */
void myfs_clear_all_index_node_entries(BLOCK *index_node_block)
{
  INDEX_NODE index_node;
  //Set index node
  myfs_set_index_node(&index_node, T_UNUSED, 0, UNALLOCATED_BLOCK, 0);
  //Set entries in index node block to index node
  for (int i = 0; i < N_INDEX_NODES_PER_BLOCK; i++) {
      index_node_block->content.index_nodes.index_node[i] = index_node;
  }
}

/*
 * Given a valid directory index_node, return the index_node reference for the sub-item
 * that matches <element_name>
 *
 * @param index_node      (IN) Pointer to a loaded index_node structure.  Must be a directory index_node
 * @param element_name    (IN) Name of the directory element to look up
 *
 * @return = INDEX_NODE_REFERENCE for the sub-item if found; UNALLOCATED_INDEX_NODE if not found
 *
 * 
 */

INDEX_NODE_REFERENCE myfs_find_entity_in_directory(INDEX_NODE *index_node, char *element_name)
{
  if(debug)
    fprintf(stderr,"\tDEBUG: myfs_find_entity_in_directory: %s\n", element_name);

  BLOCK_REFERENCE ref = index_node->content;

  BLOCK block;

  // Loop over all directory blocks in the linked list
  while(ref != UNALLOCATED_BLOCK) {
    if(debug)
      fprintf(stderr,"\tDEBUG: myfs_find_entity_in_directory() check block: %d\n", ref);

    // Read the directory data block
    if(vdisk_read_block(ref, &block) != 0) {
      return(UNALLOCATED_INDEX_NODE);
    }
  
    // Loop over all entries
    for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; ++i) {
      if(debug)
	fprintf(stderr, "\tDEBUG: Check: %d %s with %s\n", i,
		block.content.directory.entry[i].name, element_name);
      if(strncmp(block.content.directory.entry[i].name, element_name, FILE_NAME_SIZE) == 0) {
	// Match: we are done
	if(debug)
	  fprintf(stderr, "\tDEBUG: Match: %d\n", block.content.directory.entry[i].index_node_reference);
	return(block.content.directory.entry[i].index_node_reference);
      }
    }
    // Go to the next directory block
    ref = block.next;
  }

  // Did not find any matches
  if(debug)
    fprintf(stderr, "\tDEBUG: No match\n");

  return(UNALLOCATED_INDEX_NODE);
}

/**
 *  Given a current working directory and either an absolute or relative path, find both the index node of the
 * file or directory and the index node of the parent directory.  If one or both are not found, then they are
 * set to UNALLOCATED_INDEX_NODE.
 *
 *  This implementation handles a variety of strange cases, such as consecutive /'s and /'s at the end of
 * of the path (we have to maintain some extra state to make this work properly).
 *
 * @param cwd (IN) Absolute path for the current working directory
 * @param path (IN) Absolute or relative path of the file/directory to be found
 * @param parent (OUT) Pointer to the found index node reference for the parent directory
 * @param child (OUT) Pointer to the found node reference for the file or directory specified by path
 * @param local_name (OUT) String name of the file or directory without any path information
 *             (i.e., name relative to the parent).  If this pointer is NULL, then the name is
 *             not copied to this string
 * @return 0 if no errors
 *         -1 if child not found
 *         -x if an error
 *
 */
int myfs_path_to_index_node(char *cwd, char * path, INDEX_NODE_REFERENCE *parent,
			    INDEX_NODE_REFERENCE *child,
			    char *local_name)
{
  INDEX_NODE_REFERENCE grandparent;
  char full_path[MAX_PATH_LENGTH];

  // Construct an absolute path the file/directory in question
  if(path[0] == '/') {
    strncpy(full_path, path, MAX_PATH_LENGTH-1);
  }else{
    if(strlen(cwd) > 1) {
      strncpy(full_path, cwd, MAX_PATH_LENGTH-1);
      strncat(full_path, "/", 2);
      strncat(full_path, path, MAX_PATH_LENGTH-1-strnlen(full_path, MAX_PATH_LENGTH));
    }else{
      strncpy(full_path, "/", 2);
      strncat(full_path, path, MAX_PATH_LENGTH-2);
    }
  }

  if(debug) {
    fprintf(stderr, "\tDEBUG: Full path: %s\n", full_path);
  };

  // Start scanning from the root directory index_node
  // Root directory index_node
  grandparent = *parent = *child = 0;
  if(debug)
    fprintf(stderr, "\tDEBUG: Start search: %d\n", *parent);

  // Parse the full path
  char *directory_name;
  directory_name = strtok(full_path, "/");
  while(directory_name != NULL) {
    if(strlen(directory_name) >= FILE_NAME_SIZE-1) 
      // Truncate the name
      directory_name[FILE_NAME_SIZE - 1] = 0;
    if(debug){
      fprintf(stderr, "\tDEBUG: Directory: %s\n", directory_name);
    }
    if(strlen(directory_name) != 0) {
      // We have a non-empty name
      // Remember this name
      if(local_name != NULL) {
	// Copy local name of file 
	strncpy(local_name, directory_name, MAX_PATH_LENGTH-1);
	// Make sure we have a termination
	local_name[MAX_PATH_LENGTH-1] = 0;
      }

      // Real next element
      INDEX_NODE index_node;
      // Fetch the index_node that corresponds to the child
      if(myfs_read_index_node_by_reference(*child, &index_node) != 0) {
	return(-3);
      }

      // Check the type of the index_node
      if(index_node.type != T_DIRECTORY) {
	// Parent is not a directory
	*parent = *child = UNALLOCATED_INDEX_NODE;
	return(-2);  // Not a valid directory
      }
      // Get the new index_node that corresponds to the name by searching the current directory
      INDEX_NODE_REFERENCE new_index_node = myfs_find_entity_in_directory(&index_node, directory_name);
      grandparent = *parent;
      *parent = *child;
      *child = new_index_node;
      if(new_index_node == UNALLOCATED_INDEX_NODE) {
	// name not found
	//  Is there another (nontrivial) step in the path?
	//  Loop until end or we have found a nontrivial name
	do {
	  directory_name = strtok(NULL, "/");
	  if(directory_name != NULL && strlen(directory_name) >= FILE_NAME_SIZE-1) 
	    // Truncate the name
	    directory_name[FILE_NAME_SIZE - 1] = 0;
	}while(directory_name != NULL && (strcmp(directory_name, "") == 0));
	
	if(directory_name != NULL) {
	  // There are more sub-items - so the parent does not exist
	  *parent = UNALLOCATED_INDEX_NODE;
	};
	// Directory/file does not exist
	return(-1);
      };
    }
    // Go on to the next directory
    directory_name = strtok(NULL, "/");
    if(directory_name != NULL && strlen(directory_name) >= FILE_NAME_SIZE-1) 
      // Truncate the name
      directory_name[FILE_NAME_SIZE - 1] = 0;
  };

  // Item found.
  if(*child == UNALLOCATED_INDEX_NODE) {
    // We went too far - roll back one step ***
    *child = *parent;
    *parent = grandparent;
  }
  
  if(debug) {
    fprintf(stderr, "\tDEBUG: Found: %d, %d\n", *parent, *child);
  }
  
  // Success!
  return(0);
} 

/**
   Scan through a sequence of directory blocks in order to find an unused directory entry.

   @param head_directory_block_ref (IN) Reference to the block that contains the first
                                   directory block 
   @param directory_block          (OUT) Pointer to the directory block structure that
                                   contains the hole or the last directory block if
				   there is no hole
   @param directory_block_ref      (OUT) Reference to the directory block that contains the
                                   hole or the last directory if there is no hole
   @param directory_index          (OUT) Index of the directory hole in the block or
                                   undefined if there is no hole			   
				   
 */

int myfs_find_directory_hole(BLOCK_REFERENCE head_directory_block_ref,
    BLOCK* directory_block,
    BLOCK_REFERENCE* directory_block_ref,
    int* directory_index)
{
    *directory_block_ref = head_directory_block_ref;
    int last_block_ref = UNALLOCATED_BLOCK;

    while (*directory_block_ref != UNALLOCATED_BLOCK) {
        // Remember last valid block reference
        last_block_ref = *directory_block_ref;

        // Read the block
        if (vdisk_read_block(*directory_block_ref, directory_block) < 0) {
            fprintf(stderr, "Unable to read directory block\n");
            exit(-1);
        }

        // Scan through the directory entries
        for (int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; ++i) {
            if (directory_block->content.directory.entry[i].index_node_reference == UNALLOCATED_INDEX_NODE) {
                // Found a hole!
                *directory_index = i;
                return(1);
            }
        }

        // Move to the next directory block
        *directory_block_ref = directory_block->next;
    }

    // Not found
    // Provide the block ref for the last directory block
    *directory_block_ref = last_block_ref;
    return(0);
}

/**
   Allocate a new block and append it onto an existing linked list of blocks

   @param volume_block (IN) Pointer to the volume block.  Note: updates not written to disk
   @param block        (IN) Existing block to append to.  Changes written to disk.
                             (OUT) Contents are replaced with the new block.
			     Note: updates not written to disk
   @param block_ref    (IN) Pointer to block reference for the existing block
                       (OUT) Contents replaced with the newly allocated block reference
   @return int         Returns 0 if success
 */
int myfs_append_new_block_to_existing_block(BLOCK *volume_block,
					    BLOCK *block, BLOCK_REFERENCE *block_ref)
{
    //Allocate new block and get block reference
    BLOCK_REFERENCE newRef = myfs_allocate_new_block(volume_block);
    //If no space for new block, return unallocated block
    if (newRef == UNALLOCATED_BLOCK) {
        fprintf(stderr, "Unable to allocate new block, append failed.");
        return UNALLOCATED_BLOCK;
    }
    BLOCK newBlock;
    //Set appended block to existing block's next reference
    newBlock.next = block->next;
    //Set existing block's next to appended block reference
    block->next = newRef;
    //Write changes to existing block to disk
    if (vdisk_write_block(*block_ref, block) < 0) {
        // Fatal error
        fprintf(stderr, "myfs_append_new_block_to_existing_block: Unable to write block %d\n", (int)*block_ref);
        exit(-1);
    }
    //Rereference input to new block
    *block = newBlock;
    *block_ref = newRef;
    //Return success
    return 0;
}

/**
   Scan through the index node blocks & try to find a hole

   @param index_node_block (OUT) Last block of the linked list
   @param index_block_ref  (OUT) Last block ref in the linked list
 */
INDEX_NODE_REFERENCE myfs_find_index_node_hole(BLOCK* index_node_block,
    BLOCK_REFERENCE* index_block_ref)
{
    BLOCK_REFERENCE bref = ROOT_INDEX_NODE_BLOCK;
    INDEX_NODE_REFERENCE ret = 0;

    while (bref != UNALLOCATED_BLOCK) {
        *index_block_ref = bref;
        // Load the block
        if (vdisk_read_block(bref, index_node_block) < 0) {
            // Fatal error
            fprintf(stderr, "myfs_find_index_node_hole: Unable to read block %d\n", bref);
            exit(-1);
        }

        // Scan the block for unused entries
        for (int i = 0; i < N_INDEX_NODES_PER_BLOCK; ++i, ++ret) {
            if (index_node_block->content.index_nodes.index_node[i].type == T_UNUSED) {
                // Found one!
                return(ret);
            }
        }

        // Not found in this block. Go to the next one in the linked list
        bref = index_node_block->next;
    }

    // Not found at all
    return(UNALLOCATED_INDEX_NODE);
}

/**
 * Return the bit index for the first 0 bit in a byte (starting from 7th bit
 *   and scanning to the right)
 *
 * @param value: a byte
 * @return The bit number of the first 0 in value (starting from the 7th bit
 *         -1 if no zero bit is found
 */

int myfs_find_open_bit(unsigned char value)
{
    if (debug) {
        fprintf(stderr, "Finding open bit of %d.\n", value);
    }
    for (int i = 0; i < 8; i++) {

        //If least significant bit is equal to 0, return index
        if ((value & 1) == 0) {
            if (debug) {
                fprintf(stderr, "Open bit found: %d.\n", i);
            }
            return i;
        }
        //Check next bit
        value = value >> 1;
    }
    //No zero bit found
    if (debug) {
        fprintf(stderr, "No open bit found.\n");
    }
    return -1;
}

/**
 * Allocate a new block
 * - If one is found, then the free block linked list is updated
 *
 * @param volume_block  (IN/OUT) A link to a buffer ALREADY containing the data from the volume block.
 *    This buffer may be modified (but will not be written to the disk; we will let
 *    the calling function handle this).
 *
 * @return The index of the allocated data block.  If no blocks are available,
 *        then UNALLOCATED_BLOCK is returned
 *
 */
BLOCK_REFERENCE myfs_allocate_new_block(BLOCK *volume_block)
{
    if (debug) {
        fprintf(stderr, "Allocating new block.\n");
    }
    BLOCK_REFERENCE free_ref;
    //If volume block is full, return UNALLOCATED_BLOCK
    if (volume_block->content.volume.n_allocated_blocks >= volume_block->content.volume.n_blocks) {
        fprintf(stderr, "Unable to allocate block, no free blocks available.\n");
        return UNALLOCATED_BLOCK;
    }
    //Find unallocated block
    for (int i = 0; i < sizeof(volume_block->content.volume.block_allocation_table); i++) {
        //If byte has unallocated blocks
        if (volume_block->content.volume.block_allocation_table[i] < 0xFF) {
            //Find the open bit
            int openBit = myfs_find_open_bit(volume_block->content.volume.block_allocation_table[i]);
            //Set the free reference variable to (byte index * 8 + openBit)
            free_ref = (i * 8) + openBit;
            //Set found open bit to 1
            volume_block->content.volume.block_allocation_table[i] = volume_block->content.volume.block_allocation_table[i] | (1 << openBit);
            //Increment number of allocated blocks
            volume_block->content.volume.n_allocated_blocks++;
            //Return block reference
            if (debug) {
                fprintf(stderr, "Allocation success, Block reference is %d.\n", (int)free_ref);
            }
            return free_ref;
        }
    }
    //If we didn't find an unallocated block, big woops error
    fprintf(stderr, "Volume block's number of allocated blocks indicated it was not full, but block allocation table was full. Error.");
    return UNALLOCATED_BLOCK;
}

/***
    Remove an entry from a directory block linked list

    @param dir_ref (IN) Block reference to an existing directory
    @param name    (IN) String name of the entity within the directory in question

    @return 0 on success; -x on error
 */

int myfs_remove_directory_entry(BLOCK_REFERENCE dir_ref, char *name)
{
    BLOCK dir_block;
    //Search through the linked list of directory blocks
    while (dir_ref != UNALLOCATED_BLOCK) {
        //Read the directory block
        if (vdisk_read_block(dir_ref, &dir_block) < 0) {
            fprintf(stderr, "Unable to read directory block\n");
            exit(-1);
        }
        //For each entry in directory block
        for (int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++) {
            //If if the entry matches
            if (strcmp(dir_block.content.directory.entry[i].name, name) == 0) {
                //Set the name to the empty string
                strncpy(dir_block.content.directory.entry[i].name, "", FILE_NAME_SIZE);
                //Set index node reference to UNALLOCATED_INDEX_NODE
                dir_block.content.directory.entry[i].index_node_reference = UNALLOCATED_INDEX_NODE;
                //Return success
                if (vdisk_write_block(dir_ref, &dir_block) < 0) {
                    fprintf(stderr, "Unable to write directory block\n");
                    exit(-1);
                }
                return 0;
            }
        }
        dir_ref = dir_block.next;
    }
    //If we didn't find any entries, return error
    fprintf(stderr, "Directory entry not found.\n");
    return -1;
}


/**
 * Deallocate linked list of blocks
 * - Modify the in-memory copy of the volume block
 * - Add the specified block to THE END of the free block linked list
 * - Modify the disk copy of the deallocated block: next_block points to
 *     UNALLOCATED_BLOCK
 *   
 *
 * @param volume_block (IN/OUT) Pointer to a loaded volume block.  Changes to the MB will
 *           be made here, but not written to disk
 *
 * @param block_reference (IN) Reference to the head block that is being deallocated
 *
 */
int myfs_deallocate_blocks(BLOCK *volume_block, BLOCK_REFERENCE bref)
{
    if (bref > UNALLOCATED_BLOCK | bref < 0) {
        fprintf(stderr, "Trying to deallocate an invalid block reference, error");
        return -1;
    }
    BLOCK block;
    BLOCK_REFERENCE nextBlock;
    //While we still have blocks in the linked list
    while (bref != UNALLOCATED_BLOCK) {
        //Set bit at (BLOCK_REFERENCE % 8) in byte located at (BLOCK_REFERENCE / 8) to 0
        volume_block->content.volume.block_allocation_table[bref / 8] = (volume_block->content.volume.block_allocation_table[bref / 8] ^ (1 << (bref % 8)));
        //Read block
        if (vdisk_read_block(bref, &block) < 0) {
            fprintf(stderr, "Unable to read block\n");
            exit(-1);
        }
        //Get next block
        nextBlock = block.next;
        //Set next block to unallocated
        block.next = UNALLOCATED_BLOCK;
        //Write block
        if (vdisk_write_block(bref, &block) < 0) {
            fprintf(stderr, "Unable to write block\n");
            exit(-1);
        }
        //Set bref to next
        bref = nextBlock;
    }
    volume_block->content.volume.n_allocated_blocks--;
    //Success
    return 0;
};

