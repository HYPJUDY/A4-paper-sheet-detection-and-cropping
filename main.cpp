/*
#  File        : main.cpp
#  Description : Main procedure of A4 Paper Sheet Detection 
#                and Cropping with Hough Transform and Warping
#  Copyright   : HYPJUDY 2017/4/6
#  Details     : https://hypjudy.github.io/2017/03/28/cvpr-A4-paper-sheet-detection-and-cropping/
#  Code        : https://github.com/HYPJUDY/A4-paper-sheet-detection-and-cropping
*/

#include "Warping.h"
int main() {
	int CASE = 1; // 1 for dataset1; otherwise for dataset2

	/* Parameters for dataset */
	int image_num = 16;
	const char* data_folder = "dataset1/";
	if (CASE == 1) {
		image_num = 16;
		data_folder = "dataset1/";
	}
	else {
		image_num = 5;
		data_folder = "dataset2/";
	}
	
	// adjust the num array below to process different image
	std::vector<const char*> num = { "0", "1", "2", "3", "4", "5",
		"6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16" };
	for (int i = 0; i < image_num; ++i) {
		// load source image
		char inPath[80];
		strcpy(inPath, data_folder);
		strcat(inPath, num[i]);
		strcat(inPath, ".bmp");
		
		Hough hough(inPath);
		Warping Warping(hough);
		
		char outPath[80];
		strcpy(outPath, data_folder);
		strcat(outPath, num[i]);
		strcat(outPath, "_marked.bmp");
		hough.getMarkedImg().display().save(outPath);

		char outPath2[80];
		strcpy(outPath2, data_folder);
		strcat(outPath2, num[i]);
		strcat(outPath2, "_A4.bmp");
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