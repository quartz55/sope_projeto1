#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

int main(int argc, char *argv[]){

  if(argc != 2){
    printf("Usage: %s <dir>\n", argv[0]);
    exit(2);
  }


  setbuf(stdout, NULL);

  /* create words file dired sting */
  char word_d[80];
  strcpy(word_d, argv[1]);
  strcat(word_d, "words.txt");

  printf("+-------------------------------\n");
  printf("|Words dired: %s\n", word_d);


  /* open directory given */
  DIR *dp;
  struct dirent *ep;

  dp = opendir (argv[1]);
  if (dp != NULL)
    {
      while ((ep = readdir(dp)) != NULL){
        /* skip current dir, up dir and words.txt */
        if(strcmp(ep->d_name, ".") == 0 ||
           strcmp(ep->d_name, "..") == 0 ||
           strcmp(ep->d_name, "words.txt") == 0){
          continue;
        }

        /* find chapter number */
        char temp[80];
        strcpy(temp, ep->d_name);
        char *pch;
        pch = strtok(temp, ".");

        /* create file dired string */
        char dired[80];
        strcpy(dired, argv[1]);
        strcat(dired, ep->d_name);

        /* -- execute sw -- */
        printf("+-------------------------------\n");
        printf("|Dired: %s\n", dired);
        printf("+-------------------------------\n");
        printf("| executing \"sw %s %s %s\"\n", dired, pch, word_d);
        printf("|");

        if(fork() == 0){ /* -- sw -- */
          execlp("./sw", "./sw", dired, pch, word_d, NULL);
          fprintf(stderr, "**ERROR** sw not executed **ERROR**\n");
          exit(3);
        }
        wait(NULL);
      }
      printf("+-------------------------------\n");
      (void) closedir (dp);
    }
  else
    fprintf(stderr, "Couldn't open the directory");

  exit(0);
}
