//Tyler Sanbar - Project 1

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>

#define INBUFSIZE 200
#define MAX_ARGS 3
#define BLOCKSIZE 112

int main(int argc, char* argv[]) {

	char cmdArg;
	int location;
	int block;
	int value;
	float fValue;
	int offset;
	unsigned char hexVal;

	int testing = 0;
	int currTest = 1;
	int numTests = 6;
	char inputBuffer [500];

	char outputBuffer[500];
	FILE* in = NULL;
	FILE* out = NULL;

	//Command Line Argument Error Handling
	if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		return 1;
	}
	//Sets default filename, if command line file name is supplied, updates it
	char* fileName = "data.bin";
	if(argc == 2) fileName = argv[1];
	//Assigns int file descriptor and opens file for reading and writing, if it doesn't exist, creates new file
	int file = open(fileName, O_RDWR | O_CREAT);
	if (file < 0) {
		fprintf(stderr, "Unable to open file.\n");
	}
	//Creates 112 byte buffer with all elements initialized to 0
	unsigned char byteBuffer[BLOCKSIZE];
	memset(byteBuffer, 0, BLOCKSIZE);

	//Declare and initialize variables for tokenizer
	const char SEPARATORS[] = " \t\n";         // Possible separators between tokens
	char in_buffer[INBUFSIZE];                 // Input buffer from STDIN
	char* args[30] = { NULL };                     // Array of pointers to strings
	char** arg;                               // Working pointer that steps through the args (a pointer to a pointer to a string)

testLoop:
	if (testing == 1) {
		sprintf(inputBuffer, "Test/input%d.txt", currTest);
		sprintf(outputBuffer, "Test/testOutput%d.txt", currTest);

		in = freopen(inputBuffer, "r", stdin);
		out =  freopen(outputBuffer, "w", stdout);
		currTest++;
	}
	//Gets input, if EOF, exit
	while ((fgets(in_buffer, INBUFSIZE, stdin) != NULL)) {
		//String Tokenizer
		arg = args;
		*arg++ = strtok(in_buffer, SEPARATORS);   // tokenize input
		while ((*arg++ = strtok(NULL, SEPARATORS)));
		if ((args[3] != NULL) | (args[0] == NULL)) {
			fprintf(stderr, "Invalid number of arguments\n");
			continue;
		}
		//Assign command Argument to first token
		cmdArg = *args[0];

		//Case Switch for all arguments
		switch (cmdArg) {
		case 'd':
			if (args[1] != NULL) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			//16jx7i
			for (int i = 0; i < 7; i++) {
				//Print row i x column j
				for (int j = 0; j < 16; j++) printf("%02x ", (unsigned int)byteBuffer[(i * 16) + j]);
				//New line
				printf("\n");
			}
			break;
		case '0':
			if (args[1] != NULL) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			memset(byteBuffer, 0, BLOCKSIZE);
			break;
		case 'H':
			if ((args[2] == NULL) | (args[1] == NULL)) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			//H LOC VAL
			//Set int location equal to second argument string 
			location = atoi(args[1]);
			//Set int value equal to third argument's hexidecimal input string
			byteBuffer[location] = strtol(args[2], NULL, 16);
			//Set the memory in the buffer at the location equal to the value
			//d, 0, h, H, i and I
			break;
		case 'h':
			if ((args[2] != NULL) | (args[1] == NULL)) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			printf("%x\n", (unsigned int) byteBuffer[atoi(args[1])]);
			break;
		case 'C':
			if ((args[2] == NULL) | (args[1] == NULL)) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			//C LOC VAL
			//Set int location equal to second argument string 
			location = atoi(args[1]);
			//Set the buffer char equal to the argument char
			byteBuffer[location] = (unsigned char) *args[2];
			break;
		case 'c':
			if ((args[2] != NULL) | (args[1] == NULL)) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			location = atoi(args[1]);
			printf("%c\n", byteBuffer[location]);
			break;
		case 'I':
			if ((args[2] == NULL) | (args[1] == NULL)) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			location = atoi(args[1]);
			value = atoi(args[2]);
			memcpy(&byteBuffer[location], &value, sizeof(value));
			break;
		case 'i':
			if ((args[2] != NULL) | (args[1] == NULL)) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			location = atoi(args[1]);
			value = 0;
			//Copies 4 bytes of memory starting from location in bytebuffer to integer of value
			memcpy(&value, &byteBuffer[location], sizeof(value));
			//Prints integer value in decimal format
			printf("%d\n", value);
			break;
		case 'F':
			if ((args[2] == NULL) | (args[1] == NULL)) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			location = atoi(args[1]);
			fValue = atof(args[2]);
			memcpy(&byteBuffer[location], &fValue, sizeof(value));
			break;
		case 'f':
			if ((args[2] != NULL) | (args[1] == NULL)) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			location = atoi(args[1]);
			fValue = 0.0;
			//Copies 4 bytes of memory starting from location in bytebuffer to float of value
			memcpy(&fValue, &byteBuffer[location], sizeof(fValue));
			//Prints integer value in decimal format
			printf("%f\n", fValue);
			break;
		case 'S':
			if ((args[2] == NULL) | (args[1] == NULL)) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			location = atoi(args[1]);
			//Copies string length number of bytes from args[2] to byteBuffer at location
			memcpy(&byteBuffer[location], args[2], strlen(args[2]) + 1);
			break;
		case 's':
			if ((args[2] != NULL) | (args[1] == NULL)) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			location = atoi(args[1]);
			while(byteBuffer[location] != 0) printf("%c", byteBuffer[location++]);
			printf("\n");
			break;
		case 'A':
			if ((args[2] == NULL) | (args[1] == NULL)) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			location = atoi(args[1]);
			//Finds end of string at specified location
			while (byteBuffer[location] != 0) byteBuffer[location++];
			//Copies string length number of bytes from args[2] to byteBuffer at new location
			memcpy(&byteBuffer[location], args[2], strlen(args[2]) + 1);
			break;
		case 'w':
			if ((args[2] != NULL) | (args[1] == NULL)) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			//Set offset to specified block
			offset = atoi(args[1]) * BLOCKSIZE;
			lseek(file, offset, SEEK_SET);
			write(file, byteBuffer, BLOCKSIZE);
			break;
		case 'r':
			if ((args[2] != NULL) | (args[1] == NULL)) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			//Set offset to specified block
			offset = atoi(args[1]) * BLOCKSIZE;
			lseek(file, offset, SEEK_SET);
			read(file, byteBuffer, BLOCKSIZE);
			break;
		case '+':
			if ((args[2] != NULL) | (args[1] == NULL)) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			location = atoi(args[1]);
			value = 0;
			memcpy(&value, &byteBuffer[location], sizeof(value));
			value++;
			memcpy(&byteBuffer[location], &value, sizeof(value));
			break;
		case '-':
			if ((args[2] != NULL) | (args[1] == NULL)) {
				fprintf(stderr, "Invalid number of parameters\n");
				continue;
			}
			location = atoi(args[1]);
			value = 0;
			memcpy(&value, &byteBuffer[location], sizeof(value));
			value--;
			memcpy(&byteBuffer[location], &value, sizeof(value));
			break;
		default:
			fprintf(stderr, "No argument matching input\n");
			break;
		}
	}
	while ((currTest <= numTests) && (testing == 1)) {
		fclose(in);
		fclose(out);
		//printf("Test");
		goto testLoop;
	}
	close(file);
	return 0;
}