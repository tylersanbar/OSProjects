#!/bin/bash

#Runs all tests

echo Running Tests

./Tests/project4_test1.sh > t1.out
./Tests/project4_test2.sh > t2.out
./Tests/project4_test3.sh > t3.out
./Tests/project4_test4.sh > t4.out
./Tests/project4_test5.sh > t5.out
./Tests/project4_test6.sh > t6.out
./Tests/project4_test7.sh > t7.out
./Tests/project4_test8.sh > t8.out

echo Finished Running Tests

echo Making diff files
diff Tests/project4_test1.out t1.out> t1diff
diff Tests/project4_test2.out t2.out > t2diff
diff Tests/project4_test3.out t3.out > t3diff
diff Tests/project4_test4.out t4.out > t4diff
diff Tests/project4_test5.out t5.out > t1diff
diff Tests/project4_test6.out t6.out > t2diff
diff Tests/project4_test7.out t7.out > t3diff
diff Tests/project4_test8.out t8.out > t4diff

