#include <stdio.h>
#include "myfs.h"

int main(int argc, char **argv)
{
  printf("BLOCK_SIZE: %d\n", (int) sizeof(BLOCK));
  printf("DATA_BLOCK_SIZE: %d\n", (int) sizeof(DATA_BLOCK));
  printf("VOLUME_BLOCK_SIZE: %d\n", (int) sizeof(VOLUME_BLOCK));
  printf("INDEX_NODE_BLOCK_SIZE: %d\n", (int) sizeof(INDEX_NODE_BLOCK));
  printf("BLOCK_REFERENCE_SIZE: %d\n", (int) sizeof(BLOCK_REFERENCE));

  printf("DIRECTORY_BLOCK_SIZE: %d\n", (int) sizeof(DIRECTORY_BLOCK));
  printf("MAX_BLOCKS: %d\n", MAX_BLOCKS);
  printf("UNALLOCATED_BLOCK reference: %d\n", UNALLOCATED_BLOCK);
  printf("UNALLOCATED_INDEX_NODE reference: %d\n", UNALLOCATED_INDEX_NODE);
  printf("DATA_BLOCK_SIZE: %d\n", DATA_BLOCK_SIZE);
  printf("INDEX_NODES_PER_BLOCK: %d\n", N_INDEX_NODES_PER_BLOCK);
  //  printf("N_INDEX_NODES: %d\n", N_INDEX_NODES);
  printf("DIRECTORY_ENTRIES_PER_BLOCK: %d\n", N_DIRECTORY_ENTRIES_PER_BLOCK);

  /*
  BLOCK b;

  printf("Block location: %x\n", &b);
  printf("Next block location: %x\n", &b.next_block);
  printf("Content block location: %x\n", &b.content);
  printf("Data block location: %x\n", &b.content.data);
  printf("Volume block location: %x\n", &b.content.volume);
  */
}
