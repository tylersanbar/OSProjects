#!/bin/bash

#Runs all tests

echo Running Tests

./Tests/test1 > t1.out
./Tests/test2 > t2.out
./Tests/test3 > t3.out
./Tests/test4 > t4.out

echo Finished Running Tests

echo Making diff files
diff Tests/test1.out t1.out > t1diff
diff Tests/test2.out t2.out > t2diff
diff Tests/test3.out t3.out > t3diff
diff Tests/test4.out t4.out > t4diff
