#include "local_functions.h"


//checks whether main app has been called with valid parameters
bool is_Call_Valid(int number, char *params[])
{
  if(number<3)
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

//may be modified with following function to make one returning stat type
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


char *replace_Catalog(char *aux, char *input_folder_path, char *output_folder_path)
{
  char *path = aux + strlen(input_folder_path);
  char *new_path = malloc(strlen(output_folder_path)+strlen(path)+1);
  strcpy(new_path,output_folder_path);
  strcat(new_path,path);
  return new_path;
}

char *add_To_Path(char *path, char *file_name)
{
  int len = strlen(path) + strlen(file_name) + 1;
  char *new_path = (char *)malloc(len * sizeof(char));
  strcpy(new_path, path);
  strcat(new_path,"/");
  strcat(new_path,file_name);
  return new_path;
}

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
    if (strcmp(file->d_name, file_name) == 0)
      if (!(get_Time(old_path) - get_Time(new_path)))
        check = 0;
  }
  closedir(catalog_path);
  return check;
}

void delete_File(char *aux, char *input_folder_path, char *output_folder_path, bool recursive)
{
  struct dirent *file;
  DIR *catalog_path, *tmp;
  catalog_path = opendir(aux);
  while(file = readdir(catalog_path))
  {
    if((file->d_type)==DT_DIR)
    {
      if(recursive)
      {
        if(!(strcmp(file->d_name,".") == 0 || strcmp(file->d_name, "..") == 0))
        {
          char *new_path = add_To_Path(aux,file->d_name);
          delete_File(new_path,output_folder_path,input_folder_path,recursive);
          if(!(tmp=opendir(add_To_Path(output_folder_path, file -> d_name))))
          {
            syslog(LOG_INFO, "Catalog %s deleted.", new_path);
            remove(new_path);
          }
          else
            closedir(tmp);
        }
      }

    }
    else
    {
      char *new_path = add_To_Path(aux,file->d_name);
      if(access(add_To_Path(output_folder_path, file -> d_name),F_OK) == -1)
      {
        syslog(LOG_INFO, "File %s deleted.", new_path);
        remove(new_path);
      }
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
  path = opendir(input_folder_path);
  while(file = readdir(path))
  {
    char *file_name = file -> d_name;
    char *old_path = add_To_Path(input_folder_path, file_name);
    char *new_path = add_To_Path(output_folder_path, file_name);
    if((file->d_type)==DT_REG)
    {
      if(file_Comparing(file_name, input_folder_path, output_folder_path))
      {
        if(get_Size(old_path)>size_of_file)
          copy_File_By_Mapping(old_path, new_path);
        else
          copy_File(old_path, new_path);
      }
    }
    else if((file -> d_type) == DT_DIR && recursive)
    {
      if((strcmp(file_name,".") != 0) && (strcmp(file_name,"..") != 0))
      {
        if(!(tmp = opendir(new_path)))
        {
          mkdir(new_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
          syslog(LOG_INFO, "Created folder %s", new_path);
        }
        else
        closedir(tmp);
        browse_Folder(old_path, new_path, recursive, size_of_file);
      }
    }
  }
  closedir(path);
}
