#!/bin/sh
./myfs_format 128
cat project4_test7.sh | ./myfs_create test.sh
./myfs_more test.sh
./myfs_move test.sh test1.sh
./myfs_list

echo INSPECT
./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1
./myfs_inspect -data 3
./myfs_inspect -data 4

echo LINK
./myfs_link test1.sh test1a.sh
./myfs_list

echo INSPECT
./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1
./myfs_inspect -data 3
./myfs_inspect -data 4

echo CREATE DIR
./myfs_mkd tests
./myfs_move test1a.sh tests

echo INSPECT
./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1
./myfs_inspect -data 3
./myfs_inspect -data 4
./myfs_inspect -index 2
./myfs_inspect -dir 9

echo AGUMENT
echo "# one more line" | ./myfs_append test1.sh
./myfs_more tests/test1a.sh

echo INSPECT
./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1
./myfs_inspect -data 3
./myfs_inspect -data 4
./myfs_inspect -index 2
./myfs_inspect -dir 9

echo RM
./myfs_rm test1.sh

echo INSPECT
./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1
./myfs_inspect -data 3
./myfs_inspect -data 4
./myfs_inspect -index 2
./myfs_inspect -dir 9

echo RM
./myfs_rm tests/test1a.sh

echo INSPECT
./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1
./myfs_inspect -index 2
./myfs_inspect -dir 9
