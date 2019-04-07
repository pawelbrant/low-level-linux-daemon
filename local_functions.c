#include "local_functions.h"

bool is_Call_Valid(int number, char *params[])
{
  if(number<3)
  {
    printf("Not enought input arguments");
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

off_t get_Size(char *input)
{
  struct stat size;
  if(stat(input, &size) == 0)
  {
    return size.st_size;
  }
  return -1;
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
  printf("\n\n\n");
  printf("replace_Catalog path %s\n",path);
  printf("replace_Catalog new_path %s\n",new_path);
  printf("\n\n\n");
  return new_path;
}

char *add_To_Path(char *path, char *file_name)
{
  char *new_path = malloc(strlen(path)+2+strlen(file_name));
  strcpy(new_path, path);
  strcat(new_path,"/");
  strcat(new_path,file_name);
  new_path[strlen(path)+1+strlen(file_name)]='\0';
  printf("\n\n\n");
  printf("add_to_path file_name %s\n",path);
  printf("\n\n\n");
  return new_path;
}

bool file_Comparing(char *path, char *input_folder_path, char *output_folder_path)
{
  bool result = 0;
  char *path_name = path+strlen(input_folder_path);
  char *search = malloc(strlen(path_name));
  char *new_path = replace_Catalog(path, input_folder_path, output_folder_path);
  int i = strlen(new_path);
  for(i; new_path[i] != '/'; i--);
  strcpy(search,new_path+i+1);
  new_path[i]='\0';
  struct dirent *file;
  DIR *catalog_path;
  catalog_path = opendir(new_path);
  while(file = readdir(catalog_path))
  {
    if(strcmp(file->d_name, search)==0)
    {
      free(search);
      if((file->d_type)==DT_DIR) return 0;
      else return 1;
    }
    else result=1;
  }
  closedir(catalog_path);
  return result;
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
        if(!(strcmp(file->d_name,".") == 0 || strcmp(file->d_name, "..")==0))
        {
          char *new_path = add_To_Path(aux,file->d_name);
          delete_File(new_path,output_folder_path,input_folder_path,recursive);
          if(!(tmp=opendir(replace_Catalog(new_path,input_folder_path,output_folder_path))))
          {
            syslog(LOG_INFO, "Catalog %s deleted.", new_path);
            remove(new_path);
          }
          else closedir(tmp);
        }
      }

    }
    else
    {
      char *new_path = add_To_Path(aux,file->d_name);
      if(access(replace_Catalog(new_path,output_folder_path,input_folder_path),F_OK) == -1)
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
  syslog(LOG_INFO, "Works up to this place");
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
    syslog(LOG_INFO, "Success.");
  }
}

void copy_File_By_Mapping(char *input, char *output)
{
  syslog(LOG_INFO, "Works up to this place2");
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

void browse_Folder(char *aux, char *input_folder_path, char *output_folder_path, bool recursive, int size_of_file)
{
  printf("%d\n", size_of_file );
  printf("We are in %s\n", aux);
  struct dirent *file;
  DIR *path, *tmp;
  path = opendir(aux);
  char *new_path;
  while(file = readdir(path))
  {
    printf("%s  \n", file->d_name);
    if((file->d_type)==DT_DIR)
    {
      syslog(LOG_INFO, "WORKS.");
      if(recursive)
      {
        if(!(strcmp(file->d_name,".")==0 || strcmp(file->d_name,"..")==0))
        {
          char *folder_path = replace_Catalog(add_To_Path(aux, file->d_name),input_folder_path, output_folder_path);
          if(!(tmp = opendir(folder_path)))
          {
            syslog(LOG_INFO, "Created folder %s", folder_path);
            mkdir(folder_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
          }
          else closedir(tmp);
          new_path = add_To_Path(aux, file->d_name);
          browse_Folder(new_path, input_folder_path,output_folder_path, recursive, size_of_file);
        }
      }
    }
    else if((file->d_type)==DT_REG)
    {
      syslog(LOG_INFO, "Czy działa poprawnie?");
      new_path = add_To_Path(aux, file->d_name);
      int i = file_Comparing(new_path, input_folder_path, output_folder_path);
      printf("\n\nAAAAAAAAAAAAAAAA\n%d\n\n",i);
      if(i==1)
      {
        syslog(LOG_INFO, "teraz działa dobrze");
        if(get_Size(new_path)>size_of_file)
        {
          copy_File_By_Mapping(new_path, replace_Catalog(new_path, input_folder_path, output_folder_path));
        }
        else
        {
          copy_File(new_path, replace_Catalog(new_path, input_folder_path, output_folder_path));
        }
      }
    }
  }
  closedir(path);
}
