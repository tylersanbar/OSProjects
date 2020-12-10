#!/bin/sh
./myfs_format 128
cat project4_test6.sh | ./myfs_create test.sh
./myfs_more test.sh
./myfs_more test_wrong.sh
./myfs_more wrong/test.sh

echo INSPECT
./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1
./myfs_inspect -data 3
./myfs_inspect -data 4

echo MOVE TO DIRECTORY
./myfs_mkd tests
./myfs_move test.sh tests

./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1
./myfs_inspect -data 3
./myfs_inspect -data 4
./myfs_inspect -data 5
./myfs_inspect -index 2
./myfs_inspect -dir 6


