#!/bin/sh
rm -f unixfile2.txt
./myfs_format 128
cat project4_test8.sh | ./myfs_create test1.sh
./myfs_mkp unixfile2.txt test2.sh
./myfs_more test2.sh

echo INSPECT
./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1
./myfs_inspect -data 3
./myfs_inspect -data 4
./myfs_inspect -index 2

echo COPY
./myfs_copy test1.sh test2.sh
cat unixfile2.txt

echo INSPECT
./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1
./myfs_inspect -data 3
./myfs_inspect -data 4
./myfs_inspect -index 2
./myfs_inspect -data 8

./myfs_mkp unixfile3.txt test3.sh
./myfs_copy test2.sh test3.sh
./myfs_more test3.sh
cat unixfile3.txt

echo INSPECT
./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1
./myfs_inspect -data 3
./myfs_inspect -data 4
./myfs_inspect -index 2
./myfs_inspect -data 8
./myfs_inspect -index 3
./myfs_inspect -data 9

echo LINE COUNT
./myfs_lc test1.sh
./myfs_lc test3.sh
wc unixfile2.txt
wc unixfile3.txt



