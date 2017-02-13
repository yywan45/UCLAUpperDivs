#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void signal_handler(int signum) {
  fprintf(stderr, "Signal number %d received. Quitting program now.\n", signum);
  exit(3);
}


int main(int argc, char *argv[]) {

  struct option long_options[] = {
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"segfault", no_argument, 0, 's'},
    {"catch", no_argument, 0, 'c'},
    {0, 0, 0, 0}
  };

  
  int ret = 0;
  int ifd = 0;
  int ofd = 0;
  int segfault_flag = 0;
  int catch_flag = 0;
  int options_index = 0;
  
  while (1) {

    ret = getopt_long(argc, argv, "i:o:sc", long_options, &options_index);
    
    if (ret == -1)
      break;
    
    switch (ret) {
    case 'i':
      ifd = open(optarg, O_RDONLY);
      if (ifd >= 0) {
	close(0);
	dup(ifd);
	close(ifd);
      }
      else {
	fprintf(stderr, "Unable to open the specified input file: %s\n", optarg);
        perror("Unable to open the specified input file");
        exit(1);
      }
      break;
    case 'o':
      ofd = creat(optarg, 0666);
      if (ofd >= 0) {
	close(1);
	dup(ofd);
	close(ofd);
      }
      else {
        fprintf(stderr, "Unable to create the specified output file: %s\n", optarg);
        perror("Unable to create the specified output file");
        exit(2);
      }
      break;
    case 's':
      segfault_flag = 1;
      break;
    case 'c':
      catch_flag = 1;
      break;
    }
  }

  if (catch_flag) {
    signal(SIGSEGV, signal_handler);
  }
  
  if (segfault_flag) {
    char* segfault_char = NULL;
    segfault_char[0] = 'x';
  }
  
  int read_size = 0;
  char buffer;
  
  while ((read_size = read(0, &buffer, 1)) > 0)
    write(1, &buffer, read_size);
  
  return 0;
}

    
