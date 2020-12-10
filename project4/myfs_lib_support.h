#ifndef MYFS_LIB_SUPPORT_H
#define MYFS_LIB_SUPPORT_H

#include "myfs_lib.h"

// Project 3
void myfs_set_index_node(INDEX_NODE* inode, INDEX_NODE_TYPE type, int references,
	BLOCK_REFERENCE content, int size);
int myfs_read_index_node_by_reference(INDEX_NODE_REFERENCE i, INDEX_NODE* inode);
int myfs_write_index_node_by_reference(INDEX_NODE_REFERENCE i, INDEX_NODE* inode);

void myfs_init_directory_block_and_index_node(INDEX_NODE* inode, BLOCK* block,
	BLOCK_REFERENCE self_block_reference,
	INDEX_NODE_REFERENCE self_inode_reference,
	INDEX_NODE_REFERENCE parent_inode_reference);

//INDEX_NODE_REFERENCE myfs_find_entity_in_directory(INDEX_NODE* index_node, char* element_name);
INDEX_NODE_REFERENCE myfs_find_entity_in_directory(INDEX_NODE* index_node, char* element_name,
	BLOCK* block, BLOCK_REFERENCE* ref, int* entry_index);
int myfs_path_to_index_node(char* cwd, char* path, INDEX_NODE_REFERENCE* parent,
	INDEX_NODE_REFERENCE* child, char* local_name);


int myfs_path_to_index_node_general(char* cwd, char* path, INDEX_NODE_REFERENCE* parent,
	INDEX_NODE_REFERENCE* child,
	char* local_name,
	BLOCK* directory_block,
	BLOCK_REFERENCE* directory_block_ref,
	int* entity_index);

int myfs_find_directory_hole(BLOCK_REFERENCE head_directory_block_ref,
	BLOCK* directory_block,
	BLOCK_REFERENCE* directory_block_ref,
	int* directory_index);

INDEX_NODE_REFERENCE myfs_find_index_node_hole(BLOCK* block, BLOCK_REFERENCE* bRef);
int myfs_find_open_bit(unsigned char value);
BLOCK_REFERENCE myfs_allocate_new_block(BLOCK* volume_block);
int myfs_remove_directory_entry(BLOCK_REFERENCE dir_ref, char* name);
int myfs_deallocate_blocks(BLOCK* volume_block, BLOCK_REFERENCE bref);
void myfs_clear_all_directory_entries(BLOCK* directory_block);
void myfs_clear_all_index_node_entries(BLOCK* index_node_block);
int myfs_append_new_block_to_existing_block(BLOCK* volume_block,
	BLOCK* block,
	BLOCK_REFERENCE* block_ref);
// Project 4
INDEX_NODE_REFERENCE myfs_create_file(BLOCK* volume_block,
	INDEX_NODE_REFERENCE parent, char* name);
int myfs_delete_file(char* cwd, char* path);

#endif
