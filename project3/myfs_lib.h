#ifndef MYFS_LIB_H
#define MYFS_LIB_H
#include "myfs.h"

#define MAX_PATH_LENGTH 200

extern int debug;

// PROVIDED
void myfs_get_environment(char *cwd, char *disk_name);

// PROJECT 3: to implement
int myfs_format_disk(char  *virtual_disk_name, int n_blocks);
int myfs_list(char *cwd, char *path);
int myfs_mkd(char *cwd, char *path);
int myfs_rmd(char *cwd, char *path);


// PROJECT 4: to implement
MYFILE* myfs_fopen(char *cwd, char *path, char *mode);
void myfs_fclose(MYFILE *fp);
int myfs_fwrite(MYFILE *fp, unsigned char * buf, int len);
int myfs_fread(MYFILE *fp, unsigned char * buf, int len);
int myfs_delete_file(char *cwd, char *path);
int myfs_link(char *cwd, char *path_src, char *path_dst);

#endif

