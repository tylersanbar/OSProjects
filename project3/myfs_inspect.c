/**
 *  Project 3
 *
 *  myfs_inspect
 *
 * Author: Andrew H. Fagg (CS3113)
 *
 *  Allows the user to view raw block and index_node-level information. 
 *  This is not intended as a standard user executable.  Instead, it
 *  is intended to aid in debugging and testing.
 *
 *  Usage: myfs_inspect -help
 *     (for full set of commands)
 *
 *  Author: CS3113
 *
 */

#include <stdio.h>
#include <string.h>

#include "myfs_lib_support.h"

#include "vdisk.h"

/**
   Print the type of an index node given INDEX_NODE_TYPE

   @param type Type of the index node
 */
void print_type(INDEX_NODE_TYPE type)
{
  switch(type)
    {
    case T_UNUSED:
      printf("UNUSED");
      break;
    case T_DIRECTORY:
      printf("DIRECTORY");
      break;
    case T_FILE:
      printf("FILE");
      break;
    case T_PIPE:
      printf("PIPE");
      break;
    }
}

int main(int argc, char** argv) {
  // Get the key environment variables
  char cwd[MAX_PATH_LENGTH];
  char disk_name[MAX_PATH_LENGTH];

  // Get the environment variable information
  myfs_get_environment(cwd, disk_name);

  // Connect to the virtual disk
  if(vdisk_open(disk_name, 0) != 0) {
    return(-1);
  }

  // Respond to the different options
  if(argc == 1){
    // Zero arguments
    printf("Usage: myfs_inspect -help\n");


    // 2-argument commands
  }else if(argc == 2){
    if(strncmp(argv[1], "-volume", 8) == 0) {
      // Display the volume record
      BLOCK block;
      if(vdisk_read_block(VOLUME_BLOCK_REFERENCE, &block) != 0) {
	fprintf(stderr, "Error reading volume block\n");
      }else{
	// Block read: report state
	printf("N_BLOCKS: %d\n", block.content.volume.n_blocks);
	printf("N_ALLOCATED_BLOCKS: %d\n", block.content.volume.n_allocated_blocks);
	printf("N_ALLOCATED_INDEX_NODES: %d\n", block.content.volume.n_allocated_index_nodes);
	// Print the block allocation table up to the byte that contains the bit for the last block
	printf("Block allocation table:\n");
	for(int i = 0; i < (block.content.volume.n_blocks+7) >> 3; ++i) {
	  printf("%02d: %02x\n", i, block.content.volume.block_allocation_table[i]);
	}
      }

    }else if(strncmp(argv[1], "-help", 6) == 0) {
      // User is asking for help
      printf("Usage:\n");
      printf("myfs_inspect -volume\t\t Show the volume block\n");
      printf("myfs_inspect -help\t\t Print this help\n");
      printf("myfs_inspect -index <#>\t\t Print contents of INDEX_NODE #\n");
      printf("myfs_inspect -iblock <#>\t\t Print index node contents of block #\n");
      printf("myfs_inspect -dir <#>\t Print the contents of directory block #\n");
      printf("myfs_inspect -block <#>\t\t Print the top-level block data for block #\n");
      printf("myfs_inspect -data <#>\t\t Print the raw data contents for the block (including printable characters)\n");
    }else{
      fprintf(stderr, "Unknown argument (%s)\n", argv[1]);
    }

  }

  // 3-argument commands
  else if(argc == 3) {
    if(strncmp(argv[1], "-index", 7) == 0) {
      // Index node query
      int index;
      if(sscanf(argv[2], "%d", &index) == 1){
	if(index < 0) {
	  fprintf(stderr, "Index_Node index out of range (%s)\n", argv[2]);
	}else{
	  // Print out the specified index node data
	  INDEX_NODE index_node;
	  int ret = myfs_read_index_node_by_reference(index, &index_node);
	  if(ret < 0) {
	    fprintf(stderr, "Index node does not exist (%d)\n", index);
	    exit(-1);
	  }
	  
	  printf("Index node: %d\n", index);
	  printf("Type: ");
	  print_type(index_node.type);
 
	  printf("\nNreferences: %d\n", index_node.references);
	  printf("Content block: %d\n", index_node.content);
	  printf("Size: %d\n", index_node.size);
	}
      }else{
	fprintf(stderr, "Unknown argument (-index %s)\n", argv[2]);
      }

      // Index node block
    }else if(strncmp(argv[1], "-iblock", 8) == 0) {
      int block_ref;
      BLOCK block;

      if(sscanf(argv[2], "%d", &block_ref) == 1){
	if(block_ref < 0) {
	  fprintf(stderr, "Block ref out of range (%s)\n", argv[2]);
	}else{
	  if(vdisk_read_block(block_ref, &block) < 0) {
	    fprintf(stderr, "Error reading block %d\n", block_ref);
	  }else{
	    // Loop over all index node entries
	    for(int i=0; i < N_INDEX_NODES_PER_BLOCK; ++i) {
	      if(block.content.index_nodes.index_node[i].type != T_UNUSED) {
		// Index node is valid, print its details
		printf("Relative Index Node %d\t", i);
		print_type(block.content.index_nodes.index_node[i].type);
		printf("\tNref=%d\tContent=%d\tsize=%d\n",
		      block.content.index_nodes.index_node[i].references,
		      block.content.index_nodes.index_node[i].content,
		      block.content.index_nodes.index_node[i].size);
	      }
	    }
	    // Print the reference to the next block in the list
	    printf("Next block: %d\n", block.next);
	  }
	}
      }else{
	fprintf(stderr, "Unknown argument (%s)\n", argv[1]);
      }

      // Directory node block
    }else if(strncmp(argv[1], "-dir", 8) == 0) {
      // Inspect directory block
      int index;

      // Parse parameter
      if(sscanf(argv[2], "%d", &index) == 1){
	if(index < 0 || index >= MAX_BLOCKS) {
	  fprintf(stderr, "Block index out of range (%s)\n", argv[2]);
	}else{
	  // success
	  BLOCK block;
	  // Read the block
	  if(vdisk_read_block(index, &block) < 0) {
	    fprintf(stderr, "Error reading block %d\n", index);
	    exit(-1);
	  }

	  // display block data
	  printf("Directory at block %d:\n", index);

	  // Loop over all directory entries
	  for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; ++i) {
	    // Is it valid?
	    if(block.content.directory.entry[i].index_node_reference != UNALLOCATED_INDEX_NODE) {
	      // Yes: print the entry
	      printf("Entry %d: name=\"%s\", index_node=%d\n", i,
		     block.content.directory.entry[i].name,
		     block.content.directory.entry[i].index_node_reference);
	    }
	  }
	  // What is the reference to the next block?
	  printf("Next block: %d\n", block.next);
	}
      }

      // Generic block data
    }else if(strncmp(argv[1], "-block", 7) == 0) {
      // Inspect high-level block
      int index;

      // Parse the one argument
      if(sscanf(argv[2], "%d", &index) == 1){
	if(index < 0 || index >= MAX_BLOCKS) {
	  fprintf(stderr, "Block index out of range (%s)\n", argv[2]);
	}else{
	  // Success
	  BLOCK block;
	  if(vdisk_read_block(index, &block) < 0) {
	    fprintf(stderr, "Cannot read block %d\n", index);
	    exit(-1);
	  }
	  printf("Block %d:\n", index);
	  printf("Next block: %d\n", block.next);
	}
      }

      // Print the raw data associated with a block
    }else if(strncmp(argv[1], "-data", 6) == 0) {
      // Inspect raw block
      int index;

      // Parse the argument
      if(sscanf(argv[2], "%d", &index) == 1){
	if(index < 0 || index >= MAX_BLOCKS) {
	  fprintf(stderr, "Block index out of range (%s)\n", argv[2]);
	}else{
	  // Success
	  BLOCK block;

	  // Get the spcified block
	  if(vdisk_read_block(index, &block) < 0) {
	    fprintf(stderr, "Cannot read block %d\n", index);
	    exit(-1);
	  }
	  printf("Raw data at block %d:\n", index);
	  // Loop over all bytes in the data portion of the block
	  for(int i = 0; i < DATA_BLOCK_SIZE; ++i) {
	    if(block.content.data.data[i] >= ' ' && block.content.data.data[i] <= '~')
	      // Byte is printable
	      printf("%3d: %02x %c\n", i, block.content.data.data[i],
		     block.content.data.data[i]);
	    else
	      // Byte is not printable
	      printf("%3d: %02x\n", i, block.content.data.data[i]);
	  }
	  printf("Next block: %d\n", block.next);
	}
      }
    }

 }

  // All done: detach from the disk
  vdisk_close();
}

