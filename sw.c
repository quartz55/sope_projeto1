#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define READ 0
#define WRITE 1

int main (int argc, char *argv[])
{

  if(argc != 2){
    printf("Usage: %s <filename>.txt\n", argv[0]);
    exit(1);
  }

  FILE *wordsFile;
  /*
   * Open words.txt file
   */
  wordsFile = fopen("./files/words.txt", "r");
  if(!wordsFile){
    fprintf(stderr, "'words.txt' could not be found\n");
    exit(2);
  }

  /*
   * Creating pipe
   */
  int mypipe[2];
  if(pipe(mypipe) == -1){
    fprintf(stderr, "Couldn't pipe\n");
    exit(4);
  }

  char word[100];
  char c;
  pid_t pid;
  while((c = fscanf(wordsFile, "%s", word)) != EOF){
    /* printf("------%s\n", word); */

    pid = fork();
    if(pid == 0){
      /* dup2(mypipe[WRITE], STDOUT_FILENO); */
      dup2(mypipe[READ], STDIN_FILENO);
      close(mypipe[READ]);
      close(mypipe[WRITE]);
      execlp("grep", "grep", "-no", word, NULL);
      fprintf(stderr, "-----grep not executed !\n");
      exit(3);
    }
    else if(pid > 0){
      pid = fork();
      if(pid == 0){
        dup2(mypipe[WRITE], STDOUT_FILENO);
        /* dup2(mypipe[READ], STDIN_FILENO); */
        close(mypipe[READ]);
        close(mypipe[WRITE]);
        execlp("cat", "cat", argv[1], NULL);
        fprintf(stderr, "-----cat not executed !\n");
        exit(3);
      }
    }
    wait(NULL);
  }

  fclose(wordsFile);

  exit(0);
}
