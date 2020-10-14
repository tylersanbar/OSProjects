#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include "record.h"
#include <sys/types.h>
#include <sys/stat.h>

//Function Definitions
int printHTML();

//Back-End Database---------------------
//database stored in file called grades.bin
//When program starts, open grades.bin for reading and writing, create if doesn't exist
//File consists of fixed size blocks of AssignmentRecord data structure defined in record.h
//Command line parameters include append, change score, charge record to valid or invalid, and print
int main(int argc, char* argv[]) {
	//Definitions
	char* fileName = "grades.bin";
	char* cmdArg;
	int parser;
	int index = 0;
	int fileSize;
	off_t current;
	AssignmentRecord record;

	//Testing
	if (argc == 1) {
		fprintf(stderr, "Not enough arguments\n");
		return -1;

		//char* newArg[] = { "project2","html" };
		//char* newArg[] = { "project2","server" };
		//char* newArg[] = { "project2","text" };
		//char* newArg[] = { "project2","append", "1234567890", "PROJECT", "525", "361" };
		//char* newArg[] = { "project2","append", "Test2", "113", "1" };
		//char* newArg[] = { "project2","append", "Test3", "865875", "213423" };
		//char* newArg[] = { "project2","valid" "1" };
		//char* newArg[] = { "project2","invalid", "1" };
		//char* newArg[] = { "project2","score", "0", "12345" };
		//argc = 6;
		//argv = newArg;
	}
	cmdArg = argv[1];
	//
	int ARSize = sizeof(AssignmentRecord);
	unsigned char buffer[sizeof(AssignmentRecord)] = { 0 };
	//Command Line Error Handling
	if (argc > 6) {
		fprintf(stderr, "Too many arguments\n");
		return -1;
	}

	//File Management
	int file = open(fileName, O_RDWR | O_CREAT, S_IWRITE | S_IREAD);
	if (file < 0) {
		fprintf(stderr, "Unable to open file.\n");
		return -1;
	}
	fileSize = lseek(file, 0, SEEK_END);

	//printf("Command Arg: %s %d \n",cmdArg, strcmp(cmdArg, "score"));

	//Command Line Arguments
	if (strcmp(cmdArg, "append") == 0) {
		if (argc != 6) {
			fprintf(stderr, "Invalid Arguments\n");
			return -1;
		}
		//Set Record
		record.valid = 1;
		//Set Name
		memset(record.name, 0, NAME_SIZE);
		strncpy(record.name, argv[2], NAME_SIZE - 1);
		//memcpy(record.name, argv[2], (int) fmin(strlen(argv[2]), 19) );
		//Set Assignment Type
		int type = -1;
		for (int i = 0; i < 4; i++) {
			if (strcmp(type_names[i], argv[3]) == 0) {
				type = i;
				break;
			}
		}
		if (type != -1) record.type = type;
		else {
			fprintf(stderr, "No matching type\n");
			return -1;
		}
		char check;
		//Set Max Score
		parser = sscanf(argv[4], "%f%c", &record.max_score,&check);
		if(parser != 1) {
			fprintf(stderr, "Invalid Max Score\n");
			return -1;
		}
		
		//Set Score
		parser = sscanf(argv[5], "%f%c", &record.score,&check);
		if (parser != 1) {
			fprintf(stderr, "Invalid Score\n");
			return -1;
		}
		//Write to end of file
		lseek(file, 0, SEEK_END);
		memcpy(&buffer, &record, ARSize);
		write(file, buffer, ARSize);
	}
	else if (strcmp(cmdArg, "score") == 0) {
		if (argc != 4) {
			fprintf(stderr, "Invalid Arguments\n");
			return -1;
		}
		//Parse Index
		parser = sscanf(argv[2], "%d", &index);
		current = ARSize * index;
		if (current >= fileSize) {
			fprintf(stderr, "Index does not exist.\n");
			return -1;
		}
		lseek(file, current, SEEK_SET);
		//Read Record
		read(file, buffer, ARSize);
		memcpy(&record, &buffer, ARSize);
		//Change Score
		parser = sscanf(argv[3], "%f", &record.score);
		//Write Record
		memcpy(&buffer, &record, ARSize);
		lseek(file, current, SEEK_SET);
		write(file, buffer, ARSize);
	}
	else if (strcmp(cmdArg, "valid") == 0) {
		if (argc != 3) {
			fprintf(stderr, "Invalid Arguments\n");
			return -1;
		}
		//Parse Index
		parser = sscanf(argv[2], "%d", &index);
		current = ARSize * index;
		if (current >= fileSize) {
			fprintf(stderr, "Index does not exist.\n");
			return -1;
		}
		lseek(file, current, SEEK_SET);

		//Read Record
		read(file, buffer, ARSize);
		memcpy(&record, &buffer, ARSize);
		//Change Valid
		record.valid = 1;
		//Write Record
		memcpy(&buffer, &record, ARSize);
		lseek(file, ARSize * index, SEEK_SET);
		write(file, buffer, ARSize);
	}
	else if (strcmp(cmdArg, "invalid") == 0) {
		if (argc != 3) {
			fprintf(stderr, "Invalid Arguments\n");
			return -1;
		}
		//Parse Index
		parser = sscanf(argv[2], "%d", &index);
		current = ARSize * index;
		if (current >= fileSize) {
			fprintf(stderr, "Index does not exist.\n");
			return -1;
		}
		lseek(file, current, SEEK_SET);
		//Read Record
		read(file, buffer, ARSize);
		memcpy(&record, &buffer, ARSize);
		//Change Valid
		record.valid = 0;
		//Write Record
		memcpy(&buffer, &record, ARSize);
		lseek(file, ARSize * index, SEEK_SET);
		write(file, buffer, ARSize);
	}
	else if (strcmp(cmdArg, "text") == 0) {
		current = 0;
		while (current < (fileSize - 1)) {
			lseek(file, current, SEEK_SET);
			current += ARSize;
			read(file, buffer, ARSize);
			memcpy(&record, &buffer, ARSize);
			char* t = type_names[record.type];
			printf("Name: %s\nType: %s\nMax Score:%f\nScore:%f\nValid?:%d\n", record.name, t, record.max_score, record.score, record.valid);
		}
	}
	else if (strcmp(cmdArg, "html") == 0) printHTML();
	else if (strcmp(cmdArg, "server") == 0) {
		//infinite loop
		while (1) {
		//Open grades.html pipe in write mode. Block until a process opens the file for reading
			int pipe = open("grades.html", O_WRONLY);
		//Reroute pipe file descriptor to file descriptor 1
			dup2(pipe, 1);
		//Print full HTML output to STDOUT
			printHTML();
		//Close pipe file descriptor and STDOUT
			close(pipe);
			close("grades.html");
		//Sleep for 1 second
			sleep(1);
		}
		//Repeat
	}

	close(file);
	return 0;
}

int printHTML() {
	//Strings for table
	char validTable[4000] = "";
	char validEntry[150] = "";
	char validAverage[150] = "";
	char invalidTable[4000] = "";
	char invalidEntry[150] = "";
	char invalidAverage[150] = "";
	//AR Data
	AssignmentRecord indexRecord;
	int index = 0;
	int ARSize = sizeof(AssignmentRecord);
	//Average
	float validScoreSum = 0;
	float invalidScoreSum = 0;
	float validMaxSum = 0;
	float invalidMaxSum = 0;
	int numValid = 0;
	int numInvalid = 0;

	//File Management
	char* fileName = "grades.bin";
	unsigned char buffer[sizeof(AssignmentRecord)] = { 0 };
	int file = open(fileName, O_RDWR | O_CREAT, S_IWRITE | S_IREAD);
	if (file < 0) {
		fprintf(stderr, "Unable to open file.\n");
		return -1;
	}
	int fileSize = lseek(file, 0, SEEK_END);
	//Print HTML and Valid Grade table header
	printf(
		"<HTML>\n"
		"<HEAD>\n"
		"<TITLE>CS 3113 Grades</TITLE>\n"
		"</HEAD>\n"
		"<BODY>\n"
		"<H1>CS 3113 Grades</H1>\n"
		"<H2>Valid Grades</H2>\n"
		"<TABLE BORDER=2>\n"
		"<TR>\n"
		"<TD> <B>Index</B>\n"
		"<TD> <B>Assignment Name</B>\n"
		"<TD> <B>Assignment Type</B>\n"
		"<TD> <B>Score</B>\n"
		"<TD> <B>Max Score</B>\n"
	);

	//Print Valid Table
	while (index < (fileSize - 1)) {
		lseek(file, index, SEEK_SET);
		index += ARSize;
		read(file, buffer, ARSize);
		memcpy(&indexRecord, &buffer, ARSize);
		char* t = type_names[indexRecord.type];
		if (indexRecord.valid == 1) {
			printf("<TR>\n<TD> %d\n<TD> %s\n<TD> %s\n<TD> %.2f\n<TD> %.2f\n", (index / ARSize - 1), indexRecord.name, t, indexRecord.score, indexRecord.max_score);
			validScoreSum += indexRecord.score;
			validMaxSum += indexRecord.max_score;
			numValid++;
		}
	}
	//End Table
	printf(
		"</TABLE>\n"
		"<P>\n"
	);
	//Print Valid Average
	if (numValid > 1) printf("<B>Average score: %.2f percent</B>\n", (validScoreSum * 100 / validMaxSum));
	//Non Valid Grades Header
	printf(
		"<P>\n"
		"<H2>Non-Valid Grades</H2>\n"
		"<TABLE BORDER=2>\n"
		"<TR>\n"
		"<TD> <B>Index</B>\n"
		"<TD> <B>Assignment Name</B>\n"
		"<TD> <B>Assignment Type</B>\n"
		"<TD> <B>Score</B>\n"
		"<TD> <B>Max Score</B>\n"
	);
	//Print InValid Table
	index = 0;
	while (index < (fileSize - 1)) {
		lseek(file, index, SEEK_SET);
		index += ARSize;
		read(file, buffer, ARSize);
		memcpy(&indexRecord, &buffer, ARSize);
		char* t = type_names[indexRecord.type];
		if (indexRecord.valid == 0) {
			printf("<TR>\n<TD> %d\n<TD> %s\n<TD> %s\n<TD> %.2f\n<TD> %.2f\n", (index / ARSize - 1), indexRecord.name, t, indexRecord.score, indexRecord.max_score);
			invalidScoreSum += indexRecord.score;
			invalidMaxSum += indexRecord.max_score;
			numInvalid++;
		}
	}
	printf(
		"</TABLE>\n"
		"<P>\n"
	);
	//Print Invalid Average
	if (numInvalid > 1) printf("<B>Average score: %.2f percent</B>\n", (invalidScoreSum * 100 / invalidMaxSum));
	printf(
		"<H2>References</H2>\n"
		"<UL>\n"
		"<LI><A HREF=\"https://cs.ou.edu/~fagg/classes/cs3113\">Introduction to Operating Systems</A>\n"
		"<LI><A HREF=\"https://cs.ou.edu\">Computer Science Department</A>\n"
		"</UL>\n"
		"<P>\n"
		"</BODY>\n"
		"</HTML>\n"
	);
	close(file);
	fflush(stdout);
	return 0;
}
