#!/bin/sh
./myfs_format 128
./myfs_touch foo
./myfs_list
./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1

