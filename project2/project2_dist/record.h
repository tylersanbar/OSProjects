#ifndef RECORD_H
#define RECORD_H

// Maximum size of assignment name
#define NAME_SIZE 20

// Name types
typedef enum {EXAM=0, HOMEWORK, QUIZ, PROJECT} AssignmentType;

// Name Strings
char *type_names[] = {"EXAM", "HOMEWORK", "QUIZ", "PROJECT"};

// Record of a single assignment
typedef struct {
  char valid;
  char name[NAME_SIZE];
  AssignmentType type;
  float max_score;
  float score;
  
}AssignmentRecord;

#endif
