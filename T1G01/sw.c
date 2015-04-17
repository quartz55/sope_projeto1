#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define READ 0
#define WRITE 1

int main (int argc, char *argv[])
{

  if(argc != 5){
    printf("Usage: %s <filename>.txt <chapter> <words>.txt <outdir>\n", argv[0]);
    exit(1);
  }

  FILE *wordsFile;
  /*
   * Open words.txt file
   */
  wordsFile = fopen(argv[3], "r");
  if(!wordsFile){
    fprintf(stderr, "%s could not be found\n", argv[3]);
    exit(2);
  }

  char out_d[256];
  strcpy(out_d, argv[4]);
  strcat(out_d, "temp/");
  mkdir(out_d, 0777);

  /* Open temp file "<chapter>_temp.txt"to write to */
  char tempFileName[80];
  strcpy(tempFileName, out_d);
  strcat(tempFileName, argv[2]);
  strcat(tempFileName, "_temp.txt");
  FILE *tempF  = fopen(tempFileName, "w");

  fprintf(stdout, "\tsw - Reading '%s' and writing chapter %s words to '%s'\n", argv[1], argv[2], tempFileName);

  /*
   * Creating pipe
   */
  int catGrepPipe[2];
  int grepSortPipe[2];

  char word[100];
  char c;
  pid_t pid;
  while((c = fscanf(wordsFile, "%s", word)) != EOF){

    if(pipe(catGrepPipe) == -1){
      fprintf(stderr, "Couldn't pipe catGrepPipe\n");
      exit(4);
    }

    if(pipe(grepSortPipe) == -1){
      fprintf(stderr, "Couldn't pipe grepSortPipe\n");
      exit(4);
    }

    pid = fork();
    if(pid == 0){
      dup2(grepSortPipe[WRITE], STDOUT_FILENO);
      dup2(catGrepPipe[READ], STDIN_FILENO);

      close(catGrepPipe[READ]);
      close(catGrepPipe[WRITE]);

      close(grepSortPipe[READ]);
      close(grepSortPipe[WRITE]);
      execlp("grep", "grep", "-no", word, NULL);
      fprintf(stderr, "-----grep not executed !\n");
      exit(3);
    }

    pid = fork();
    if(pid == 0){
      dup2(catGrepPipe[WRITE], STDOUT_FILENO);
      close(catGrepPipe[READ]);
      close(catGrepPipe[WRITE]);
      execlp("cat", "cat", argv[1], NULL);
      fprintf(stderr, "-----cat not executed !\n");
      exit(3);
    }
    close(catGrepPipe[READ]);
    close(catGrepPipe[WRITE]);
    wait(NULL);
    wait(NULL);

    /* Sort like "<Word> : <chapter>-<line>" */
    pid = fork();
    if(pid == 0){
      close(grepSortPipe[WRITE]);

      char c;
      int firstNumber = 1;
      setbuf(stdout, NULL);

      while(read(grepSortPipe[READ], &c, sizeof(char))){
        if(c >= '0' && c <= '9'){
          if(firstNumber){
            fprintf(tempF, "%s:%s-", word, argv[2]);
            firstNumber = 0;
          }
          fprintf(tempF, "%c", c);
        }
        else if(c == ':'){
          fprintf(tempF, "#\n");
          firstNumber = 1;
        }
      }

      close(grepSortPipe[READ]);
      fclose(tempF);
      exit(0);
    }
    close(grepSortPipe[READ]);
    close(grepSortPipe[WRITE]);
    wait(NULL);
  }

  fclose(wordsFile);

  exit(0);
}
