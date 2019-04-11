#include "local_functions.h"

int main(int argc, char *argv[])
{
  // opening log
  openlog("low-level-linux-daemon", LOG_PID|LOG_CONS, LOG_USER);
  syslog(LOG_NOTICE, "Starting daemon");

  // checking if arguments point to directories
  if (!(is_Call_Valid(argc, argv)))
  {
    exit(EXIT_FAILURE);
  }

  //saving paths from parameters
  char *source = argv[1];
  char *destination = argv[2];

  //setting default option values
  int sleep_time = 300, max_size = 1024;
  bool recursive = false;

  // modifying option values from given parameters
  char choice;
  while ((choice = getopt(argc, argv, "t:s:R")) != -1)
  {
    switch(choice)
    {
      case 't': //argument z nowa wartoscia spania demona
        sleep_time = atoi(optarg);
        break;

      case 'R':
        recursive = true;
        break;

      case 's':
        max_size = atoi(optarg);
        break;
    }
  }
  if (sleep_time == 0 || max_size == 0)
  {
    syslog(LOG_ERR, "Invalid option values. Daemon shutting down");
    exit(EXIT_FAILURE);
  }
  //creating new process ID and session ID variables
  pid_t pid, sid;

  //forking off the parent process
  pid = fork();
  if (pid < 0)
  {
    syslog(LOG_ERR, "Unable to fork");
    exit(EXIT_FAILURE);
  }

  //exiting the parent process
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  // changing the file mode mask
  umask(0);

  // creating a new SID for the child process
  sid = setsid();
  if (sid < 0)
  {
    syslog(LOG_ERR, "Unable to get session ID");
    exit(EXIT_FAILURE);
  }

  // changing the current working directory
  if ((chdir("/")) < 0) {
    /* Log the failure */
    exit(EXIT_FAILURE);
  }

  // closing out the standard file descriptors
  close(STDIN_FILENO);
  //close(STDOUT_FILENO);
  close(STDERR_FILENO);

  syslog(LOG_NOTICE,
    "Daemon starts work with given parameters:\t"\
    "Source directory: %s\t"\
    "Destination directory: %s\t"\
    "Sleep time: %d seconds\t"\
    "Recursive: %d\t"\
    "Max file size for standard copying: %d",
    source, destination, sleep_time, recursive, max_size
  );

  while (1) {
    delete_File(destination, source, destination, recursive);
    browse_Folder(source, source, destination, recursive, max_size);
    syslog(LOG_NOTICE, "Entering sleep mode");
    if(sleep(sleep_time) == 0)
      syslog(LOG_NOTICE, "Wake up");
  }
  closelog();
  exit(EXIT_SUCCESS);
}
