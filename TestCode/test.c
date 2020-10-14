#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

void * checkType(void* arg, char expectedType);

int main(int argc, char* argv[]) {
	float f = 3.1415;
	char* str;
	float f;
	void* arr[] = { &str, &f };
	void* returnedValue = checkType(&f, type);
	return 0;
}

//Scans argv string for expected type, redirects array of pointers to parsed values 
void ** parseCmdArgs(int args, char* argv, void* expectedTypes[]) {
	const char formatString[50] = { 0 };
	char checkChar;
	void* returnPointer = 0;
	int valuesParsed;

	strncat(formatString, "%", 1);
	strncat(formatString, &expectedType, 1);
	strncat(formatString, "%c", 2);


	valuesParsed = sscanf(arg, formatString, returnPointer, &checkChar);
	if (valuesParsed != 1) {
		fprintf(stderr, "Invalid type");
		return -1;
	}
	return returnPointer;
}