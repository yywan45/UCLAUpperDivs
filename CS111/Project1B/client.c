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
#include <netdb.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/socket.h>

/* Use this variable to remember original terminal attributes. */
struct termios saved_attributes;

// Variables to use with options
int port_flag = 0;
int encrypt_flag = 0;
int log_flag = 0;
int log_fd = -1;

// Encryption variables
MCRYPT td;
int i;
int key_fd = -1;
char *key; /* created using mcrypt_gen_key */
char *IV;
int keysize = 24; /* 192 bits == 24 bytes */

// Server/Client variables
int port_no;
int socket_fd;
struct sockaddr_in serv_addr;
struct hostent *server;

/* to store options for getopt */
static struct option long_options[] = {
  { "port", required_argument, 0, 'p' },
  { "encrypt", no_argument, 0, 'e' },
  { "log", required_argument, 0, 'l' }
};

void reset_input_mode(void)
{
  tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
}

void set_input_mode(void)
{
  struct termios tattr;

  /* Make sure stdin is a terminal. */
  if (!isatty (STDIN_FILENO))
    {
      fprintf (stderr, "Not a terminal.\n");
      exit (EXIT_FAILURE);
    }

  /* Save the terminal attributes so we can restore them later. */
  tcgetattr (STDIN_FILENO, &saved_attributes);
  atexit (reset_input_mode);

  /* Set the funny terminal modes. */
  tcgetattr (STDIN_FILENO, &tattr);
  tattr.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
  tattr.c_cc[VMIN] = 1;
  tattr.c_cc[VTIME] = 0;
  tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}

void setup_client(void)
{
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    fprintf(stderr, "Error opening socket.\n");
    exit(1);
  }

  server = gethostbyname("localhost");
  if (server == NULL) {
    fprintf(stderr, "Error finding localhost.\n");
    exit(1);
  }

  memset((char *) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr,
    server->h_length);
  serv_addr.sin_port = htons(port_no);

  if (connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    fprintf(stderr, "Error connecting to server.\n");
    exit(1);
  }
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

void *read_from_server(void* ptr)
{
  char c;
  int size = 0, max_size = 1;
  char *buffer = (char*) malloc(max_size);
  int newline_flag = 0;

  while (read(socket_fd, &c, 1) > 0) {

    char c_decrypt = c;
    if (encrypt_flag)
      mdecrypt_generic(td, &c_decrypt, 1);
    if (c_decrypt == '\n')
      newline_flag = 1;

    if (log_flag) {
      buffer[size++] = newline_flag ? '\n' : c;
      if (size >= max_size)
        buffer = (char*) realloc(buffer, max_size+=20);          
      if (newline_flag) {
        char *message = (char*) malloc(50);
        sprintf(message, "RECEIVED %i bytes: ", size-1);
        write(log_fd, message, strlen(message));
        free(message);
        write(log_fd, buffer, size);
        size = 0;
        newline_flag = 0;
      }
    }
    write(STDOUT_FILENO, &c_decrypt, 1);

  }
  close(socket_fd);
  exit(1);
}

void setup_connection(void)
{
  pthread_t thread;
  if (pthread_create(&thread, NULL, &read_from_server, &socket_fd)) {
    fprintf(stderr, "Error creating new thread.\n");
    exit(1);
  }

  if (encrypt_flag)
    setup_mcrypt();

  char c;
  int size = 0, max_size = 1;
  char *buffer = (char*) malloc(max_size);
  int newline_flag = 0;

  while(read(STDIN_FILENO, &c, 1)) {
    if (c == '\004') {
      close(socket_fd);
      exit(0);
    }
    else if (c == '\n' || c == '\r') {
      if (c == '\n')
        newline_flag = 1;
      write(STDOUT_FILENO, "\r\n", 2);
      c = '\n';
      if (encrypt_flag)
        mcrypt_generic(td, &c, 1);
      write(socket_fd, &c, 1);
    }
    else {
      write(STDOUT_FILENO, &c, 1);
      if (encrypt_flag)
        mcrypt_generic(td, &c, 1);
      write(socket_fd, &c, 1);
    }
    if (log_flag) {
      buffer[size++] = newline_flag ? '\n' : c;
      if (size >= max_size)
        buffer = (char*) realloc(buffer, max_size+=20);          
      if (newline_flag) {
        char *message = (char*) malloc(50);
        sprintf(message, "SENT %i bytes: ", size-1);
        write(log_fd, message, strlen(message));
        free(message);
        write(log_fd, buffer, size);
        size = 0;
        newline_flag = 0;
      }
    }
  }
}

int main(int argc, char** argv)
{
  int ret = 0;
  int options_index = 0;

  while (1)
  {
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
      case 'l':
        log_flag = 1;
        log_fd = creat(optarg, 0666);
        break;
      default:
        abort();
    }
  }

  if (log_flag && log_fd < 0) {
    fprintf(stderr, "Error opening log file.\n");
    exit(1); // TODO: check if exit 1 or 2
  }

  if (!port_flag) {
    fprintf(stderr, "No port specified.\n");
    exit(1);
  }

  set_input_mode();
  setup_client();
  setup_connection();
  return 0;
}
