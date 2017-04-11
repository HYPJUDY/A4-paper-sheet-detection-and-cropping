/*
#  File        : main.cpp
#  Description : Main procedure of A4 Paper Sheet Detection 
#                and Cropping with Hough Transform and Warping
#  Copyright   : HYPJUDY ( https://hypjudy.github.io/ ) 2017/4/6
*/

#include "Warping.h"
int main() {
	// adjust the num array and forloop below to process different image
	std::vector<const char*> num = { "0", "1", "2", "3", "4", "5", 
		"6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16" };
	for (int i = 0; i <= 15; ++i) {
		// load source image
		char inPath[80];
		strcpy(inPath, "Dataset/");
		strcat(inPath, num[i]);
		strcat(inPath, ".bmp");
		
		Hough hough(inPath);
		Warping Warping(hough);
		
		char outPath[80];
		strcpy(outPath, "Dataset/");
		strcat(outPath, num[i]);
		strcat(outPath, "_marked.bmp");
		hough.getMarkedImg().save(outPath);

		char outPath2[80];
		strcpy(outPath2, "Dataset/");
		strcat(outPath2, num[i]);
		strcat(outPath2, "_A4.bmp");
		Warping.getCroppedImg().save(outPath2);
	}
	return 0;
}