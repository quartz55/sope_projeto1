#include <libgen.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[]){

  if(argc != 2){
    printf("Usage: %s <filesdir>\n", argv[0]);
    exit(2);
  }

  setbuf(stdout, NULL);

  /* Create path names */
  char real_d_files[256];
  realpath(argv[1], real_d_files);
  strcat(real_d_files, "/");

  char real_d_bin[256];
  realpath(argv[0], real_d_bin);
  strcat(real_d_bin, "/");

  char *base_d = dirname(real_d_bin);
  strcat(base_d, "/");

  char sw_path[256];
  strcpy(sw_path, base_d);
  strcat(sw_path, "sw");

  char csc_path[256];
  strcpy(csc_path, base_d);
  strcat(csc_path, "csc");

  char temp_d[256];
  strcpy(temp_d, base_d);
  strcat(temp_d, "temp/");

  /* create words file dired sting */
  char word_d[256];
  strcpy(word_d, real_d_files);
  strcat(word_d, "words.txt");

  printf("+-------------------------------\n");
  printf("|Words dired: %s\n", word_d);

  /* open directory given */
  DIR *dp;
  struct dirent *ep;

  dp = opendir (real_d_files);
  if (dp != NULL){
    while ((ep = readdir(dp)) != NULL){
      /* skip current dir, up dir and words.txt */
      if(strcmp(ep->d_name, ".") == 0 ||
         strcmp(ep->d_name, "..") == 0 ||
         strcmp(ep->d_name, "words.txt") == 0 ||
         strcmp(ep->d_name, "index.txt") == 0){
        continue;
      }

      /* find chapter number */
      char temp[256];
      strcpy(temp, ep->d_name);
      char *pch;
      pch = strtok(temp, ".");

      /* create file dired string */
      char dired[256];
      strcpy(dired, real_d_files);
      strcat(dired, ep->d_name);

      /* -- execute sw -- */
      printf("+-------------------------------\n");
      printf("|Dired: %s\n", dired);
      printf("+-------------------------------\n");
      /* printf("| executing \"sw %s %s %s\"\n", dired, pch, word_d); */
      printf("|");

      if(fork() == 0){ /* -- sw -- */
        execlp(sw_path, "sw", dired, pch, word_d, base_d, NULL);
        fprintf(stderr, "**ERROR** sw not executed **ERROR**\n");
        exit(3);
      }
      wait(NULL);
    }
    printf("+-------------------------------\n");
    (void) closedir (dp);
  }
  else{
    fprintf(stderr, "Couldn't open the directory");
    exit(1);
  }

  if(fork() == 0){ /* --- csc --- */
    execlp(csc_path, csc_path, real_d_files, temp_d, NULL);
    fprintf(stderr, "**ERROR** csc not executed **ERROR**\n");
    exit(3);
  }
  wait(NULL);

  /* Remove temp files and directory */
  dp = opendir (temp_d);
  if (dp != NULL){
    while ((ep = readdir(dp)) != NULL){
      /* create file dired string */
      char dired[256];
      strcpy(dired, temp_d);
      strcat(dired, ep->d_name);

      remove(dired);
    }
    (void) closedir (dp);
  }
  else{
    fprintf(stderr, "Couldn't open the directory");
    exit(1);
  }
  rmdir(temp_d);

  exit(0);
}
