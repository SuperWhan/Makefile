/* CSci4061 F2018 Assignment 1
* login: cselabs login name (login used to submit)
* date: mm/dd/yy
* name: full name1, full name2, full name3 (for partner(s))
* id: id for first name, id for second name, id for third name */

// This is the main file for the code
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"

/*-------------------------------------------------------HELPER FUNCTIONS PROTOTYPES---------------------------------*/
void show_error_message(char * ExecName);
//Write your functions prototypes here
void show_targets(target_t targets[], int nTargetCount);

int build_run_tree(char* target_name, target_t targets[], int nTargetCount);

/*-------------------------------------------------------END OF HELPER FUNCTIONS PROTOTYPES--------------------------*/


/*-------------------------------------------------------HELPER FUNCTIONS--------------------------------------------*/

//This is the function for writing an error to the stream
//It prints the same message in all the cases
void show_error_message(char * ExecName)
{
	fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", ExecName);
	fprintf(stderr, "-f FILE\t\tRead FILE as a makefile.\n");
	fprintf(stderr, "-h\t\tPrint this message and exit.\n");
	exit(0);
}

//Write your functions here

//Phase1: Warmup phase for parsing the structure here. Do it as per the PDF (Writeup)
void show_targets(target_t targets[], int nTargetCount)
{
	//Write your warmup code here
	for (int i = 0; i < nTargetCount; i++) {
		int d_count = targets[i].DependencyCount;
		printf(" * Target Name: %s\n", targets[i].TargetName);
		printf(" * DenpendencyCount: %d\n", d_count);
		printf(" * DependencyNames: ");
		while (d_count > 0) {
			printf("%s ", targets[i].DependencyNames[d_count-1]);
			d_count --;
		}
		printf("\n");
		printf(" * Command: %s\n\n", targets[i].Command);
	}
}

/*----------------------- excute for tree & it's dependency--------------------*/

int build_run_tree(char* target_name, target_t targets[], int nTargetCount)
{
	target_t t_target;
	int target_id;
	int file_flag;
	int d_count = 0;

	target_id = find_target(target_name, targets, nTargetCount);
	file_flag = does_file_exist(target_name);

	if (target_id != -1) {
		int d_Num = targets[target_id].DependencyCount;
		// if no dependency, is a leaf, then update the target
		if (d_Num == 0) {
			targets[target_id].Status = 1;
		}

		while (d_count < d_Num) {
			char *d_Name = targets[target_id].DependencyNames[d_count];
			// printf("%s\n", d_Name);
			d_count++;
			// go through the whole DAG, and give them update flag.
			if (build_run_tree(d_Name, targets, nTargetCount) == 1 ||
					compare_modification_time (target_name, d_Name) == -1 ||
					compare_modification_time(target_name, d_Name) == 2) {
						targets[target_id].Status = 1;
					}
		}
	}

	else {
		// not a target and no such file. goes error
		if (file_flag == -1) {
			fprintf(stderr, "Error: Input target '%s' has no match or without such file \n", target_name);
			exit(0);
		}
		// not a target, but has such file, muat be a leaf file, keep it.
		return targets[target_id].Status;
	}

	if (targets[target_id].Status) {
		targets[target_id].Status = 0;
		pid_t pid = fork();

		if (pid == -1) {
			perror ("fork error");
			exit(-1);
		}

		// child process
		else if (pid == 0) {
			printf("%s\n", targets[target_id].Command);
			char *token[1024];
			parse_into_tokens(targets[target_id].Command, token, " ");
			execvp(*token, token);

		}
		// parent process
		else {
		int wait_status;
		wait (&wait_status);
		if (WEXITSTATUS(wait_status) != 0){
			printf("child exited with error code = %d \n", WEXITSTATUS(wait_status));
			exit(-1);
			}
		}
	}
	return targets[target_id].Status;
}


/*----------------------- excute for tree & it's dependency--------------------*/
/*-------------------------------------------------------END OF HELPER FUNCTIONS-------------------------------------*/


/*-------------------------------------------------------MAIN PROGRAM------------------------------------------------*/
//Main commencement
int main(int argc, char *argv[])
{
  target_t targets[MAX_NODES];
  int nTargetCount = 0;

  /* Variables you'll want to use */
  char Makefile[64] = "Makefile";
  char TargetName[64];

  /* Declarations for getopt. For better understanding of the function use the man command i.e. "man getopt" */
  extern int optind;   		// It is the index of the next element of the argv[] that is going to be processed
  extern char * optarg;		// It points to the option argument
  int ch;
  char *format = "f:h";
  char *temp;

  //Getopt function is used to access the command line arguments. However there can be arguments which may or may not need the parameters after the command
  //Example -f <filename> needs a finename, and therefore we need to give a colon after that sort of argument
  //Ex. f: for h there won't be any argument hence we are not going to do the same for h, hence "f:h"
  while((ch = getopt(argc, argv, format)) != -1)
  {
	  switch(ch)
	  {
	  	  case 'f':
	  		  temp = strdup(optarg);
	  		  strcpy(Makefile, temp);  // here the strdup returns a string and that is later copied using the strcpy
	  		  free(temp);	//need to manually free the pointer
	  		  break;

	  	  case 'h':
	  	  default:
	  		  show_error_message(argv[0]);
	  		  exit(1);
	  }

  }

  argc -= optind;
  if(argc > 1)   //Means that we are giving more than 1 target which is not accepted
  {
	  show_error_message(argv[0]);
	  return -1;   //This line is not needed
  }

  /* Init Targets */
  memset(targets, 0, sizeof(targets));   //initialize all the nodes first, just to avoid the valgrind checks

  /* Parse graph file or die, This is the main function to perform the toplogical sort and hence populate the structure */
  if((nTargetCount = parse(Makefile, targets)) == -1)  //here the parser returns the starting address of the array of the structure. Here we gave the makefile and then it just does the parsing of the makefile and then it has created array of the nodes
	  return -1;


  //Phase1: Warmup-----------------------------------------------------------------------------------------------------
  //Parse the structure elements and print them as mentioned in the Project Writeup
  /* Comment out the following line before Phase2 */
  show_targets(targets, nTargetCount);
  //End of Warmup------------------------------------------------------------------------------------------------------

  /*
   * Set Targetname
   * If target is not set, set it to default (first target from makefile)
   */
  if(argc == 1)
	strcpy(TargetName, argv[optind]);    // here we have the given target, this acts as a method to begin the building
  else
  strcpy(TargetName, targets[0].TargetName);  // default part is the first target

  /*
   * Now, the file has been parsed and the targets have been named.
   * You'll now want to check all dependencies (whether they are
   * available targets or files) and then execute the target that
   * was specified on the command line, along with their dependencies,
   * etc. Else if no target is mentioned then build the first target
   * found in Makefile.
   */

  //Phase2: Begins ----------------------------------------------------------------------------------------------------
  /*Your code begins here*/

	for (int i = 0; i <nTargetCount; i++)
	targets[i].Status = 0;

	build_run_tree(TargetName, targets, nTargetCount);


  /*End of your code*/
  //End of Phase2------------------------------------------------------------------------------------------------------

  return 0;
}
/*-------------------------------------------------------END OF MAIN PROGRAM------------------------------------------*/
