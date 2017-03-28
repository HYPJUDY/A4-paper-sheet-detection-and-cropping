/*
#  File        : main.cpp ( C++ source file )
#
#  Description : Detect Edges of Paper Sheet with Hough Transform
#
#  Copyright   : HYPJUDY ( https://hypjudy.github.io/ )
*/

/* Main procedure */
#include "hough_transform.h"
int main() {
	std::vector<const char*> num = { "0", "1", "2", "3", "4", "5" };
	for (int i = 0; i <= 5; ++i) {
		// load source image
		char inPath[80];
		strcpy(inPath, "Dataset/");
		strcat(inPath, num[i]);
		strcat(inPath, ".bmp");
		
		Hough hough(inPath);
		
		// save source image
		char outPath[80];
		strcpy(outPath, "Dataset/");
		strcat(outPath, num[i]);
		strcat(outPath, "_out.bmp");
		hough.getRGBImg().save(outPath);
	}
	return 0;
}