#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <math.h>
#include <float.h>
#include "worker.h"


/*
 * Read an image from a file and create a corresponding 
 * image struct 
 */

Image* read_image(char *filename)
{
        Image *img;
        return img;
}

/*
 * Print an image based on the provided Image struct 
 */

void print_image(Image *img){
        printf("P3\n");
        printf("%d %d\n", img->width, img->height);
        printf("%d\n", img->max_value);
       
        for(int i=0; i<img->width*img->height; i++)
           printf("%d %d %d  ", img->p[i].red, img->p[i].green, img->p[i].blue);
        printf("\n");
}

/*
 * Compute the Euclidian distance between two pixels 
 */
float eucl_distance (Pixel p1, Pixel p2) {
        return sqrt( pow(p1.red - p2.red,2 ) + pow( p1.blue - p2.blue, 2) + pow(p1.green - p2.green, 2));
}

/*
 * Compute the average Euclidian distance between the pixels 
 * in the image provided by img1 and the image contained in
 * the file filename
 */

float compare_images(Image *img1, char *filename) {
       return 0;
}

/* process all files in one directory and find most similar image among them
* - open the directory and find all files in it 
* - for each file read the image in it 
* - compare the image read to the image passed as parameter 
* - keep track of the image that is most similar 
* - write a struct CompRecord with the info for the most similar image to out_fd
*/
CompRecord process_dir(char *dirname, Image *img){
	// initialize the path of sub directory 
	char path[PATHLENGTH];
	DIR *dirp;
	// initialize the max_distance in the directory
	float max_distance = FLT_MAX;
	float temp = FLT_MAX;
	// if open sub directory fails, exit with error report
	if((dirp = opendir(dirname)) == NULL) {
		perror("opendir");
		exit(1);
	}
	// loop through each file in the sub directory
	strit dirent *dp;
        CompRecord CRec;
	while((dp = readdir(dirp)) != NULL){
		// strncpy the path of the file to the root 
		strncpy(path, dirname, PATHLENGTH);
		strncat(path, "/", PATHLENGTH - strlen(path) - 1);
		strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);
		// check if the path is correct
		struct stat sbuf;
		if(stat(path, &sbuf) == -1){
		perror("stat");
		exit(1);
		}
		// check if the file is a directory or it is regular file
		if(S_ISREG(sbuf.st_mode)) {
				// if it is a regular file, call compare_image 
				temp = compare_images(img, path);
				// let max_distance stores the largest distance
				if(temp > max_distance){
					max_distance = temp;
				}
		}
				
		}
		// write the file name and max_distance to the return result
		strcpy(CRec.filename, path);
		CRec.distance = max_distance;
		 
        return CRec;
}
