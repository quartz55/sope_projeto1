#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define READ 0
#define WRITE 1

int main(int argc, char *argv[]){

  if(argc != 2){
    printf("Usage: %s <outdir>\n", argv[0]);
    exit(2);
  }


  setbuf(stdout, NULL);

  /* create temp dired sting */
  char *temp_d = "./temp/";

  /*
   * Creating pipe
   */
  int readSortPipe[2];
  int sortParsePipe[2];

  if(pipe(readSortPipe) == -1){
    fprintf(stderr, "Couldn't pipe\n");
  }

  if(pipe(sortParsePipe) == -1){
    fprintf(stderr, "Couldn't pipe\n");
  }

  /* open directory given */
  DIR *dp;
  struct dirent *ep;

  dp = opendir (temp_d);
  if (dp != NULL)
    {
      while ((ep = readdir(dp)) != NULL){
        /* skip current dir, up dir and words.txt */
        if(strcmp(ep->d_name, ".") == 0 ||
           strcmp(ep->d_name, "..") == 0 ){
          continue;
        }

        /* find chapter number */
        char temp[80];
        strcpy(temp, ep->d_name);

        /* create file dired string */
        char dired[80];
        strcpy(dired, "./temp/");
        strcat(dired, ep->d_name);

        /* -- write files to pipe -- */
        printf("+-------------------------------\n");
        printf("|Dired: %s\n", dired);
        printf("+-------------------------------\n");
        printf("| printing file contents to pipe\n");

        if(fork() == 0){
          dup2(readSortPipe[WRITE], STDOUT_FILENO);
          close(readSortPipe[WRITE]);
          close(readSortPipe[READ]);
          close(sortParsePipe[WRITE]);
          close(sortParsePipe[READ]);
          execlp("cat", "cat", dired, NULL);
          fprintf(stderr, "**ERROR** cat not executed **ERROR**\n");
          exit(3);
        }
        wait(NULL);
      }
      printf("+-------------------------------\n");
      (void) closedir (dp);
    }
  else
    fprintf(stderr, "Couldn't open the directory");

  /* -- execute sort -- */
  if(fork() == 0){
    dup2(readSortPipe[READ], STDIN_FILENO);
    dup2(sortParsePipe[WRITE], STDOUT_FILENO);
    close(readSortPipe[WRITE]);
    close(readSortPipe[WRITE]);
    close(sortParsePipe[WRITE]);
    close(sortParsePipe[READ]);
    execlp("sort", "sort", "-t:", "-k1,1d", "-k2,2V", NULL);
    fprintf(stderr, "**ERROR** sort not executed **ERROR**\n");
    exit(3);
  }
  close(readSortPipe[READ]);
  close(readSortPipe[WRITE]);
  wait(NULL);

  if(fork() == 0){
    close(sortParsePipe[WRITE]);
    dup2(sortParsePipe[READ], STDIN_FILENO);

    char buf[BUFSIZ];
    char currentWord[80];
    int firstTime = 1;

    char index_d[80];
    strcpy(index_d, argv[1]);
    strcat(index_d, "index.txt");

    FILE *indexFile = fopen (index_d, "w");
    fprintf(indexFile, "INDEX\n");

    while(fgets(buf, sizeof(buf), stdin) != NULL){
      if (buf[strlen(buf)-1] == '\n') {
        char str[80];
        strcpy(str, buf);

        char *word;
        word = strtok(str, ":");

        strcpy(str, buf);
        char *number;
        number = strtok(str, ":");
        number = strtok(NULL, "#");

        if(strcmp(word, currentWord) == 0){
          if(firstTime){
            fprintf(indexFile, "%s: %s", word, number);
            firstTime = 0;
          }
          fprintf(indexFile, ", %s", number);
        }
        else{
          fprintf(indexFile, "\n\n%s: %s", word, number);
          strcpy(currentWord, word);
          firstTime = 0;
        }
      }
    }

    fprintf(indexFile, "\n");
    fclose(indexFile);

    close(sortParsePipe[READ]);
    exit(0);
  }
  wait(NULL);
  close(sortParsePipe[WRITE]);
  close(sortParsePipe[READ]);

  exit(0);
}
