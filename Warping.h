/*
#  File        : Warping.h
#  Description : Perspective transformation of 2D image
#  Copyright   : HYPJUDY ( https://hypjudy.github.io/ ) 2017/4/6
*/

#pragma once
#ifndef _Warping_
#define _Warping_
#include "Hough.h"
#include<Eigen/Dense>

class Warping {
private:
	CImg<float> dest_A4; // 210mm*297mm -> 410*594
	CImg<float> src;
	const float W = 410, H = 594;
	// destination corners
	const float u1 = 0, v1 = 0, // top-left
		u2 = W - 1, v2 = 0, // top-right
		u3 = 0, v3 = H - 1, // bottom-left
		u4 = W - 1, v4 = H - 1; // bottom-right
	float x1, y1, x2, y2, x3, y3, x4, y4; // source corners
	float a, b, c, d, e, f, m, l; // parameters

	void perspectiveTransform();
	float getXTransformInv(int u, int v);
	float getYTransformInv(int u, int v);
	void reverseMapping();
	float bilinearInterpolate(float x, float y, int z);
	void mapping(float x, float y);
public:
	Warping(Hough hough2);
	CImg<float> getCroppedImg() { return dest_A4; }
};

#endif