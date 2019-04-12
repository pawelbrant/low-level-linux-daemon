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
#include <errno.h>

int is_Call_Valid(int number_of_parameters, char *parameters[]);
int get_Type(char *path);
off_t get_Size(char *input);
time_t get_Time(char *input);
mode_t get_Permissions(char *input);
int change_Parameters(char *input, char *output);
char *add_To_Path(char *path, char *file_name);
int are_Same(char *file_name, char *input_folder_path, char *output_folder_path);
int delete_File(char *input_folder_path, char *output_folder_path, bool recursive);
void copy_File(char *input, char *output);
void copy_File_By_Mapping(char *input, char *output);
void Login(int sig);
void browse_Folder(char *input_folder_path, char *output_folder_path, bool recursive, int size_of_file);
