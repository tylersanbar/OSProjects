#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** argv)
{
  int fds[2];

  // Create the pipe
  if(pipe(fds) != 0) {
    fprintf(stderr, "Error opening pipe\n");
    exit(-1);
  }

  // Fork into two processes
  int pid;
  
  pid = fork();
  if(pid < 0) {
      fprintf(stderr, "Error forking\n");
      exit(-1);
  }else if(pid > 0) {
    // Parent process
    int mypid = getpid();
    int val;
    
    // Parent will do the writing
    close(fds[0]);

    // Close fd 1
    close(1);
    dup2(fds[1], 1);
    close(fds[1]);

    // FD 1 now points to the input of the pipe
    
    // Write a string
    char str[] = "foo bar baz";
    int len = strlen(str) + 1;   // +1 to include the terminator
    
    if(write(1, str, len) != len) {
      perror("Write str error");
      exit(-1);
    }
    
    // Wait for the child to exit
    wait(NULL);
    printf("%d: Closing...\n", mypid);

  }else {
    // Child process
    int mypid = getpid();

    // Will receive data only (not write)
    close(fds[1]);
    
    // Read string
    char buffer[100];
    if(read(fds[0], buffer, 99) < 0) {
      perror("str read error");
      exit(-1);
    }
    buffer[99] = 0;

    printf("%d: Got %s\n", mypid, buffer);
  }
  return(0);
};
      
	
