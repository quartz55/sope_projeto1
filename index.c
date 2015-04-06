#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

  if(argc != 2){
    printf("Usage: %s <dir>\n", argv[0]);
    exit(2);
  }

  exit(0);
}
