#include "local_functions.h"

volatile bool wake;
volatile bool stop;

//signal handler
void handler(int sig)
{
  wake = true;
  if(sig == SIGTERM)
    stop = true;
}

int main(int argc, char *argv[])
{
  // opening log
  openlog("low-level-linux-daemon", LOG_PID|LOG_CONS, LOG_USER);
  syslog(LOG_INFO, "Starting daemon");

  // checking if arguments point to directories
  if (!(is_Call_Valid(argc, argv)))
  {
    exit(EXIT_FAILURE);
  }

  //saving paths from parameters
  char *source = argv[1];
  char *destination = argv[2];

  //setting default option values
  int sleep_time = 300;
  long long max_size = 1024;
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
    syslog(LOG_ERR, "Invalid option parameters values");
    syslog(LOG_NOTICE, "Daemon shutting down");
    exit(EXIT_FAILURE);
  }
  //creating new process ID and session ID variables
  pid_t pid, sid;

  //forking off the parent process
  pid = fork();
  if (pid == -1)
  {
    syslog(LOG_ERR, "%s", strerror(errno));
    syslog(LOG_NOTICE, "Daemon shutting down");
    exit(EXIT_FAILURE);
  }

  //exiting the parent process
  if (pid > 0)
  {
    exit(EXIT_SUCCESS);
  }

  // changing the file mode mask
  umask(0);

  // creating a new SID for the child process
  sid = setsid();
  if (sid == -1)
  {
    syslog(LOG_ERR, "%s", strerror(errno));
    syslog(LOG_NOTICE, "Daemon shutting down");
    exit(EXIT_FAILURE);
  }

  // closing the standard file descriptors
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  syslog(LOG_NOTICE,
    "Daemon starts work with given parameters:  "\
    "Source directory: %s  "\
    "Destination directory: %s  "\
    "Sleep time: %d seconds  "\
    "Recursive: %d  "\
    "Max file size for standard copying: %llu",
    source, destination, sleep_time, recursive, max_size
  );

  //handling SIGUSR1 signal
  signal(SIGUSR1, handler);
  signal(SIGTERM, handler);

  while (1)
  {
    check_Existing(source, destination, recursive);
    browse_Folder(source, destination, recursive, max_size);
    syslog(LOG_NOTICE, "Entering sleep mode");
    if(sleep(sleep_time) == false || wake == true)
    {
      if(wake == true)
      {
        if(stop == true)
          break;
        syslog(LOG_NOTICE, "Woken up by user");
        wake = false;
      }
      else
      {
        syslog(LOG_NOTICE, "Woken up by itself");
      }
    }
  }
  syslog(LOG_NOTICE, "Daemon shutting down");
  closelog();
  return 0;
}
