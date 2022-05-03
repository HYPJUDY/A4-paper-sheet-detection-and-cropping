/*
#  File        : main.cpp
#  Description : Main procedure of A4 Paper Sheet Detection 
#                and Cropping with Hough Transform and Warping
#  Copyright   : HYPJUDY 2017/4/6
#  Details     : https://hypjudy.github.io/2017/03/28/cvpr-A4-paper-sheet-detection-and-cropping/
#  Code        : https://github.com/HYPJUDY/A4-paper-sheet-detection-and-cropping
*/

#include <limits.h>
#include "Warping.h"

int main(int argc, char **argv) {
	char inPath[NAME_MAX], outPath[NAME_MAX], outPath2[NAME_MAX];

	for (int i = 1; i < argc; ++i) {
		strncpy(inPath, argv[i], NAME_MAX - 1);
		sprintf(outPath, "/tmp/output-%02d.jpg", i);
		sprintf(outPath2, "/tmp/output_A4-%02d.jpg", i);

		Hough hough(inPath);
		Warping Warping(hough);
		
		hough.getMarkedImg().display().save(outPath);
		Warping.getCroppedImg().display().save(outPath2);
	}
	return 0;
}

/* Error cases guide:
* exit(-1): ERROR: Please set parameter Q larger in file \
*			'hough_transform.h' to filter out four edges!
* exit(-2): ERROR: Bug in function void Hough::getHoughEdges()!\
*           Please check the ifelse statement to filter out four hough_edges.
* exit(-3): ERROR: Can not detect four ordered_corners in function \
            void Hough::orderCorners(). Please try to adjust parameters.
*/
