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
	if((dirp = opendir(startdir)) == NULL) {
		perror("opendir");
		exit(1);
	} 
	
	/* For each entry in the directory, eliminate . and .., and check
	* to make sure that the entry is a directory, then find the correct 
	* number of directories to set up the pipe
	*/
		
	struct dirent *dp;
        CompRecord CRec;
	CRec.distance = FLT_MAX;
	CompRecord Temp;
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
	printf("the correct number of file in current dir is %d\n", dir_number);	
	

	// reset the current dir and re-read it again to run process_dir
	if((dirp = opendir(startdir)) == NULL) {
		perror("opendir");
		exit(1);
	} 
	int pipe_fd[dir_number][2]; // set up the pipe_fd

	
	// ****** you can start your fork here
	

	
	// read each sub directory in the current directory and fork process to 
	// run process_dir
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

		// Only call process_dir if it is a directory
		// Otherwise ignore it.
		if(S_ISDIR(sbuf.st_mode)) {
                        Temp = process_dir(path, select_image, 6);
			// find the largest to store to CRec;
			if(Temp.distance < CRec.distance){
				strcpy(CRec.filename, Temp.filename);
				CRec.distance = Temp.distance;
			}
		}
		
	}
	
        printf("The most similar image is %s with a distance of %f\n", CRec.filename, CRec.distance);
	
	return 0;
}
