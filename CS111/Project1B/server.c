#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <assert.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <mcrypt.h>
#include <netinet/in.h>
#include <sys/socket.h>

// Variables to use with options
int port_flag = 0;
int port_no;
int encrypt_flag = 0;

// Pipe variables
int pID = -1;

// Encryption variables
MCRYPT td;
int i;
int key_fd = -1;
char *key; /* created using mcrypt_gen_key */
char *IV;
int keysize = 24; /* 192 bits == 24 bytes */

// Server/Client variables
int port_no;
int connection_flag;
int clilen;
int socket_fd_1, socket_fd_2;
struct sockaddr_in serv_addr, cli_addr;

// Pipes
int pipe_from_shell[2];     
int pipe_to_shell[2];

/* to store options for getopt */
static struct option long_options[] = {
  { "port", required_argument, 0, 'p' },
  { "encrypt", no_argument, 0, 'e' }
};

void sigpipe_handler() {
  close(socket_fd_1);
  close(socket_fd_2);
  if (encrypt_flag) {
    mcrypt_generic_deinit(td);
    mcrypt_module_close(td);
  }
  exit(2); // EOF or SIGPIPE
}

void setup_mcrypt()
{
  key = calloc(1, keysize);

  td = mcrypt_module_open("twofish", NULL, "cfb", NULL);
  if (td == MCRYPT_FAILED) {
    fprintf(stderr, "Error opening mcrypt module.\n");
    exit(1);
  }

  key_fd = open("my.key", O_RDONLY);
  if (key_fd < 0) {
    fprintf(stderr, "Error opening my.key.\n");
    exit(1);
  }

  IV = malloc(mcrypt_enc_get_iv_size(td));
  for (i=0; i < mcrypt_enc_get_iv_size(td); i++) {
    IV[i]=rand();
  }

  if (i = mcrypt_generic_init(td, key, keysize, IV) < 0) {
    fprintf(stderr, "Error with encryption.\n");
    mcrypt_perror(i);
    exit(1);
 }

 // close(key_fd); // TODO: are these three lines necessary
 // free(key);
 // free(IV);
}

void setup_server(void)
{    
  socket_fd_1 = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd_1 < 0) {
    fprintf(stderr, "Error opening socket.\n");
    exit(1);
  }

  memset((char *) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port_no);

  if (bind(socket_fd_1, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    fprintf(stderr, "Error in binding server address.\n");
    exit(1);
  }
  else
    connection_flag = 1;

  listen(socket_fd_1, 5);
  clilen = sizeof(cli_addr);
  
  socket_fd_2 = accept(socket_fd_1, (struct sockaddr *) &cli_addr, &clilen);
  if (socket_fd_2 < 0) {
    fprintf(stderr, "Error in accepting connection.\n");
    exit(1);
  }
}

void thread()
{
  char c;
  while (read(pipe_from_shell[0], &c, 1)) {
    if (encrypt_flag)
      mcrypt_generic(td, &c, 1);
    write(STDOUT_FILENO, &c, 1);
  }
  close(socket_fd_2);
  close(socket_fd_1);
  kill(pID, SIGKILL);
  exit(2);
}

void parent_process(void)
{
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  close(pipe_to_shell[0]);
  close(pipe_from_shell[1]);

  pthread_t my_thread;
  if (pthread_create(&my_thread, NULL, (void*)thread, NULL)) {
    fprintf(stderr, "Error creating parent's read thread.\n");
    exit(1);
  }

  dup2(socket_fd_2, STDIN_FILENO);
  dup2(socket_fd_2, STDOUT_FILENO);
  dup2(socket_fd_2, STDERR_FILENO);
  close(socket_fd_2);

  char c;
  while (read(STDIN_FILENO, &c, 1) > 0) {
    if (encrypt_flag)
      mdecrypt_generic(td, &c, 1); // TODO: change back to decrypt
    write(pipe_to_shell[1], &c, 1);
  }
  close(socket_fd_2);
  close(socket_fd_1);
  kill(pID, SIGKILL);
  exit(1); // read error or EOF
}

void child_process(void)
{
  close(pipe_to_shell[1]);
  close(pipe_from_shell[0]);
  dup2(pipe_to_shell[0], STDIN_FILENO);
  dup2(pipe_from_shell[1], STDOUT_FILENO);
  close(pipe_to_shell[0]);
  close(pipe_from_shell[1]);

  char *execvp_argv[2];
  char execvp_filename[] = "/bin/bash";
  execvp_argv[0] = execvp_filename;
  execvp_argv[1] = NULL;
  if (execvp(execvp_filename, execvp_argv) == -1) {
    fprintf(stderr, "Error creating bash shell, execvp() failed.\n");
    exit(1);
  }
}

void setup_shell(void)
{
  if (pipe(pipe_from_shell) == -1 || pipe(pipe_to_shell) == -1) {
    fprintf(stderr, "pipe() failed!\n");
    exit(1);
  }

  pID = fork();

  if (pID > 0) parent_process();
  if (pID == 0) child_process();
}


int main(int argc, char** argv)
{
  
  int ret = 0;
  int options_index = 0;

  while (1)
  {
    signal(SIGPIPE, sigpipe_handler);

    int ret = 0;
    ret = getopt_long(argc, argv, "", long_options, &options_index);
    
    if (ret == -1)
      break;
    
    switch (ret)
    {
      case 'p':
        port_flag = 1;
        port_no = atoi(optarg);
        break;
      case 'e':
        encrypt_flag = 1;
        break;
      default:
        abort();
    }
  }

  if (!port_flag) {
    fprintf(stderr, "No port specified.\n");
    exit(1);
  }

  setup_server();
  if(encrypt_flag)
    setup_mcrypt();
  if(connection_flag)
    setup_shell();
  return 0;
}

