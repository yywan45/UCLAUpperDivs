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

/* Use this variable to remember original terminal attributes. */

struct termios saved_attributes;
pid_t child_pid;
int shell_flag = 0;
int pipe_from_shell[2];
int pipe_to_shell[2];
pid_t child_pid = -1;
int sig_number = 0;

/* to store options for getopt */
static struct option long_options[] = {
  { "shell", no_argument, 0, 's' }
};

void
reset_input_mode (void)
{
  tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
}

void
set_input_mode (void)
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

void exit_handler(int exitnum)
{
  int child_status;
  waitpid(child_pid, &child_status, 0);

  if (WIFEXITED(child_status)) {
    printf("child process' exit status: %d\n", WEXITSTATUS(child_status));
  }
  else
    fprintf(stderr, "Exit with signal number: %d\n", sig_number);
  exit(exitnum);
}

void thread()
{
  char c;
  while (read(pipe_from_shell[0], &c, 1) > 0)
    write(STDOUT_FILENO, &c, 1);
  exit_handler(1);
}

void sigpipe_handler() {
  exit_handler(1);
}

void shell()
{
  
  if (pipe(pipe_from_shell) == -1 || pipe(pipe_to_shell) == -1) {
    fprintf(stderr, "pipe() failed!\n");
    exit(1);
  }

  child_pid = fork();

  if (child_pid > 0) { // parent process
    close(pipe_to_shell[0]);
    close(pipe_from_shell[1]);

    pthread_t my_thread;
    if (pthread_create(&my_thread, NULL, (void*)thread, NULL)) {
      fprintf(stderr, "Error creating parent's read thread.\n");
      exit(1);
    }

    char c;
    while (read(STDIN_FILENO, &c, 1) > 0) {
      if (c == '\004') /* C-d */ {
	kill(child_pid, SIGHUP);
	sig_number = SIGHUP;
	close(pipe_to_shell[1]);
	close(pipe_from_shell[0]);
	exit_handler(0);
      }
      else if (c == '\003') /* C-c */ {
	kill(child_pid, SIGINT);
	sig_number = SIGINT;
      }
      else if (c == '\n' || c == '\r') {
	write(STDOUT_FILENO, "\r\n", 2);
	write(pipe_to_shell[1], "\n", 1);
      }
      else {
	write(STDOUT_FILENO, &c, 1);
	write(pipe_to_shell[1],&c, 1);
      }
    }
    
  } else if (child_pid == 0) { // child process                                          
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
      fprintf(stderr, "execvp() failed!\n");
      exit_handler(1);
    }
  } else { // fork() failed!                                                             
    fprintf(stderr, "fork() failed!\n");
    exit(1);
  }
  exit_handler(1);
}

void normal(void)
{
  char c;
  while (1)
  {
    read(STDIN_FILENO, &c, 1);
    
    if (c ==  '\r' || c == '\n')
      write(STDIN_FILENO, "\r\n", 2);
    else if (c == '\004') // C-d
      break;
    else
      write(STDIN_FILENO, &c, 1);
  }
}

int main(int argc, char** argv)
{
  signal(SIGPIPE, sigpipe_handler);
  
  int ret = 0;
  int options_index = 0;

  while (1)
  {
    ret = getopt_long(argc, argv, "s", long_options, &options_index);
    
    if (ret == -1)
      break;
    
    switch (ret)
    {
      case 's':
        shell_flag = 1;
        break; 
      default:
	abort();
    }
  }

  set_input_mode();
  
  if (shell_flag)
    shell();
  else
    normal();
   
  return 0;
}
