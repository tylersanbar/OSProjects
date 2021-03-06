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
./myfs_copy stories/fox.txt newfox.txt
./myfs_mkd to_read
./myfs_copy newfox.txt to_read/newfox.txt
echo "Socks on Knox and Knox in box." | ./myfs_append to_read/newfox.txt
./myfs_list
./myfs_list stories
./myfs_list to_read
./myfs_more to_read/newfox.txt
./myfs_move stories/fox.txt to_read
./myfs_list stories
./myfs_list to_read
./myfs_more to_read/fox.txt

./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1
./myfs_inspect -dir 3
./myfs_inspect -index 2
./myfs_inspect -index 3
./myfs_inspect -index 4
./myfs_inspect -index 5
./myfs_inspect -dir 6

./myfs_move to_read/newfox.txt oldfox.txt
./myfs_more oldfox.txt

./myfs_inspect -volume
./myfs_inspect -index 0
./myfs_inspect -dir 2
./myfs_inspect -index 1
./myfs_inspect -dir 3
./myfs_inspect -index 2
./myfs_inspect -index 3
./myfs_inspect -index 4
./myfs_inspect -index 5
./myfs_inspect -dir 6
