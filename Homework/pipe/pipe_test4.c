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

    // Print a string
    printf("%s\n", str);

    // flush
    fflush(stdout);

    // Wait for the child to exit
    wait(NULL);
    printf("%d: Closing...\n", mypid);

  }else {
    // Child process
    int mypid = getpid();

    // Will receive data only (not write)
    close(fds[1]);

    // Reroute standard in
    close(0);
    dup2(fds[0], 0);
    close(fds[0]);
    
    // Read string
    char buffer[100];
    fgets(buffer, 99, stdin);

    buffer[99] = 0;

    printf("%d: Got %s\n", mypid, buffer);
  }
  return(0);
};
      
	
