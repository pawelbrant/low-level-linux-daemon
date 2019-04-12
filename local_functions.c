#include "local_functions.h"


//checks whether main app has been called with valid parameters
//takes number of program arguments, list of those arguments
bool is_Call_Valid(int number, char *params[])
{
  if(number < 3)
  {
    printf("Not enought input arguments\n");
    syslog(LOG_ERR, "Not enought input arguments");
    return false;
  }
  if(is_Directory(params[1]))
  {
    if(is_Directory(params[2]))
      return true;
    else
    syslog(LOG_ERR, "Second argument is not a directory");
    return false;
  }
  syslog(LOG_ERR, "First argument is not a directory");
  return false;
}

//checks whether given path points a directory
//takes path
bool is_Directory(char *path)
{
  struct stat s;
  if(stat(path, &s) == 0)
  {
    if(s.st_mode & S_IFDIR)
      return true;
    return false;
  }
  syslog(LOG_ERR, "Unable to get stat");
  return false;
}

//returns size of file
//takes path
off_t get_Size(char *input)
{
  struct stat size;
  if(stat(input, &size) == -1)
  {
    syslog(LOG_ERR, "Can't get size of file %s", input);
    exit(EXIT_FAILURE);
  }
  return size.st_size;
}

//returns modification date of a file
//takes path
time_t get_Time(char *input)
{
  struct stat time;
  if(stat(input, &time) == -1)
  {
    syslog(LOG_ERR, "Can't get modification data for %s", input);
    exit(EXIT_FAILURE);
  }
  return time.st_mtime;
}

//returns permissions of a file
//takes paths
//currently unused
mode_t get_Permissions(char *input)
{
  struct stat permissions;
  if(stat(input, &permissions) == -1)
  {
    syslog(LOG_ERR, "Can't get permissions for %s", input);
    exit(EXIT_FAILURE);
  }
  return permissions.st_mode;
}

//changes modification date of a file to match another file
//takes two paths
void change_Parameters(char *input, char *output)
{
  struct utimbuf time;
  time.actime = 0;
  time.modtime = get_Time(input);
  if(utime(output, &time) != 0)
  {
    syslog(LOG_ERR, "Error with modification data.");
    exit(EXIT_FAILURE);
  }
}

//creates a valid path to a file from path to a directory and filename
//takes path to directory and filename
char *add_To_Path(char *path, char *file_name)
{
  int len = strlen(path) + strlen(file_name) + 1;
  char *new_path = (char *)malloc(len * sizeof(char));
  strcpy(new_path, path);
  strcat(new_path,"/");
  strcat(new_path,file_name);
  return new_path;
}

//checks if there is a maching file or directory in given folder
//takes filename and path to a directory
bool file_Comparing(char *file_name, char *input_folder_path, char *output_folder_path)
{
  char *old_path = add_To_Path(input_folder_path, file_name);
  char *new_path = add_To_Path(output_folder_path, file_name);
  bool check = 1;
  struct dirent *file;
  DIR *catalog_path;
  catalog_path = opendir(output_folder_path);
  while(file = readdir(catalog_path))
  {
    if(strcmp(file->d_name, file_name) == 0)
    {
      if((file -> d_type) == DT_REG)
      {
        if(!(get_Time(old_path) - get_Time(new_path)))
          check = 0;
      }
      else
      {
        check = 0;
      }
    }
  }
  free(old_path);
  free(new_path);
  closedir(catalog_path);
  return check;
}

void delete_File(char *input_folder_path, char *output_folder_path, bool recursive)
{
  struct dirent *file, *content;
  DIR *catalog_path, *temp;
  char *old_path;
  char *new_path;
  catalog_path = opendir(output_folder_path);
  // printf("%s\n", "1");
  while(file = readdir(catalog_path))
  {
    if((file -> d_type) == DT_DIR && recursive)
    {
      if(!(strcmp(file -> d_name,".") == 0 || strcmp(file -> d_name, "..") == 0))
      {
        if(file_Comparing(file -> d_name, output_folder_path, input_folder_path))
        {
          char *old_path = add_To_Path(input_folder_path, file -> d_name);
          char *new_path = add_To_Path(output_folder_path, file -> d_name);
          temp = opendir(new_path);
          while(content = readdir(temp))
          {
            if(access(new_path, F_OK) == 0)
            {
              remove(add_To_Path(new_path, content -> d_name));
            }
          }
          closedir(temp);
          remove(new_path);
          syslog(LOG_INFO, "Catalog %s deleted.", new_path);
        }
        else
        {
          char *old_path = add_To_Path(input_folder_path, file -> d_name);
          char *new_path = add_To_Path(output_folder_path, file -> d_name);
          delete_File(old_path, new_path, recursive);
        }
      }
    }
    else if((file -> d_type) = DT_REG)
    {
      // printf("%s\n", "2");
      char *old_path = add_To_Path(input_folder_path, file -> d_name);
      char *new_path = add_To_Path(output_folder_path, file -> d_name);
      if(access(new_path, F_OK) == 0 && file_Comparing(file -> d_name, output_folder_path, input_folder_path))
      {
        remove(new_path);
        syslog(LOG_INFO, "File %s deleted.", new_path);
      }
      free(old_path);
      free(new_path);
    }
  }
  closedir(catalog_path);
}

void copy_File(char *input, char *output)
{
  char buffer[16];
  int input_file, output_file, read_input, read_output;
  input_file = open(input, O_RDONLY);
  output_file = open(output, O_CREAT | O_WRONLY | O_TRUNC, 0644);

  if(input_file == -1 || output_file == -1)
  {
    syslog(LOG_ERR, "Can't open a file.");
    exit(EXIT_FAILURE);
  }
  while((read_input = read(input_file, buffer, sizeof(buffer)))>0)
  {
    read_output = write(output_file, buffer, (ssize_t) read_input);
    if(read_output != read_input)
    {
      perror("Error.");
      exit(EXIT_FAILURE);
    }
    close(input_file);
    close(output_file);
    change_Parameters(input,output);
    syslog(LOG_INFO, "Coppied in normal mode: File %s to %s.", input, output);
  }
}

void copy_File_By_Mapping(char *input, char *output)
{
  int size = get_Size(input);
  int input_file = open(input, O_RDONLY);
  int output_file = open(output, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if(input_file == -1 || output_file == -1)
  {
    syslog(LOG_ERR, "Can't open a file.");
    exit(EXIT_FAILURE);
  }
  char *map = (char*) mmap (0, size, PROT_READ, MAP_SHARED | MAP_FILE, input_file, 0);
  write(output_file, map, size);
  close(input_file);
  close(output_file);
  munmap(map, size);
  change_Parameters(input, output);
  syslog(LOG_INFO, "Coppied using mapping: File %s to %s.", input, output);
}

void Login(int sig)
{
  syslog(LOG_INFO, "Waking up.");
}

void browse_Folder(char *input_folder_path, char *output_folder_path, bool recursive, int size_of_file)
{
  struct dirent *file;
  DIR *path, *tmp;
  char *file_name;
  char *old_path;
  char *new_path;
  path = opendir(input_folder_path);
  while(file = readdir(path))
  {
    if((file -> d_type) == DT_REG)
    {
      old_path = add_To_Path(input_folder_path, file -> d_name);
      new_path = add_To_Path(output_folder_path, file -> d_name);
      if(file_Comparing(file -> d_name, input_folder_path, output_folder_path))

        if(get_Size(old_path)>size_of_file)
          copy_File_By_Mapping(old_path, new_path);
        else
          copy_File(old_path, new_path);
      free(old_path);
      free(new_path);
    }
    else if((file -> d_type) == DT_DIR && recursive)
    {
      if(!(strcmp(file -> d_name,".") == 0 || strcmp(file -> d_name,"..") == 0))
      {
        new_path = add_To_Path(output_folder_path, file -> d_name);
        if(!(tmp = opendir(new_path)))
        {
          mkdir(new_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
          syslog(LOG_INFO, "Created folder %s", new_path);
        }
        else
          closedir(tmp);
        free(new_path);
        old_path = add_To_Path(input_folder_path, file -> d_name);
        browse_Folder(old_path, new_path, recursive, size_of_file);
        free(old_path);
      }
    }
  }
  closedir(path);
}
