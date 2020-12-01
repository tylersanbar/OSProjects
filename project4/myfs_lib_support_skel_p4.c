
/**
   Scan through the index node blocks & try to find a hole

   @param index_node_block (OUT) Block of the linked list that contains the hole
   @param index_block_ref  (OUT) Reference to the block that contains the hole

   @return If there is a hole: the index node reference to that hole
           If there is no hole: UNALLOCATED INDEX NODE
 */
INDEX_NODE_REFERENCE myfs_find_index_node_hole(BLOCK *index_node_block,
					       BLOCK_REFERENCE *index_block_ref)
{
  BLOCK_REFERENCE bref = ROOT_INDEX_NODE_BLOCK;
  INDEX_NODE_REFERENCE ret = 0;

  while(bref != UNALLOCATED_BLOCK) {
    *index_block_ref = bref;
    // Load the block
    if(vdisk_read_block(bref, index_node_block) < 0) {
      // Fatal error
      fprintf(stderr, "myfs_find_index_node_hole: Unable to read block %d\n", bref);
      exit(-1);
    }

    // Scan the block for unused entries
    for(int i = 0; i < N_INDEX_NODES_PER_BLOCK; ++i, ++ret) {
      if(index_node_block->content.index_nodes.index_node[i].type == T_UNUSED) {
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
			     BLOCK *directory_block,
			     BLOCK_REFERENCE *directory_block_ref,
			     int *directory_index)
{
  *directory_block_ref = head_directory_block_ref;
  int last_block_ref = UNALLOCATED_BLOCK;

  while(*directory_block_ref != UNALLOCATED_BLOCK) {
    // Remember last valid block reference
    last_block_ref = *directory_block_ref;

    // Read the block
    if(vdisk_read_block(*directory_block_ref, directory_block) < 0) {
      fprintf(stderr, "Unable to read directory block\n");
      exit(-1);
    }

    // Scan through the directory entries
    for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; ++i) {
      if(directory_block->content.directory.entry[i].index_node_reference == UNALLOCATED_INDEX_NODE) {
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


/*
 * Given a valid directory index_node, return the index_node reference for the sub-item
 * that matches <element_name>
 *
 * @param index_node      (IN) Pointer to a loaded index_node structure.  Must be a directory index_node
 * @param element_name    (IN) Name of the directory element to look up
 * @param block           (OUT) Directory block that contains the entity
 * @param ref             (OUT) Index into the directory block for the entity
 *
 * @return = INDEX_NODE_REFERENCE for the sub-item if found; UNALLOCATED_INDEX_NODE if not found
 *
 * PROVIDED
 */

INDEX_NODE_REFERENCE myfs_find_entity_in_directory(INDEX_NODE *index_node, char *element_name,
						   BLOCK *block, BLOCK_REFERENCE *ref,
						   int *entry_index)
{
  if(debug)
    fprintf(stderr,"\tDEBUG: myfs_find_entity_in_directory: %s\n", element_name);

  *ref = index_node->content;

  // Loop over all directory blocks in the linked list
  while(*ref != UNALLOCATED_BLOCK) {
    if(debug)
      fprintf(stderr,"\tDEBUG: myfs_find_entity_in_directory() check block: %d\n", *ref);

    // Read the directory data block
    if(vdisk_read_block(*ref, block) != 0) {
      return(UNALLOCATED_INDEX_NODE);
    }
  
    // Loop over all entries
    for(*entry_index = 0; *entry_index < N_DIRECTORY_ENTRIES_PER_BLOCK; ++*entry_index) {
      if(debug)
	fprintf(stderr, "\tDEBUG: Check: %d %s with %s\n", *entry_index,
		block->content.directory.entry[*entry_index].name, element_name);
      if(block->content.directory.entry[*entry_index].index_node_reference != UNALLOCATED_INDEX_NODE &&
	 strncmp(block->content.directory.entry[*entry_index].name, element_name, FILE_NAME_SIZE) == 0) {
	// Match: we are done
	if(debug)
	  fprintf(stderr, "\tDEBUG: Match: %d\n",
		  block->content.directory.entry[*entry_index].index_node_reference);
	return(block->content.directory.entry[*entry_index].index_node_reference);
      }
    }
    // Go to the next directory block
    *ref = block->next;
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
 *             (i.e., name relative to the parent); must be MAX_PATH_LENGTH in size.
 *             If this pointer is NULL, then the name is not copied to this string.  
 * @return 0 if no errors
 *         -1 if child not found
 *         -x if an error
 *
 */
int myfs_path_to_index_node(char *cwd, char * path, INDEX_NODE_REFERENCE *parent,
			    INDEX_NODE_REFERENCE *child,
			    char *local_name)
{
  BLOCK b;
  int i;
  BLOCK_REFERENCE r;
  return(myfs_path_to_index_node_general(cwd, path, parent, child, local_name, &b, &r, &i));
}

/**
 * Base implementation of path_to_index_node(), with extra output parameters.  Useful for myfs_move()
 *
 * @param cwd (IN) Absolute path for the current working directory
 * @param path (IN) Absolute or relative path of the file/directory to be found
 * @param parent (OUT) Pointer to the found index node reference for the parent directory
 * @param child (OUT) Pointer to the found node reference for the file or directory specified by path
 * @param local_name (OUT) String name of the file or directory without any path information
 *             (i.e., name relative to the parent); must be MAX_PATH_LENGTH in size.
 *             If this pointer is NULL, then the name is not copied to this string.
 * @param directory_block (OUT) Pointer to a directory block; after call contains the parent directory
 *             block that contains the child
 * @param directory_block_ref (OUT) Pointer to a block reference; after call contains the reference to
 *             the directory_block
 * @param entity_index (OUT) Pointer to an integer; after call contains the index in the directory block
 *             of the child (counting from 0)
 * @return 0 if no errors
 *         -1 if child not found
 *         -x if an error
 *
 */
int myfs_path_to_index_node_general(char *cwd, char * path, INDEX_NODE_REFERENCE *parent,
				    INDEX_NODE_REFERENCE *child,
				    char *local_name,
				    BLOCK *directory_block,
				    BLOCK_REFERENCE *directory_block_ref,
				    int *entity_index)
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
      INDEX_NODE_REFERENCE new_index_node = myfs_find_entity_in_directory(&index_node, directory_name,
									  directory_block,
									  directory_block_ref,
									  entity_index);
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



/************************************************************************/
// Project 4

/**
 *  Create a zero-length file within a specified directory
 *
 *  @param volume_block (IN/OUT) Already loaded volume block
 *  @param parent (IN) Index_Node reference for the parent directory
 *  @param local_name (IN) Name of the file within the parent directory
 *  @return Index_Node reference index for the newly created file
 *          UNALLOCATED_INDEX_NODE if an error
 *
 *  Errors include: virtual disk read/write errors, no available index_nodes
 *
 *  Note: the modified volume block is written to the virtual disk
 *  
 */
INDEX_NODE_REFERENCE myfs_create_file(BLOCK *volume_block,
				      INDEX_NODE_REFERENCE parent, char *local_name)
{
  // Does the parent have a slot?
  INDEX_NODE index_node;

  // Read the parent index_node
  if(myfs_read_index_node_by_reference(parent, &index_node) != 0) {
    return UNALLOCATED_INDEX_NODE;
  }


  // How many new blocks do we need? (0,1,2)
  int n_new_blocks = 0;   // Case 3 (no new block for data storage)
  BLOCK directory_block;
  BLOCK_REFERENCE directory_block_ref;
  int directory_index;
  int has_directory_hole = myfs_find_directory_hole(index_node.content, &directory_block,
							&directory_block_ref,
							&directory_index);
  if(!has_directory_hole) {
    ++n_new_blocks;
  }

  BLOCK index_node_block;
  BLOCK_REFERENCE index_block_ref;
  INDEX_NODE_REFERENCE index_node_hole = myfs_find_index_node_hole(&index_node_block, &index_block_ref);
  if(index_node_hole == UNALLOCATED_INDEX_NODE) {
    ++n_new_blocks;
  }

  // Do we have enough free blocks to implement this operation?
  if(volume_block->content.volume.n_allocated_blocks + n_new_blocks >
     volume_block->content.volume.n_blocks) {
    // Not enough blocks to allocate
    fprintf(stderr, "File system full\n");
    return UNALLOCATED_INDEX_NODE;
  }

  // Now allocate everything

  // Index node block: case 1B
  if(index_node_hole == UNALLOCATED_INDEX_NODE) {
    if(debug)
      fprintf(stderr, "ADDING INODE BLOCK\n");
    if(myfs_append_new_block_to_existing_block(volume_block, &index_node_block, &index_block_ref) < 0)
      return UNALLOCATED_INDEX_NODE;
    if(debug)
      fprintf(stderr, "ADDING INODE BLOCK (%d)\n", index_block_ref);
    myfs_clear_all_index_node_entries(&index_node_block);

    // Write out the index node block
    if(vdisk_write_block(index_block_ref, &index_node_block) < 0)
      return UNALLOCATED_INDEX_NODE;

    // Recompute index node hole
    index_node_hole = myfs_find_index_node_hole(&index_node_block, &index_block_ref);
  }

  // Extra directory block: case 2A
  if(!has_directory_hole) {
    if(debug)
      fprintf(stderr, "ADDING DIRECTORY BLOCK\n");
    if(myfs_append_new_block_to_existing_block(volume_block, &directory_block, &directory_block_ref) < 0)
      return UNALLOCATED_INDEX_NODE;
    if(debug)
      fprintf(stderr, "ADDING DIRECTORY BLOCK (%d)\n", directory_block_ref);

    // Initialize the contents of the new directory block
    myfs_clear_all_directory_entries(&directory_block);
    // This is a new block, so we will use the first entry
    directory_index = 0;
  }


  // Update the parent inode
  index_node.size++;
  if(myfs_write_index_node_by_reference(parent, &index_node) < 0) {
    fprintf(stderr, "New directory: parent inode write error\n");
    return UNALLOCATED_INDEX_NODE;
  }

  // Update the parent directory entry
  directory_block.content.directory.entry[directory_index].index_node_reference = index_node_hole;
  strcpy(directory_block.content.directory.entry[directory_index].name, local_name);
  if(vdisk_write_block(directory_block_ref, &directory_block) < 0) {
    fprintf(stderr, "New directory entry write parent\n");
    return UNALLOCATED_INDEX_NODE;
  }

  // Update the root block
  volume_block->content.volume.n_allocated_index_nodes++;
  if(vdisk_write_block(VOLUME_BLOCK_REFERENCE, &volume_block) < 0) {
    fprintf(stderr, "Volume block write error\n");
    return UNALLOCATED_INDEX_NODE;
  }


  // Create the new inode
  INDEX_NODE new_index_node;
  myfs_set_index_node(&new_index_node, T_FILE, 1, UNALLOCATED_BLOCK, 0);

  if(debug)
    fprintf(stderr, "Creating new index node: %d\n", index_node_hole);

  // Write the inode out to disk
  if(myfs_write_index_node_by_reference(index_node_hole, &new_index_node) < 0) {
    return UNALLOCATED_INDEX_NODE;
  }


  // Success
  return(index_node_hole);
}

