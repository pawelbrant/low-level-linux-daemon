#include "local_functions.h"

int main(int argc, char *argv[])
{
  printf("%s %s\n", "Argv 1 to", argv[1]);
  printf("%s %s\n", "Argv 2 to", argv[2]);
  // opening log
  openlog("low-level-linux-daemon", LOG_PID|LOG_CONS, LOG_USER);
  syslog(LOG_NOTICE, "Starting daemon");

  // checking if arguments point to directories
  if (!(is_Call_Valid(argc, argv)))
  {
    exit(EXIT_FAILURE);
  }

  int sleep_time = 300, max_size = 999999, choice;
  bool recursive = false;

  // checking option values
  while (getopt(argc, argv, "t:size:R") != -1)
  {
    switch(choice)
    {
      case 't': //argument z nowa wartoscia spania demona
        sleep_time = atoi(optarg);
        break;

      case 'R':
        recursive = true;
        break;

      case 'm':
        max_size = atoi(optarg);
        break;
    }
  }

  /* Our process ID and Session ID */
  pid_t pid, sid;

  /* Fork off the parent process */
  pid = fork();
  if (pid < 0)
  {
    syslog(LOG_ERR, "Unable to fork");
    exit(EXIT_FAILURE);
  }
  /* If we got a good PID, then
  we can exit the parent process. */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /* Change the file mode mask */
  umask(0);

  /* Create a new SID for the child process */
  sid = setsid();
  if (sid < 0)
  {
    syslog(LOG_ERR, "Unable to get session ID");
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
    printf("%s %s\n", "Argv 1 to", argv[1]);
    printf("%s %s\n", "Argv 2 to", argv[2]);
    delete_File(argv[2], argv[1], argv[2], recursive);
    browse_Folder(argv[1], argv[1], argv[2], recursive, max_size);
    syslog(LOG_NOTICE, "Entering sleep mode");
    if(sleep(sleep_time) == 0)
      syslog(LOG_NOTICE, "Wake up");
    // pid_t pid = fork();
    // if (pid < 0)
    // printf("Fork zakonczony porazka\n");
    // else if (pid > 0) /* Here comes the parent process */
    // printf("Fork zakonczony sukcesem\n");
    // else { /* Here comes the child process*/
    //   execlp("echo", "echo", "Daemon jest uruchomiony", (char*)NULL);
    //   printf("Nie mozna wywolac 'echo'\n");
    // }
  }
  closelog();
  exit(EXIT_SUCCESS);
}
