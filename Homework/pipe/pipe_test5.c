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
    
    // Parent will do the reading
    close(fds[1]);

    // Reroute standard in to the pipe
    close(0);
    dup2(fds[0], 0);
    close(fds[0]);

    // FD 0 now points to the output side of the pipe

    char buffer[100];

    // Loop as long as there are lines from stdin
    while(fgets(buffer, 100, stdin)) {
      buffer[99] = 0;
      printf("%d:%s\n", mypid, buffer);
    }
    
    // Wait for the child to exit
    wait(NULL);
    printf("%d: Closing...\n", mypid);

  }else {
    // Child process
    int mypid = getpid();

    // Will write data only (not read)
    close(fds[0]);

    // Reroute standard out
    close(1);
    dup2(fds[1], 1);
    close(fds[1]);
    
    // Standard out goes to the pipe

    for(int i = 0; i < 4; ++i) {
      printf("%d: Count: %d\n", mypid, i);
      fflush(stdout);
      sleep(1);
    }
    // Close standard out
    close(1);
    sleep(1);
  }
  return(0);
};
      
	
