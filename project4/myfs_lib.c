#include "myfs_lib.h"
#include "myfs_lib_support.h"
#include "vdisk.h"
#include "string.h"

// Yes ... a global variable
int debug = 0;

/**
 Read the MYFS_PWD, MYFS_DISK environment
 variables copy their values into cwd, disk_name an pipe_name_base.  If these
 environment variables are not set, then reasonable defaults are
 given.

 @param cwd String buffer in which to place the MYFS current working directory.
 @param disk_name String buffer in which to place file name of the virtual disk.

 PROVIDED
 */
void myfs_get_environment(char* cwd, char* disk_name)
{
	// Current working directory for the OUFS
	char* str = getenv("MYFS_CWD");
	if (str == NULL) {
		// Provide default
		strcpy(cwd, "/");
	}
	else {
		// Exists
		strncpy(cwd, str, MAX_PATH_LENGTH - 1);
		cwd[MAX_PATH_LENGTH - 1] = 0;
	}

	// Virtual disk location
	str = getenv("MYFS_DISK");
	if (str == NULL) {
		// Default
		strcpy(disk_name, "vdisk");
	}
	else {
		// Exists: copy
		strncpy(disk_name, str, MAX_PATH_LENGTH - 1);
		disk_name[MAX_PATH_LENGTH - 1] = 0;
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
	myfs_init_directory_block_and_index_node(&dirNode, &block, ROOT_DIRECTORY_BLOCK, 0, 0);
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
			fprintf(stderr, "Not Found\n");
			return (-1);
		}
		fprintf(stderr, "Error finding path to file or directory.\n");
		return(-3);
	}
	if (debug) {
		fprintf(stderr, "Entry found at index node %d, with name %s.\n", (int)new_Ref, local_name);
	}
	//Check if index node is a directory, if not, print name
	if (myfs_read_index_node_by_reference(new_Ref, &new) != 0) {
		fprintf(stderr, "Unable to read index node.\n");
	}
	if (new.type != T_DIRECTORY) {
		printf("%s\n", local_name);
		return(0);
	}
	//Make array of strings with max number of directory entries
	char contents[MAX_ENTRIES_PER_DIRECTORY][MAX_PATH_LENGTH];
	//Next concents index
	int numOfElements = 0;
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
		for (int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++) {
			//Get reference for index node
			new_Ref = directoryBlock.content.directory.entry[i].index_node_reference;
			//If entry is valid, add to list
			if (new_Ref != UNALLOCATED_INDEX_NODE) {
				if (debug) {
					fprintf(stderr, "Found entry at %d.\n", (int)new_Ref);
				}
				//Get name of entry
				strncpy(local_name, directoryBlock.content.directory.entry[i].name, MAX_PATH_LENGTH);
				//Get entry index node to check if it is a directory
				myfs_read_index_node_by_reference(new_Ref, &new);
				//If it is, add "/"
				if (new.type == T_DIRECTORY) strcat(local_name, "/");
				//Add name to list of contents
				strcpy(contents[numOfElements++], local_name);
				//If we've found all the entires, exit loop
				if (numOfElements == new.size) {
					bRef = UNALLOCATED_BLOCK;
					continue;
				}
			}
		}

	}
	//Sort list of names
	qsort(contents, numOfElements, MAX_PATH_LENGTH, cmpstr);
	//Print the sorted list of names
	for (int i = 0; i < numOfElements; i++) {
		printf("%s\n", contents[i]);
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
int myfs_mkd(char* cwd, char* path)
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
int myfs_rmd(char* cwd, char* path)
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
	if (myfs_path_to_index_node(cwd, path, &parentIndexRef, &removeIndexRef, local_name) == -1) {
		fprintf(stderr, "Directory does not exist.\n");
		return (-1);
	}
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
		fprintf(stderr, "Directory not empty\n");
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
	if (myfs_read_index_node_by_reference(parentIndexRef, &parentIndexNode) != 0) {
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

MYFILE* myfs_fopen(char* cwd, char* path, char* mode)
{
	INDEX_NODE_REFERENCE parent;
	INDEX_NODE_REFERENCE child;
	char local_name[MAX_PATH_LENGTH];
	INDEX_NODE index_node;
	int ret;

	// Check for valid mode
	if (mode[0] != 'r' && mode[0] != 'w' && mode[0] != 'a') {
		fprintf(stderr, "fopen(): bad mode.\n");
		return(NULL);
	};

	// Try to find the index_node of the child
	if ((ret = myfs_path_to_index_node(cwd, path, &parent, &child, local_name)) < -1) {
		if (debug)
			fprintf(stderr, "myfs_fopen(%d)\n", ret);
		return(NULL);
	}

	if (parent == UNALLOCATED_INDEX_NODE) {
		fprintf(stderr, "Parent directory not found.\n");
		return(NULL);
	}

	// Fetch the volume block
	BLOCK volume_block;
	if (vdisk_read_block(VOLUME_BLOCK_REFERENCE, &volume_block) < 0)
		return(NULL);

	// Now try to open
	if (child == UNALLOCATED_INDEX_NODE && mode[0] == 'r') {
		// Read a non-existent file
		fprintf(stderr, "File not found.\n");
		return(NULL);
	}
	else if (child == UNALLOCATED_INDEX_NODE) {
		// Write or append a non-existent file

		// Open non-existent file for writing
		child = myfs_create_file(&volume_block, parent, local_name);
		if (child == UNALLOCATED_INDEX_NODE) {
			// Error creating new file
			return(NULL);
		};

		// Write the changes back to the volume block
		if (vdisk_write_block(VOLUME_BLOCK_REFERENCE, &volume_block) < 0) {
			// Error
			return(NULL);
		}

		// Get the index_node for the child
		if (myfs_read_index_node_by_reference(child, &index_node) != 0) {
			return(NULL);
		};

		if (index_node.type != T_FILE) {
			// not a file: this should not happen
			fprintf(stderr, "Not a file.\n");
			return(NULL);
		}
	}
	else {
		// File exists
		// Get the index_node
		if (myfs_read_index_node_by_reference(child, &index_node) != 0) {
			return(NULL);
		}
		if (index_node.type == T_DIRECTORY) {
			// not a file
			fprintf(stderr, "Not a file.\n");
			return(NULL);
		}
	}

	// Child has a valid index_node value and we have loaded the index_node
	if (debug)
		fprintf(stderr, "Now opening file...\n");

	// Create the file structure
	MYFILE* fp = malloc(sizeof(MYFILE));
	fp->index_node_reference = child;
	fp->mode = mode[0];

	// Opening is different for files and pipes
	if (index_node.type == T_FILE) {
		// Not used
		fp->fd_external = -1;

		// What mode are we opening for?
		if (fp->mode == 'a') {
			// Open for append
			fp->offset = index_node.size;
		}
		else {
			// Open for writing or reading
			fp->offset = 0;
			if (fp->mode == 'w' && index_node.size > 0) {
				if (debug)
					fprintf(stderr, "Deallocating pre-existing blocks\n");
				index_node.size = 0;
				// Deallocate blocks owned by index_node
				myfs_deallocate_blocks(&volume_block, index_node.content);
				// Now set to no content
				index_node.content = UNALLOCATED_BLOCK;

				if (vdisk_write_block(VOLUME_BLOCK_REFERENCE, &volume_block) < 0) {
					// Error
					free(fp);
					return(NULL);
				}

				// Update the index_node on disk
				if (myfs_write_index_node_by_reference(child, &index_node) != 0) {
					// Error
					free(fp);
					return(NULL);
				}
			}
		}

		if (debug)
			fprintf(stderr, "Now caching data block refs\n");

		// Cache the blocks in the file
		fp->n_data_blocks = 0;
		BLOCK_REFERENCE bref = index_node.content;
		BLOCK b;

		while (bref != UNALLOCATED_BLOCK) {
			if (debug)
				fprintf(stderr, "Caching %d\n", bref);
			// Log the block
			fp->block_reference_cache[fp->n_data_blocks] = bref;
			// Count the block
			++fp->n_data_blocks;
			// Read the block
			if (vdisk_read_block(bref, &b) != 0) {
				free(fp);
				return(NULL);
			}
			// Move to the next block
			bref = b.next;
		};
		return(fp);
	}
	else {
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

void myfs_fclose(MYFILE* fp) {
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
int myfs_fwrite(MYFILE* fp, unsigned char* buf, int len)
{
	if (fp->mode == 'r') {
		fprintf(stderr, "Can't write to read-only file");
		return(0);
	}
	if (debug)
		fprintf(stderr, "-------\nmyfs_fwrite(%d)\n", len);

	if (fp->fd_external == -1) {
		// This is a MYFS file

		// Read the file's index node
		INDEX_NODE index_node;
		BLOCK block;
		if (myfs_read_index_node_by_reference(fp->index_node_reference, &index_node) != 0) {
			return(-1);
		}

		// Compute the index for the last block in the file + the first free byte within the block
		int current_blocks = (fp->offset + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE;
		int used_bytes_in_last_block = fp->offset % DATA_BLOCK_SIZE;
		int free_bytes_in_last_block = DATA_BLOCK_SIZE - used_bytes_in_last_block;
		int len_written = 0;

		// Fill in last block (= DATA_BLOCK_SIZE, then we need a new block)
		if (free_bytes_in_last_block < DATA_BLOCK_SIZE) {
			if (debug)
				fprintf(stderr, "Adding to existing block (%d)\n", current_blocks - 1);
			// There is space available in the last allocated block
			if (vdisk_read_block(fp->block_reference_cache[fp->n_data_blocks - 1], &block) != 0) {
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
			if (vdisk_write_block(fp->block_reference_cache[fp->n_data_blocks - 1], &block) != 0) {
				return(-3);
			}
		};

		// Allocate new blocks as we need them

		if (len > 0) {
			// We will have to modify the volume block
			BLOCK volume_block;
			if (vdisk_read_block(VOLUME_BLOCK_REFERENCE, &volume_block) != 0) {
				return(-30);
			}
			// We also need the old block (so we can expand the linked list)
			if (fp->n_data_blocks > 0) {
				if (vdisk_read_block(fp->block_reference_cache[fp->n_data_blocks - 1], &block) != 0) {
					return(-30);
				}
			};

			// Additional blocks
			while (len > 0 && fp->n_data_blocks < MAX_BLOCKS_IN_FILE) {
				if (debug)
					fprintf(stderr, "Allocating new block\n");
				BLOCK_REFERENCE data_reference = myfs_allocate_new_block(&volume_block);

				if (data_reference == UNALLOCATED_BLOCK) {
					// Out of blocks
					// Update the index_node
					if (myfs_write_index_node_by_reference(fp->index_node_reference, &index_node) != 0) {
						return(-4);
					};

					// Done: write the volume
					if (vdisk_write_block(VOLUME_BLOCK_REFERENCE, &volume_block) != 0) {
						return(-31);
					}
					return(len_written);
				}

				// Add the block to the index_node
				fp->block_reference_cache[fp->n_data_blocks] = data_reference;
				if (fp->n_data_blocks == 0) {
					// This is the first block
					index_node.content = data_reference;
				}
				else {
					// This is not the first block
					// The last block so far is stored in &block
					// Previous block must point to the new one
					block.next = data_reference;
					if (vdisk_write_block(fp->block_reference_cache[fp->n_data_blocks - 1], &block) != 0) {
						return(-30);
					};
				}
				// Increment the block count
				++fp->n_data_blocks;

				if (debug) {
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
				if (vdisk_write_block(fp->block_reference_cache[fp->n_data_blocks - 1], &block) != 0) {
					return(-5);
				}
				// Increment to the next block
				++current_blocks;
			}

			// Done allocating blocks: update the volume
			if (vdisk_write_block(VOLUME_BLOCK_REFERENCE, &volume_block) != 0) {
				return(-31);
			}
		};

		// Copied all of the bytes
		// Update the index_node
		if (myfs_write_index_node_by_reference(fp->index_node_reference, &index_node) != 0) {
			return(-6);
		};

		// Done
		return(len_written);
	}
	else {
		// This is a pipe
		// TODO: complete implementation for pipes
		int len_written = 0;
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

int myfs_fread(MYFILE* fp, unsigned char* buf, int len)
{
	// Check open mode
	if (fp->mode != 'r') {
		fprintf(stderr, "Can't read from a write-only file");
		return(0);
	}
	if (debug)
		fprintf(stderr, "\n-------\nmyfs_fread(%d)\n", len);

	if (fp->fd_external == -1) {
		// This is a MYFS file

		INDEX_NODE index_node;
		BLOCK block;
		if (myfs_read_index_node_by_reference(fp->index_node_reference, &index_node) != 0) {
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
		while (len_left > 0 && current_block < MAX_BLOCKS_IN_FILE) {
			// Read the next block
			if (debug) {
				fprintf(stderr, "\tReading block %d data[%d]\n", fp->block_reference_cache[current_block],
					current_block);
			}

			if (vdisk_read_block(fp->block_reference_cache[current_block], &block) != 0) {
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
	}
	else {
		// This is a pipe
		// TODO: complete implementation for pipes
		int len_read = 0;
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

int myfs_delete_file(char* cwd, char* path)
{
	INDEX_NODE_REFERENCE parent;
	INDEX_NODE_REFERENCE child;
	char local_name[MAX_PATH_LENGTH];
	INDEX_NODE index_node;
	INDEX_NODE index_node_parent;

	// Try to find the index_node of the child
	if (myfs_path_to_index_node(cwd, path, &parent, &child, local_name) < -1) {
		return(-3);
	};

	if (child == UNALLOCATED_INDEX_NODE) {
		fprintf(stderr, "File not found\n");
		return(-1);
	}
	// Get the index node
	if (myfs_read_index_node_by_reference(child, &index_node) != 0) {
		return(-4);
	}

	// Is it a file?
	if (!(index_node.type == T_FILE || index_node.type == T_PIPE)) {
		// Not a file
		fprintf(stderr, "Not a file or pipe\n");
		return(-2);
	}

	// Clean up the parent
	if (myfs_read_index_node_by_reference(parent, &index_node_parent) != 0) {
		return(-5);
	}
	index_node_parent.size--;
	if (myfs_write_index_node_by_reference(parent, &index_node_parent) != 0) {
		return(-6);
	}

	// Remove from directory list
	if (myfs_remove_directory_entry(index_node_parent.content, local_name) < 0) {
		return(-8);
	}

	// Commit

	// Doing a complete clean-up
	BLOCK volume_block;
	if (vdisk_read_block(VOLUME_BLOCK_REFERENCE, &volume_block) != 0) {
		return(-11);
	}

	// Free the data blocks
	if (index_node.size > 0) {
		if (myfs_deallocate_blocks(&volume_block, index_node.content) != 0) {
			return(-10);
		}
	}

	// One few index nodes
	volume_block.content.volume.n_allocated_index_nodes--;

	// Put the volume block back
	if (vdisk_write_block(VOLUME_BLOCK_REFERENCE, &volume_block) != 0) {
		return(-12);
	}

	// Set the index_node to unused
	index_node.type = T_UNUSED;
	if (myfs_write_index_node_by_reference(child, &index_node) != 0) {
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
int myfs_link(char* cwd, char* path_src, char* path_dst)
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
	if (myfs_path_to_index_node(cwd, path_src, &parent_src, &child_src, local_name_bogus) < -1) {
		return(-5);
	}
	if (myfs_path_to_index_node(cwd, path_dst, &parent_dst, &child_dst, local_name) < -1) {
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
int myfs_move(char* cwd, char* path_src, char* path_dst)
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
	if (myfs_path_to_index_node_general(cwd, path_src, &parent_src, &child_src, local_name_src,
		&parent_src_block, &parent_src_ref, &child_src_index) < -1) {
		return(-5);
	}
	if (myfs_path_to_index_node_general(cwd, path_dst, &parent_dst, &child_dst, local_name_dst,
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

int myfs_mkp(char* cwd, char* path_host, char* path_myfs)
{
	INDEX_NODE_REFERENCE parent;
	INDEX_NODE_REFERENCE child;
	char local_name[MAX_PATH_LENGTH];
	INDEX_NODE index_node_parent;
	int ret;

	// Try to find the index_node of the child
	if ((ret = myfs_path_to_index_node(cwd, path_myfs, &parent, &child, local_name)) < -1) {
		if (debug)
			fprintf(stderr, "myfs_mkp(%d)\n", ret);
		return(-10);
	}

	// TODO complete implementation

	// All done
	return(0);
}
