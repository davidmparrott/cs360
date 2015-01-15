/*
  David Parrott
  11239947
  CS 360 Assignment 3
  readable.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/*
  checkDir(char* path) is designed to be called recursively to 
  navigate through directory trees. Information from the
  dirent returned by opendir(path) is used to check if an 
  entry is a symbolic link and ignore it, or if it is a
  regular file, which is output to stdout or a directory
  which is also output to stdout but also then passed as a
  new path to checkDir()
*/
void checkDir(char * path){
  DIR * dir;
  dir = opendir(path);
  
  if(dir == NULL){
    fprintf(stderr, "Failed to open directory '%s'\n", path);
    exit(1);
  }
  while(1){
    struct dirent * entry;
    char * name;
    entry = readdir(dir);
    if(!entry)
      break;
    name = entry->d_name;
/*
  ignoring symbolic links to avoid endles loops
*/
    if(entry->d_type == 10){
      printf("Ignoring Sybolic Link: %s\n", name);
      continue;
    }
/*
  checking for . or .. 
*/
    if((strcmp(name, ".") == 0) || (strcmp(name, "..") == 0))
      continue;
/*
  if type == 8 it is a regular file. Print out full path
  if type == 4 it is a directory. Print out full path
  and recursively call checkDir()
*/
    if(entry->d_type == 8)
      printf("%s/%s\n", path, name);
    else if(entry->d_type == 4){
      printf("%s/%s/\n", path, name);
/*
  the path must be preserved in order to continue to
  navigate through the directories
*/
      char* oldPath = strdup(path);
      char* newPath = path;
      strcat(newPath, "/");
      strcat(newPath, name);
      checkDir(newPath);
      path = oldPath;
    }
  }
  closedir(dir);	
}

int main(int argc, char* argv[]){
  char * path;
  if((argc == 2) && (path = strdup(argv[1])))
    printf("%s/\n", path);
  else{
    path = getcwd(NULL, 256);
    printf("%s/\n", path);
  }    
  checkDir(path);
  return(0);
}
