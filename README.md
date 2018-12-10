/* CSci4061 F2018 Assignment 1
* login: huxxx990
* date: 10/05/18

1.__
Introduction__
make4061 is our implementation of gnu make. My program will take a standard
"Makefile" as an input or user define file name.
By default, make4061 will search its directory to look for a "Makefile",
and it will perform like the gnu make. Our make4061 will recursively resolve
make target and its dependency / dependencies.

I implemented My program with the following method:__
1. a helper function called build_run_tree and it takes three arguments,
char* target_name, target_t targets[], int nTargetCount.
2. build_run_tree will check for if the given TargetName is a build target (target_id)
and if the file exist (file_flag) then give appropriate flag.
3. if target_id is -1 means that the given TargetName is not a target,
if target_id > -1 means it is a target and the value represent the index of
this target on targets[];
4. then we will check for the number of dependencies of this target.
5. if this target does not have any dependency (leaf), we will set the status to 1
which mean this target is ready to be built.
6. if this target has dependencies, we will put the dependencies to recursive
call, if it is update to date, build_run_tree will return 1, and compare the target
with all of its dependencies, if dependencies have modified before target, set status
to 1.
7. If its status == 1, then we fork, then its child process execute the command
parent then will be waited for the child process to finish.
8. If status is not equal 1, means the target is up-to-date.
