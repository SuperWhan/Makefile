#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"

void show_error_message(char * ExecName) {
  fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", ExecName);
  fprintf(stderr, "-f FILE\t\tRead FILE as a makefile.\n");
  fprintf(stderr, "-h\t\tPrint this message and exit.\n");
  exit(0);
}

int main(int argc, char *argv[]) {
  target_t targets[MAX_NODES];
  int nTargetCount = 0;
  int updated;


  /* Variables you'll want to use */
  char Makefile[64] = "Makefile";
  char TargetName[64];

  /* Declarations for getopt */
  extern int optind;
  extern char * optarg;
  int ch;
  char * format = "f:h";

  while((ch = getopt(argc, argv, format)) != -1) {
    switch(ch) {
      case 'f':
        strcpy(Makefile, strdup(optarg));
        break;
      case 'h':

      default:
        show_error_message(argv[0]);
        exit(1);
    }
  }

  argc -= optind;
  if(argc > 1) {
    show_error_message(argv[0]);
    return -1;
  }

  /* Init Targets */
  memset(targets, 0, sizeof(targets));

  /* Parse graph file or die */
  if((nTargetCount = parse(Makefile, targets)) == -1) {
    return -1;
  }
  //Initialize the targets[].Status to zerotime run make for each run of make4061
  //This provides us with accurate values for if a dependency was visited yet or not
  for (int i=0;i<nTargetCount;i++)
  targets[i].Status = 0;
  /* Comment out the following line before submission */
 //show_targets(targets, nTargetCount);

  /*
   * Set Targetname
   * If target is not set, set it to default (first target from makefile)
   */
  if(argc == 1) {
    strcpy(TargetName, argv[optind]);
  } else {

    strcpy(TargetName, targets[0].TargetName);
  }


  /*
   * Now, the file has been parsed and the targets have been named.
   * You'll now want to check all dependencies (whether they are
   * available targets or files) and then execute the target that
   * was specified on the command line, along with their dependencies,
   * etc. Else if no target is mentioned then build the first target
   * found in Makefile.
   */

/*Call our function and retain flag.
    updated:
    0 --> Nothing was updated when running program. Tell user.
    1 --> Main target was updated.
*/
   updated = make_exec(TargetName, targets, nTargetCount);
   if (updated == 0){
     fprintf(stderr, "make4061: '%s' is up to date.\n", TargetName);
   }

  return 0;
}

/*
use make_exec to do two functions:
1)check the timestamp and file existence
2)build and create child process
*/
int make_exec(char* TargetName, target_t targets[], int nTargetCount){
  //create flag with default value 0, indicating no need to update
  int flag= 0;
  int target_index = find_target(TargetName,targets,nTargetCount);
  //default status to 0 i.e. we have not visited this node before
  //targets[target_index].Status = 0;
  char* TargetNamedepend;
  //target exists
  if(target_index != -1){
    //
     if (targets[target_index].DependencyCount > 0){
      //loop through all dependency
      //as long as one dependency's subtree needs to be updated or current level timestamp is stale
      for (int i=0; i<targets[target_index].DependencyCount; i++){
        TargetNamedepend= targets[target_index].DependencyNames[i];

        if ((make_exec(TargetNamedepend, targets, nTargetCount)==1) ||
        (compare_modification_time(TargetName, TargetNamedepend)==-1) ||
        (compare_modification_time(TargetName, TargetNamedepend)==2)){
            flag=1;
        }
      }
    }
    //it is a target but no dependency
    else{
      flag=1;
    }
      //look at dependencies and recurse through till no more dependencies are found
  }

  //target not exists
  else{
  //reach the leaf, but file or target does not exist
    if (does_file_exist(TargetName)==-1){
      fprintf(stderr, "make4061: No rule to make target '%s'. Stop\n", TargetName);
      //exit with error because file is not present
      exit(-1);
    }
    //leaf should not always create process
    return flag;
  }
  //file does exist and check update flag
  if (flag==1 &&  targets[target_index].Status == 0){
    targets[target_index].Status = 1;
    pid_t pid = fork();

    //Child process. Exec to run command
    if (pid == 0){
      printf("%s\n", targets[target_index].Command);
      // char **arg = build_argv(targets[target_index].Command);
      // execvp(*arg,arg);
    }
    //Child process could not be created. Stop
    else if (pid == -1){
      fprintf(stderr, "make4061: Child process creation for '%s' failed!\n", TargetName);
      exit(-1);
    }
    //Parent waits for child to exit
    else {
      int wstatus;
      //child created but exited improperly. Stop
      wait(&wstatus);
      if (WEXITSTATUS(wstatus) != 0){
        printf("make4061: Child exited with error code=%d with regards to '%s'\n", WEXITSTATUS(wstatus), TargetName);
        exit(-1);
      }
    }
  }
  return flag;
}
