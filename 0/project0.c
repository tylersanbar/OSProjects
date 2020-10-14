#define _CRT_SECURE_NO_WARNINGS
#define INBUFSIZE 1000
#define MAX_ARGS 20
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Define grammar strings
char* digitStrings[] = { "zero","one","two","three","four","five","six","seven","eight","nine" };
char* teenStrings[] = { "ten","eleven","twelve","thirteen","fourteen","fifteen","sixteen","seventeen","eighteen","nineteen" };
char* tenStrings[] = { "twenty","thirty","forty","fifty","sixty","seventy","eighty","ninty" };
char* hundredString = "hundred";
char* thousandString = "thousand";
char* millionString = "million";

//Define function prototypes
int translateString(char* inputStr);

int main(int argc, char* argv[]) {
	//Command Line Argument Error Handling
	if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		return 1;
	}
	char cmdArg;
	if (argc == 2) {
		cmdArg = *argv[1];
	}
	else cmdArg = 'd';
	if (cmdArg != 'd' && cmdArg != 'h' && cmdArg != 'H' && cmdArg != 'o') {
		fprintf(stderr, "Invalid argument\n");
		return 2;
	}
	//Declare and initialize variables for tokenizer
	const char SEPARATORS[] = " \t\n";         // Possible separators between tokens
	char in_buffer[INBUFSIZE];                 // Input buffer from STDIN
	char* args[MAX_ARGS];                     // Array of pointers to strings
	char** arg;                               // Working pointer that steps through the args (a pointer to a pointer to a string)
	//Gets input, if EOF, exit
	while (fgets(in_buffer, INBUFSIZE, stdin) != NULL) {
		//String Tokenizer
		arg = args;
		*arg++ = strtok(in_buffer, SEPARATORS);   // tokenize input
		while ((*arg++ = strtok(NULL, SEPARATORS)));
		//Initialize variables for number calculation
		int newNum = 0;
		int partialTotal = 0;
		int millions = 0;
		int thousands = 0;
		int hundreds = 0;
	    int total = 0;
		arg = args;
		//Calculate number from tokens
		while (*arg != NULL) {
			newNum = translateString(*arg++);
			//Add if 1-99
			if (newNum < 100) partialTotal += newNum;
			//Multiply if hundred,thousand,million
			if (newNum >= 100) partialTotal *= newNum;
			//Reset for million
			if (newNum == 1000000) {
				millions = partialTotal;
				partialTotal = 0;
				hundreds = 0;
			}
			//Reset for thousand
			if (newNum == 1000) {
				thousands = partialTotal;
				partialTotal = 0;
				hundreds = 0;
			}
			//Update hundreds
			if (newNum <= 100) hundreds = partialTotal;
		}
		//Compute total
		total = millions + thousands + hundreds;
		//Print with command line argument format
		switch (cmdArg) {
		case 'd' :
			printf("%d\n", total);
			break;
		case'h':
			printf("%x\n", total);
			break;
		case'H':
			printf("%X\n", total);
			break;
		case'o':
			printf("%o\n", total);
			break;
		}
	}
	return 0;
}
/// <summary>
/// Searches through grammar arrays and returns integer that matches the input string
/// </summary>
/// <param name="inputStr"></param>
/// <returns>int</returns>
int translateString(char* inputStr) {
	char** grammarScanner;
	int number = -1;
	int noMatch = 1;
	//Check digits
	grammarScanner = digitStrings;
	while (number < 9 && noMatch) {
		noMatch = strcmp(inputStr, *grammarScanner++);
		number++;
	}
	//Check teens
	grammarScanner = teenStrings;
	while (number < 19 && noMatch) {
		noMatch = strcmp(inputStr, *grammarScanner++);
		number++;
	}
	if (noMatch) number = 10;
	//Check tens
	grammarScanner = tenStrings;
	while (number < 90 && noMatch) {
		noMatch = strcmp(inputStr, *grammarScanner++);
		number += 10;
	}
	//Check hundred
	if (!strcmp(inputStr, hundredString)) number = 100;
	//Check thousand
	if (!strcmp(inputStr, thousandString)) number = 1000;
	//Check million
	if (!strcmp(inputStr, millionString)) number = 1000000;
	return number;
}