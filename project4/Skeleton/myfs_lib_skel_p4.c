
/*********************************************************************/
// Project 4
/**
 * Open a file or pipe
 * - mode = "r": the file must exist; offset is set to 0
 * - mode = "w": the file may or may not exist;
 *                 - if it does not exist, it is created 
 *                 - if it does exist, then the file is truncated
 *                       (size=0 and data blocks deallocated);
 *                 offset = 0 and size = 0
 * - mode = "a": the file may or may not exist
 *                 - if it does not exist, it is created 
 *                 - if it does exist, offset = size
 *
 * @param cwd Absolute path for the current working directory
 * @param path Relative or absolute path for the file in question
 * @param mode String: one of "r", "w" or "a"
 *                 (note: only the first character matters here)
 * @return Pointer to a new MYFILE structure if success
 *         NULL if error
 **/

MYFILE* myfs_fopen(char *cwd, char *path, char *mode)
{
  INDEX_NODE_REFERENCE parent;
  INDEX_NODE_REFERENCE child;
  char local_name[MAX_PATH_LENGTH];
  INDEX_NODE index_node;
  int ret;

  // Check for valid mode
  if(mode[0] != 'r' && mode[0] != 'w' && mode[0] != 'a') {
    fprintf(stderr, "fopen(): bad mode.\n");
    return(NULL);
  };

  // Try to find the index_node of the child
  if((ret = myfs_path_to_index_node(cwd, path, &parent, &child, local_name)) < -1) {
    if(debug)
      fprintf(stderr, "myfs_fopen(%d)\n", ret);
    return(NULL);
  }
  
  if(parent == UNALLOCATED_INDEX_NODE) {
    fprintf(stderr, "Parent directory not found.\n");
    return(NULL);
  }

  // Fetch the volume block
  BLOCK volume_block;
  if(vdisk_read_block(VOLUME_BLOCK_REFERENCE, &volume_block) < 0)
    return(NULL);
  
  // Now try to open
  if(child == UNALLOCATED_INDEX_NODE && mode[0] == 'r') {
    // Read a non-existent file
    fprintf(stderr, "File not found.\n");
    return(NULL);
  }else if(child == UNALLOCATED_INDEX_NODE) {
    // Write or append a non-existent file

    // Open non-existent file for writing
    child = myfs_create_file(&volume_block, parent, local_name);
    if(child == UNALLOCATED_INDEX_NODE) {
      // Error creating new file
      return(NULL);
    };

    // Write the changes back to the volume block
    if(vdisk_write_block(VOLUME_BLOCK_REFERENCE, &volume_block) < 0) {
      // Error
      return(NULL);
      }
    
    // Get the index_node for the child
    if(myfs_read_index_node_by_reference(child, &index_node) != 0) {
      return(NULL);
    };
    
    if(index_node.type != T_FILE) {
      // not a file: this should not happen
      fprintf(stderr, "Not a file.\n");
      return(NULL);
    }
  }else{
    // File exists
    // Get the index_node
    if(myfs_read_index_node_by_reference(child, &index_node) != 0) {
      return(NULL);
    }
    if(index_node.type == T_DIRECTORY) {
      // not a file
      fprintf(stderr, "Not a file.\n");
      return(NULL);
    }
  }
  
  // Child has a valid index_node value and we have loaded the index_node
  if(debug)
    fprintf(stderr, "Now opening file...\n");

  // Create the file structure
  MYFILE *fp = malloc(sizeof(MYFILE));
  fp->index_node_reference = child;
  fp->mode = mode[0];

  // Opening is different for files and pipes
  if(index_node.type == T_FILE) {
    // Not used
    fp->fd_external = -1;

    // What mode are we opening for?
    if(fp->mode == 'a'){
      // Open for append
      fp->offset = index_node.size;
    }else{
      // Open for writing or reading
      fp->offset = 0;
      if(fp->mode == 'w' && index_node.size > 0){
	if(debug)
	  fprintf(stderr, "Deallocating pre-existing blocks\n");
	index_node.size = 0;
	// Deallocate blocks owned by index_node
	myfs_deallocate_blocks(&volume_block, index_node.content);
	// Now set to no content
	index_node.content = UNALLOCATED_BLOCK;

	if(vdisk_write_block(VOLUME_BLOCK_REFERENCE, &volume_block) < 0) {
	  // Error
	  free(fp);
	  return(NULL);
	}

	// Update the index_node on disk
	if(myfs_write_index_node_by_reference(child, &index_node) != 0){
	  // Error
	  free(fp);
	  return(NULL);
	}
      }
    }

    if(debug)
      fprintf(stderr, "Now caching data block refs\n");

    // Cache the blocks in the file
    fp->n_data_blocks = 0;
    BLOCK_REFERENCE bref = index_node.content;
    BLOCK b;
  
    while(bref != UNALLOCATED_BLOCK) {
      if(debug)
	fprintf(stderr, "Caching %d\n", bref);
      // Log the block
      fp->block_reference_cache[fp->n_data_blocks] = bref;
      // Count the block
      ++fp->n_data_blocks;
      // Read the block
      if(vdisk_read_block(bref, &b) != 0) {
	free(fp);
	return(NULL);
      }
      // Move to the next block
      bref = b.next;
    };
    return(fp);
  }else{
    // A pipe
    // TODO: complete implementation
    return(fp);
  };
};

/**
 *  Close a file or pipe
 *   Cleans up and deallocates the MYFILE structure
 *
 * @param fp Pointer to the MYFILE structure
 */
     
void myfs_fclose(MYFILE *fp) {
  fp->index_node_reference = UNALLOCATED_INDEX_NODE;

  // TODO: complete implementation for the case of pipes

  free(fp);
}



/*
 * Write bytes to an open file/pipe.
 * - Allocate new data blocks, as necessary
 * - Can allocate up to MAX_BLOCKS_IN_FILE, at which point, no more bytes may be written
 * - file offset will always match file size; both will be updated as bytes are written
 *
 * @param fp MYFILE pointer (must be opened for w or a)
 * @param buf Character buffer of bytes to write
 * @param len Number of bytes to write
 * @return The number of written bytes
 *          0 if file is full and no more bytes can be written
 *         -x if an error
 * 
 */
int myfs_fwrite(MYFILE *fp, unsigned char * buf, int len)
{
  if(fp->mode == 'r') {
    fprintf(stderr, "Can't write to read-only file");
    return(0);
  }
  if(debug)
    fprintf(stderr, "-------\nmyfs_fwrite(%d)\n", len);

  if(fp->fd_external == -1) {
    // This is a MYFS file
    
    // Read the file's index node
    INDEX_NODE index_node;
    BLOCK block;
    if(myfs_read_index_node_by_reference(fp->index_node_reference, &index_node) != 0) {
      return(-1);
    }

    // Compute the index for the last block in the file + the first free byte within the block
    int current_blocks = (fp->offset + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE;
    int used_bytes_in_last_block = fp->offset % DATA_BLOCK_SIZE;
    int free_bytes_in_last_block = DATA_BLOCK_SIZE - used_bytes_in_last_block;
    int len_written = 0;

    // Fill in last block (= DATA_BLOCK_SIZE, then we need a new block)
    if(free_bytes_in_last_block < DATA_BLOCK_SIZE) {
      if(debug)
	fprintf(stderr, "Adding to existing block (%d)\n", current_blocks-1);
      // There is space available in the last allocated block
      if(vdisk_read_block(fp->block_reference_cache[fp->n_data_blocks - 1], &block) != 0) {
	return(-2);
      }

      // Number of  bytes to copy
      int len_to_copy = MIN(free_bytes_in_last_block, len);
      // Do the copy
      memcpy(&(block.content.data.data[used_bytes_in_last_block]), buf, len_to_copy);
      // Length remaining
      len = len - len_to_copy;
      // Update size of index_node
      index_node.size += len_to_copy;
      // Update number of bytes written
      len_written += len_to_copy;
      // Update offset
      fp->offset += len_to_copy;
      // Write the block back out
      if(vdisk_write_block(fp->block_reference_cache[fp->n_data_blocks - 1], &block) != 0) {
	return(-3);
      }
    };

    // Allocate new blocks as we need them

    if(len > 0) {
      // We will have to modify the volume block
      BLOCK volume_block;
      if(vdisk_read_block(VOLUME_BLOCK_REFERENCE, &volume_block) != 0) {
	return(-30);
      }
      // We also need the old block (so we can expand the linked list)
      if(fp->n_data_blocks > 0) {
	if(vdisk_read_block(fp->block_reference_cache[fp->n_data_blocks - 1], &block) != 0) {
	  return(-30);
	}
      };

      // Additional blocks
      while(len > 0 && fp->n_data_blocks < MAX_BLOCKS_IN_FILE) {
	if(debug)
	  fprintf(stderr, "Allocating new block\n");
	BLOCK_REFERENCE data_reference = myfs_allocate_new_block(&volume_block);
      
	if(data_reference == UNALLOCATED_BLOCK) {
	  // Out of blocks
	  // Update the index_node
	  if(myfs_write_index_node_by_reference(fp->index_node_reference, &index_node) != 0) {
	    return(-4);
	  };
      
	  // Done: write the volume
	  if(vdisk_write_block(VOLUME_BLOCK_REFERENCE, &volume_block) != 0) {
	    return(-31);
	  }
	  return(len_written);
	}

	// Add the block to the index_node
	fp->block_reference_cache[fp->n_data_blocks] = data_reference;
	if(fp->n_data_blocks == 0) {
	  // This is the first block
	  index_node.content = data_reference;
	}else{
	  // This is not the first block
	  // The last block so far is stored in &block
	  // Previous block must point to the new one
	  block.next = data_reference;
	  if(vdisk_write_block(fp->block_reference_cache[fp->n_data_blocks - 1], &block) != 0) {
	    return(-30);
	  };
	}
	// Increment the block count
	++fp->n_data_blocks;
      
	if(debug) {
	  fprintf(stderr, "Adding new block %d to index_node.data[%d]\n", data_reference,
		  current_blocks);
	}
	// Clean up the new block
	memset(&block, 0, BLOCK_SIZE);
	block.next = UNALLOCATED_BLOCK;
    
	// Number of  bytes to copy
	int len_to_copy = MIN(DATA_BLOCK_SIZE, len);
	// Do the copy
	memcpy(block.content.data.data, &(buf[len_written]), len_to_copy);
	// Length remaining
	len = len - len_to_copy;
	// Update size of index_node
	index_node.size += len_to_copy;
	// Update number of bytes written
	len_written += len_to_copy;
	// Update offset
	fp->offset += len_to_copy;
	// Write the block back out
	if(vdisk_write_block(fp->block_reference_cache[fp->n_data_blocks - 1], &block) != 0) {
	  return(-5);
	}
	// Increment to the next block
	++ current_blocks;
      }

      // Done allocating blocks: update the volume
      if(vdisk_write_block(VOLUME_BLOCK_REFERENCE, &volume_block) != 0) {
	return(-31);
      }
    };

    // Copied all of the bytes
    // Update the index_node
    if(myfs_write_index_node_by_reference(fp->index_node_reference, &index_node) != 0) {
      return(-6);
    };

    // Done
    return(len_written);
  }else{
    // This is a pipe
    // TODO: complete implementation for pipes

    return(len_written);
  }
}

/*
 * Read a sequence of bytes from an open file / pipe.
 * - offset is the current position within the file, and will never be larger than size
 * - offset will be updated with each read operation
 *
 * @param fp MYFILE pointer (must be opened for r)
 * @param buf Character buffer to place the bytes into
 * @param len Number of bytes to read at max
 * @return The number of bytes read
 *         0 if offset is at size
 *         -x if an error
 * 
 */

int myfs_fread(MYFILE *fp, unsigned char * buf, int len)
{
  // Check open mode
  if(fp->mode != 'r') {
    fprintf(stderr, "Can't read from a write-only file");
    return(0);
  }
  if(debug)
    fprintf(stderr, "\n-------\nmyfs_fread(%d)\n", len);

  if(fp->fd_external == -1) {
    // This is a MYFS file
    
    INDEX_NODE index_node;
    BLOCK block;
    if(myfs_read_index_node_by_reference(fp->index_node_reference, &index_node) != 0) {
      return(-1);
    }
      
    // Compute the current block and offset within the block
    int current_block = fp->offset / DATA_BLOCK_SIZE;
    int byte_offset_in_block = fp->offset % DATA_BLOCK_SIZE;
    int len_read = 0;
    int end_of_file = index_node.size;
    len = MIN(len, end_of_file - fp->offset);
    int len_left = len;

    // Loop over blocks to read
    while(len_left > 0 && current_block < MAX_BLOCKS_IN_FILE) {
      // Read the next block
      if(debug) {
	fprintf(stderr, "\tReading block %d data[%d]\n", fp->block_reference_cache[current_block],
		current_block);
      }

      if(vdisk_read_block(fp->block_reference_cache[current_block], &block) != 0) {
	return(-2);
      }

      // Number of  bytes to copy
      int len_to_copy = MIN(DATA_BLOCK_SIZE - byte_offset_in_block, len_left);

      // Do the copy
      memcpy(&(buf[len_read]), &(block.content.data.data[byte_offset_in_block]), len_to_copy);
	   
      // Length remaining
      len_left -= len_to_copy;

      // Update number of bytes written
      len_read += len_to_copy;

      // Update offset
      fp->offset += len_to_copy;

      // Increment to the next block
      ++current_block;

      // For next block, start at zero:
      byte_offset_in_block = 0;
    };

    // Done
    return(len_read);
  }else{
    // This is a pipe
    // TODO: complete implementation for pipes
    return(len_read);
  }
}

/**
 * Delete a file or pipe
 *
 * Full implementation:
 * - Remove the directory entry
 * - Decrement index_node.references
 * - If references == 0, then deallocate the contents and the index_node
 * - TODO: right now, the code assumes that there is only ever one reference
 *
 * @param cwd Absolute path for the current working directory
 * @param path Absolute or relative path of the file to be removed
 * @return 0 if success
 *         -x if error
 *
 */

int myfs_delete_file(char *cwd, char *path)
{
  INDEX_NODE_REFERENCE parent;
  INDEX_NODE_REFERENCE child;
  char local_name[MAX_PATH_LENGTH];
  INDEX_NODE index_node;
  INDEX_NODE index_node_parent;

  // Try to find the index_node of the child
  if(myfs_path_to_index_node(cwd, path, &parent, &child, local_name) < -1) {
    return(-3);
  };
  
  if(child == UNALLOCATED_INDEX_NODE) {
    fprintf(stderr, "File not found\n");
    return(-1);
  }
  // Get the index node
  if(myfs_read_index_node_by_reference(child, &index_node) != 0) {
    return(-4);
  }

  // Is it a file?
  if(!(index_node.type == T_FILE || index_node.type == T_PIPE)) {
    // Not a file
    fprintf(stderr, "Not a file or pipe\n");
    return(-2);
  }

  // Clean up the parent
  if(myfs_read_index_node_by_reference(parent, &index_node_parent) != 0) {
    return(-5);
  }
  index_node_parent.size--;
  if(myfs_write_index_node_by_reference(parent, &index_node_parent) != 0) {
    return(-6);
  }

  // Remove from directory list
  if(myfs_remove_directory_entry(index_node_parent.content, local_name) < 0) {
    return(-8);
  }

  // Commit

  // Doing a complete clean-up
  BLOCK volume_block;
  if(vdisk_read_block(VOLUME_BLOCK_REFERENCE, &volume_block) != 0) {
    return(-11);
  }
  
  // Free the data blocks
  if(index_node.size > 0) {
    if(myfs_deallocate_blocks(&volume_block, index_node.content) != 0) {
      return(-10);
    }
  }

  // One few index nodes
  volume_block.content.volume.n_allocated_index_nodes--;

  // Put the volume block back
  if(vdisk_write_block(VOLUME_BLOCK_REFERENCE, &volume_block) != 0) {
    return(-12);
  }

  // Set the index_node to unused
  index_node.type = T_UNUSED;
  if(myfs_write_index_node_by_reference(child, &index_node) != 0) {
    return(-13);
  }


  // Success
  return(0);
};


/**
 * Create a hard link to a specified file or pipe
 *
 * Full implemenation:
 * - Add the new directory entry
 * - Increment index_node.references
 *
 * @param cwd Absolute path for the current working directory
 * @param path_src Absolute or relative path of the existing file to be linked
 * @param path_dst Absolute or relative path of the new file index_node to be linked
 * @return 0 if success
 *         -x if error
 * 
 */
int myfs_link(char *cwd, char *path_src, char *path_dst)
{
  INDEX_NODE_REFERENCE parent_src;
  INDEX_NODE_REFERENCE child_src;
  INDEX_NODE_REFERENCE parent_dst;
  INDEX_NODE_REFERENCE child_dst;
  char local_name[MAX_PATH_LENGTH];
  char local_name_bogus[MAX_PATH_LENGTH];
  INDEX_NODE index_node_src;
  INDEX_NODE index_node_dst;
  BLOCK volume_block;

  // Try to find the index_nodes
  if(myfs_path_to_index_node(cwd, path_src, &parent_src, &child_src, local_name_bogus) < -1) {
    return(-5);
  }
  if(myfs_path_to_index_node(cwd, path_dst, &parent_dst, &child_dst, local_name) < -1) {
    return(-6);
  }

  // TODO: complete implementation

  // Success
  return(0);
}


/**
 * Move a file from one location to another
 *
 * Full implemenation:
 * - 1. Same parent: just change the name
 * -- Both the src and dest child names must be specified
 * - 2. Different parents:
 * -- The src child name must be specified
 * -- 2a. If the dest child is a directory, then use this as the destination and use the same child name
 * -- 2b. If the dest child does not exist, but the dest parent does, then the parent is the destination
 *        and the dest child name is the name to be used
 * -- Remove directory entry from the src directory & add it to the dest directory
 * -- Decrement size and increment size of the parent index nodes (src and dest, respectively)
 *
 * @param cwd Absolute path for the current working directory
 * @param path_src Absolute or relative path of the existing file to be moved
 * @param path_dst Absolute or relative path of the new file or directory
 * @return 0 if success
 *         -x if error
 *
 */
int myfs_move(char *cwd, char *path_src, char *path_dst)
{
  INDEX_NODE_REFERENCE parent_src;
  INDEX_NODE_REFERENCE child_src;
  INDEX_NODE_REFERENCE parent_dst;
  INDEX_NODE_REFERENCE child_dst;
  char local_name_dst[MAX_PATH_LENGTH];
  char local_name_src[MAX_PATH_LENGTH];
  INDEX_NODE index_node_src;
  INDEX_NODE index_node_dst;
  INDEX_NODE index_node_dst_child;
  BLOCK volume_block;
  BLOCK parent_src_block;
  BLOCK_REFERENCE parent_src_ref;
  int child_src_index;
  BLOCK parent_dst_block;
  BLOCK_REFERENCE parent_dst_ref;
  int child_dst_index;
  char path_dst_copy[MAX_PATH_LENGTH];

  // Save the dest path
  strncpy(path_dst_copy, path_dst, MAX_PATH_LENGTH);

  // Try to find the index_nodes
  if(myfs_path_to_index_node_general(cwd, path_src, &parent_src, &child_src, local_name_src,
				     &parent_src_block, &parent_src_ref, &child_src_index) < -1) {
    return(-5);
  }
  if(myfs_path_to_index_node_general(cwd, path_dst, &parent_dst, &child_dst, local_name_dst,
				     &parent_dst_block, &parent_dst_ref, &child_dst_index) < -1) {
    return(-6);
  }

  // TODO: complete implementation

  // Success
  return(0);
}

/**
 * Create a new pipe in MYFS
 *
 * - It is an error if the parent in MYFS does not exist
 * - It is an error if the parent is not a directory
 * - It is an error if the child in MYFS does exist
 * - It is an error if not enough blocks can be allocated
 * - Allocate a new index node for the pipe (possibly allocating a new block for the index node list)
 * -- type = T_PIPE
 * -- references = 1
 * -- content = as allocated
 * -- size = 0
 *
 * - Allocate a new directory entry in the parent directory (possibly allocating a new block for the
 *        directory list)
 * - Allocate a new data block to store the pipe data in.  Copy the host_path
 *        string into the data part of the block.  next = UNALLOCATED_BLOCK
 *
 * @param cwd Absolute path for the current working directory
 * @param path_host Absolute or relative path of a file in the host file system (Linux)
 * @param path_myfs Absolute or relative path of a file in MYFS
 * @return 0 on success
 *         -x on error
 *
 */

int myfs_mkp(char *cwd, char *path_host, char *path_myfs)
{
  INDEX_NODE_REFERENCE parent;
  INDEX_NODE_REFERENCE child;
  char local_name[MAX_PATH_LENGTH];
  INDEX_NODE index_node_parent;
  int ret;

  // Try to find the index_node of the child
  if((ret = myfs_path_to_index_node(cwd, path_myfs, &parent, &child, local_name)) < -1) {
    if(debug)
      fprintf(stderr, "myfs_mkp(%d)\n", ret);
    return(-10);
  }

  // TODO complete implementation
  
  // All done
  return(0);
}
