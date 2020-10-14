#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

int main(void) {
	//Question 1
	int q1 = 217 & 233;
	printf("Question 1: %d\n", q1);

	//Question 2
	int q2 = 0x217 & 0x233;
	printf("Question 2: %x\n", q2);

	//Question 3
	unsigned char a[10];
	*(int*)(&a[4]) = 0x12345678;
	int q3 = a[6];
	printf("Question 3: %x\n", q3);

	//Question 4
	unsigned char b[40];
	strcpy(&b[20], "12345678");
	int q4 = b[27];
	printf("Question 4: %c\n", q4);

	//Question 5
	unsigned char c[40];
	strcpy(&c[20], "12345678");
	strcat(&c[23], "ABCDEFGH");
	printf("%s\n", &c[20]);

	//Question 6
	unsigned char hunbytes[100];
	int file = open("myfile.bin", O_RDWR | O_APPEND | O_CREAT, S_IWRITE | S_IREAD);
	write(file, &hunbytes, sizeof(hunbytes));
	
	int q6 = read(file, hunbytes, 5);
	close(file);
	printf("Question 6: %d\n", q6);

	//Question 7
	//Get file with 100 bytes

	//
	int i = 5;
	int fd = open("myfile.bin", O_RDWR | O_APPEND);
	write(fd, &i, sizeof(i));
	int pos = lseek(fd, 0, SEEK_CUR);
	close(file);
	printf("Question 7: %d\n", pos);
	//Question 8
	//When called by a user process, the function strcmp() executes in

	//Question 9
	//True or False? printf() is a system call

	//Question 10
	//True or False? System calls are computationally more expensive than function calls.

	//Question 11
	//True or False?  A system call in Linux is represented using a unique integer

	//Question 12
	//True or False? The stack is used to store the current program counter as one function calls another function.

	//Question 13
	//The Process Control Block stores which of the following information?

	//Question 14
	//True or False?  A context switch refers to the retrieving of register values from the stack on return from a function call.

	//Question 15
	//True or False?  It is the job of the process scheduler to pick the process that will transition from the ready state to the running state

	//Question 16
	//True or False?  A process can only modify the contents of the stack in kernel mode

	//Question 17
	/*Consider the following Makefile:
	all: clean

	myprogram: myprogram.c
		gcc myprogram.c -o myprogram

	clean:
		 rm -f *.o myprogram
	What happens when you type 'make' at the command line?
	*/

	//Question 18
	/* Consider the following Makefile:
	FILE=homework1

	all: $(FILE)

	$(FILE): $(FILE).c
		gcc $(FILE).c -o myprogram

	clean:
		rm -f *.o $(FILE) myprogram
	What happens when you type 'make' at the command line?
	*/

	//Question 19
	//Which of the following is not a role played by the gcc compiler?

	//Question 20
	//True or False?  A cache is typically larger in size than main memory.
}