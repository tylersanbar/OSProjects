#!/bin/sh
./myfs_format 128
echo "Fox in socks." | ./myfs_create fox.txt
echo "Knox on fox in socks in box." | ./myfs_append fox.txt
./myfs_list
./myfs_more fox.txt
./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1
./myfs_inspect -data 3

