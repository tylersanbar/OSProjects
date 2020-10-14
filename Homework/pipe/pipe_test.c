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
    
    printf("%d: Give me an int: ", mypid);
    scanf("%d", &val);
    val += 5;
    
    if(write(fds[1], &val, sizeof(val)) != sizeof(val)) {
      perror("Write error");
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
    
    int myval;
    if(read(fds[0], &myval, sizeof(myval)) != sizeof(myval)) {
      perror("read");
      exit(-1);
    }
    printf("%d: Got %d\n", mypid, myval);
  }
  return(0);
};
      
	
