#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>

int main() {
  pid_t pid;
  pid = fork();

  if (pid == 0){
    sleep(5);
    printf("World\n");
  }
  if (pid > 0 ) {
    printf("Hello");
  }
}
