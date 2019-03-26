#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

int main(void) {

  /* Our process ID and Session ID */
  pid_t pid, sid;

  /* Fork off the parent process */
  pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  /* If we got a good PID, then
  we can exit the parent process. */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /* Change the file mode mask */
  umask(0);

  /* Open any logs here */

  /* Create a new SID for the child process */
  sid = setsid();
  if (sid < 0) {
    /* Log the failure */
    exit(EXIT_FAILURE);
  }



  /* Change the current working directory */
  if ((chdir("/")) < 0) {
    /* Log the failure */
    exit(EXIT_FAILURE);
  }

  /* Close out the standard file descriptors except STDOUT*/
  close(STDIN_FILENO);
  //close(STDOUT_FILENO);
  close(STDERR_FILENO);

  /* Daemon-specific initialization goes here */

  /* The Big Loop */
  while (1) {
    pid_t pid = fork();
    if (pid < 0)
    printf("Fork zakonczony porazka\n");
    else if (pid > 0) /* Here comes the parent process */
    printf("Fork zakonczony sukcesem\n");
    else { /* Here comes the child process*/
      execlp("echo", "echo", "Daemon jest uruchomiony", (char*)NULL);
      printf("Nie mozna wywolac 'echo'\n");
    }
    sleep(10); /* wait 30 seconds */
  }

  exit(EXIT_SUCCESS);
}
