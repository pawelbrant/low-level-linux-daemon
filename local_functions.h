#ifndef local_functions_h
#define local_functions_h
#endif

#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <utime.h>
#include <fcntl.h>
#include <syslog.h>

char *add_Strings(char *a, char *b);
bool is_Call_Valid(int number, char *params[]);
bool is_Directory(char *path);
off_t get_Size(char *input);
time_t get_Time(char *input);
mode_t get_Permissions(char *input);
void change_Parameters(char *input, char *output);
char *replace_Catalog(char *path, char *input_folder_path, char *output_folder_path);
char *add_To_Path(char *path, char *file_name);
bool file_Comparing(char *path, char *input_folder_path, char *output_folder_path);
void delete_File(char *aux, char *input_folder_path, char *output_folder_path, bool recursive);
void copy_File(char *input, char *output);
void Login(int sig);
void browse_Folder(char *aux, char *input_folder_path, char *output_folder_path, bool recursive, int size_of_file);
