#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <float.h>
#include "worker.h"

int main(int argc, char **argv) {
	
	char ch;
	char path[PATHLENGTH];
	char *startdir = ".";
        char *image_file = NULL;
	int dir_number;

	while((ch = getopt(argc, argv, "d:")) != -1) {
		switch (ch) {
			case 'd':
			startdir = optarg;
			break;
			default:
			fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME] FILE_NAME\n");
			exit(1);
		}
	}

        if (optind != argc-1) {
	     fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME] FILE_NAME\n");
        } else
             image_file = argv[optind];
	// add selected image to create an image struct
	     Image *select_image = read_image(image_file);
		
	// Open the directory provided by the user (or current working directory)
	
	DIR *dirp;
	DIR *dirp2;
	if((dirp = opendir(startdir)) == NULL) {
		perror("opendir");
		exit(1);
	} 
	
	/* For each entry in the directory, eliminate . and .., and check
	* to make sure that the entry is a directory, then find the correct 
	* number of directories to set up the pipe
	*/
		
	struct dirent *dp;
	struct dirent *dp2;
        CompRecord CRec;
	strcpy(CRec.filename, "");
	CRec.distance = FLT_MAX;
	CompRecord Temp;
	strcpy(Temp.filename, "");
	Temp.distance = FLT_MAX;
	dir_number = 0;
	while((dp = readdir(dirp)) != NULL) {

		if(strcmp(dp->d_name, ".") == 0 || 
		   strcmp(dp->d_name, "..") == 0 ||
		   strcmp(dp->d_name, ".svn") == 0){
			continue;
		}
		strncpy(path, startdir, PATHLENGTH);
		strncat(path, "/", PATHLENGTH - strlen(path) - 1);
		strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);

		struct stat sbuf;
		if(stat(path, &sbuf) == -1) {
			//This should only fail if we got the path wrong
			// or we don't have permissions on this entry.
			perror("stat");
			exit(1);
		} 

		// if it is the directory, increase the dir_number by 1;
		if(S_ISDIR(sbuf.st_mode)) {
			dir_number += 1;
		}
		
	}
	
	

	// reset the current dir and re-read it again to run process_dir
	
	if((dirp2 = opendir(startdir)) == NULL) {
		perror("opendir");
		exit(1);
	} 
	int pipe_fd[dir_number][2]; // set up the pipe_fd

	
	// ****** you can start your fork here
	

	
	// read each sub directory in the current directory and fork process to 
	// run process_dir
	int i = -1;
	while((dp2 = readdir(dirp2)) != NULL) {
		
		if(strcmp(dp2->d_name, ".") == 0 || 
		   strcmp(dp2->d_name, "..") == 0 ||
		   strcmp(dp2->d_name, ".svn") == 0){
			continue;
		}
		strncpy(path, startdir, PATHLENGTH);
		strncat(path, "/", PATHLENGTH - strlen(path) - 1);
		strncat(path, dp2->d_name, PATHLENGTH - strlen(path) - 1);
		struct stat sbuf;
		if(stat(path, &sbuf) == -1) {
			//This should only fail if we got the path wrong
			// or we don't have permissions on this entry.
			perror("stat");
			exit(1);
		} 
		// Only call process_dir if it is a directory
		// Otherwise ignore it.
		if(S_ISDIR(sbuf.st_mode)) {
			i ++;
			// call pipe for each sub dir
			if((pipe(pipe_fd[i])) == -1){
				perror("pipe");
				exit(1);
			}
			// call fork
			int result = fork();
			if(result <0) { // case: a system call error
				perror("fork");
				exit(1);
			}
			else if(result == 0){ // case: a child process
				if(close(pipe_fd[i][0]) == -1) {
				perror("close reading end from inside child");
				}
				// close all reading ends
				int child_no;
				for(child_no =1; child_no < i; child_no++){
					if(close(pipe_fd[child_no][0]) == -1){
						perror("close reading ends of previously forked child");
					}
				}
				// call process_dir and pass the struct to the pipe
                        	process_dir(path, select_image, pipe_fd[i][1]);
				// done with pipe and close it
				if(close(pipe_fd[i][1]) == -1) {
					perror("close pipe after writing");
				}
				// exist so no more children will be created
				exit(0);
				}
			else{	// if the process is a parent
			// close the writing end of the pipe
			if(close(pipe_fd[i][1]) == -1){
				perror("close writing end of pipe in parent");
					}
	
				}
		
			}
	}
	// only parent goes here
	// find the largest to store to CRec;
	for(int k = 0; k < dir_number; k ++){
		if(read(pipe_fd[k][0], &Temp, sizeof(CompRecord)) == -1){
			perror("reading from pipe from a child");
			}

		if(Temp.distance < CRec.distance){
			strcpy(CRec.filename, Temp.filename);
			CRec.distance = Temp.distance;
			}
	}
        printf("The most similar image is %s with a distance of %f\n", CRec.filename, CRec.distance);
	
	return 0;
}
