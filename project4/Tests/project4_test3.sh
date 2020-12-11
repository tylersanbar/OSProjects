#!/bin/sh
./myfs_format 128
./myfs_mkd stories
export MYFS_CWD=/stories
echo "Fox in socks." | ./myfs_append fox.txt
echo "Knox on fox in socks in box." | ./myfs_append fox.txt
export MYFS_CWD=/
./myfs_list
./myfs_list stories
./myfs_more stories/fox.txt
./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1
./myfs_inspect -dir 3
./myfs_inspect -index 2
./myfs_inspect -data 4
