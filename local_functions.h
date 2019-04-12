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

bool is_Call_Valid(int number_of_parameters, char *parameters[]);
bool is_Directory(char *path);
off_t get_Size(char *path);
time_t get_Time(char *path);
mode_t get_Permissions(char *path);
void change_Parameters(char *input, char *output);
char *add_To_Path(char *path, char *file_name);
bool are_Same(char *file_name, char *input_folder_path, char *output_folder_path);
void check_Existing(char *input_folder_path, char *output_folder_path, bool recursive);
void delete_Folder(char *path);
void copy_File(char *input, char *output);
void copy_File_By_Mapping(char *input, char *output);
void browse_Folder(char *input_folder_path, char *output_folder_path, bool recursive, int size_of_file);
