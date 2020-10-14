#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


// Example of using named pipes

int main(int argc, char** argv)
{
  int fd = open("mynamedpipe", O_WRONLY);
  
  // Reroute standard out to the named pipe
  close(1);
  dup2(fd, 1);
  close(fd);

  printf("Foo\n");
  printf("Bar\n");
  printf("Baz\n");

  fflush(stdout);
  close(1);

  return(0);
};
      
	
