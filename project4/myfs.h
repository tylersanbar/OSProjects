/*******
 * Low-level file system definitions
 *
 * DO NOT CHANGE THIS DATA STRUCTURE
 *
 * Author: Andrew H. Fagg (CS 3113)
 *
*
 */

 // Only evaluate these definitions once, even if included multiple times
#ifndef FILE_STRUCTS_H
#define FILE_STRUCTS_H

#include <string.h>
#include <limits.h>
#include "vdisk.h"


// Implementation of min operator
#define MIN(a, b) (((a) > (b)) ? (b) : (a))


/**********************************************************************/
/*
File system layout onto disk blocks:

Block 0: Volume block
Block 1: First Index Node block
Block 2: Root directory block
  :
  :
(all other blocks are either index node, directory or data blocks)
*/


/**********************************************************************/
// Basic types and sizes
// Chosen carefully so that all block types pack nicely into a full block
//
// NOTE: USE THESE CONSTANTS INSTEAD OF HARD-CODING VALUES IN YOUR CODE

// An index for a block (0, 1, 2, ...)
typedef unsigned short BLOCK_REFERENCE;

// Value used as an index when it does not refer to a block
#define UNALLOCATED_BLOCK (USHRT_MAX-1)

// An index that refers to an index node
typedef unsigned short INDEX_NODE_REFERENCE;

// Value used as an index when it does not refer to an index node
#define UNALLOCATED_INDEX_NODE (USHRT_MAX)

// Number of bytes available for block data
#define DATA_BLOCK_SIZE ((int)(BLOCK_SIZE-sizeof(int)))

// The block on the virtual disk containing the root directory
#define ROOT_DIRECTORY_BLOCK 2

// The block on the virtual disk containing the first index nodes
#define ROOT_INDEX_NODE_BLOCK 1

// The Index node for the root directory
#define ROOT_DIRECTORY_INDEX_NODE 0

// Size of file/directory name
#define FILE_NAME_SIZE ((int)(32 - sizeof(INDEX_NODE_REFERENCE)))

/**********************************************************************/
// Data block: storage for file contents (project 4!)
typedef struct
{
	unsigned char data[DATA_BLOCK_SIZE];
} DATA_BLOCK;


/**********************************************************************/
// Index node types
typedef enum { T_UNUSED = 0, T_DIRECTORY, T_FILE, T_PIPE } INDEX_NODE_TYPE;

// Single index node
typedef struct
{
	// Type of INDEX_NODE
	INDEX_NODE_TYPE type;

	// Number of directory references to this index node
	unsigned char references;

	// Contents.  UNALLOCATED_BLOCK means that this entry is not used
	BLOCK_REFERENCE content;

	// File: size in bytes; Directory: number of directory entries
	//  (including . and ..)
	unsigned int size;
} INDEX_NODE;

// Number of index nodes stored in each block
#define N_INDEX_NODES_PER_BLOCK ((int)(DATA_BLOCK_SIZE/sizeof(INDEX_NODE)))

// Block of index_nodes
typedef struct
{
	INDEX_NODE index_node[N_INDEX_NODES_PER_BLOCK];
} INDEX_NODE_BLOCK;


/**********************************************************************/
// Block 0: Volume block
#define VOLUME_BLOCK_REFERENCE 0

typedef struct
{
	int n_blocks;                     // Total number of blocks
	int n_allocated_blocks;           // Allocated == used
	int n_allocated_index_nodes;
	// 8 blocks per byte: One block per bit: 1 = allocated, 0 = free
	// Block 0 (zero) is byte 0, bit 0 
	//       1        is byte 0, bit 1
	//       8        is byte 1, bit 0
	unsigned char block_allocation_table[(MAX_BLOCKS + 7) >> 3];
}VOLUME_BLOCK;

/**********************************************************************/
// Single directory element
typedef struct
{
	// Name of file/directory
	char name[FILE_NAME_SIZE];

	// UNALLOCATED_INDEX_NODE if this directory entry is non-existent
	INDEX_NODE_REFERENCE index_node_reference;

} DIRECTORY_ENTRY;

// Number of directory entries stored in one data block
#define N_DIRECTORY_ENTRIES_PER_BLOCK ((int)(DATA_BLOCK_SIZE / sizeof(DIRECTORY_ENTRY)))

// Maximum number of files that can be contained in a directory (note, a directory can span multiple blocks)
#define MAX_ENTRIES_PER_DIRECTORY (N_DIRECTORY_ENTRIES_PER_BLOCK * 10)

// Directory block
typedef struct directory_block_s
{
	DIRECTORY_ENTRY entry[N_DIRECTORY_ENTRIES_PER_BLOCK];
} DIRECTORY_BLOCK;

/**********************************************************************/
// All-encompassing structure for a disk block
// The union says that all 4 of these elements occupy overlapping bytes in 
//  memory (hence, a block will only be one of these 4 at any given time)

typedef struct
{
	// Next block in a linked list (if this block belongs to one)
	BLOCK_REFERENCE next;
	union {
		DATA_BLOCK data;
		VOLUME_BLOCK volume;
		INDEX_NODE_BLOCK index_nodes;
		DIRECTORY_BLOCK directory;
	} content;
} BLOCK;


/**********************************************************************/
// Representing files (project 4!)

#define MAX_BLOCKS_IN_FILE 1000

typedef struct
{
	INDEX_NODE_REFERENCE index_node_reference;
	char mode;
	int offset;
	int fd_external;

	// Cache for file content details.  Use of these is optional
	int n_data_blocks;
	BLOCK_REFERENCE block_reference_cache[MAX_BLOCKS_IN_FILE];
} MYFILE;


#endif
