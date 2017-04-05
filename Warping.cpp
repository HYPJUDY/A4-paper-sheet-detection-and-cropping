/*
#  File        : Warping.cpp
#  Description : Perspective transformation of 2D image
#  Copyright   : HYPJUDY ( https://hypjudy.github.io/ ) 2017/4/6
*/

#include "Warping.h"
Warping::Warping(Hough hough) {
	src = hough.getRGBImg();
	CImg<double> temp(W, H, 1, 3, 0);
	dest_A4 = temp;
	std::vector<Point>  corners = hough.getOrderedCorners();
	x1 = corners[0].x, y1 = corners[0].y; // top-left
	x2 = corners[1].x, y2 = corners[1].y; // top-right
	x3 = corners[2].x, y3 = corners[2].y; // bottom-left
	x4 = corners[3].x, y4 = corners[3].y; // bottom-right
	perspectiveTransform();
	reverseMapping();
}

void Warping::perspectiveTransform() {
	Eigen::MatrixXf UV(8, 1);
	Eigen::MatrixXf M = Eigen::MatrixXf::Constant(8, 1, 0);
	Eigen::MatrixXf A(8, 8);
	UV << u1, v1, u2, v2, u3, v3, u4, v4;
	A << x1, y1, 1, 0,  0,  0, -u1*x1, -u1*y1,
		 0,  0,  0, x1, y1, 1, -v1*x1, -v1*y1,
		 x2, y2, 1, 0,  0,  0, -u2*x2, -u2*y2,
		 0,  0,  0, x2, y2, 1, -v2*x2, -v2*y2,
		 x3, y3, 1, 0,  0,  0, -u3*x3, -u3*y3,
		 0,  0,  0, x3, y3, 1, -v3*x3, -v3*y3,
		 x4, y4, 1, 0,  0,  0, -u4*x4, -u4*y4,
		 0,  0,  0, x4, y4, 1, -v4*x4, -v4*y4;
	M = A.inverse() * UV;
	a = M(0,0), b = M(1, 0), c = M(2, 0), d = M(3, 0),
		e = M(4, 0), f = M(5, 0), m = M(6, 0), l = M(7, 0);
}

float Warping::getXTransformInv(int u, int v) {
	return ((c - u)*(v*l - e) - (f - v)*(u*l - b)) /
		((u*m - a)*(v*l - e) - (v*m - d)*(u*l - b));
}

float Warping::getYTransformInv(int u, int v) {
	return ((c - u)*(v*m - d) - (f - v)*(u*m - a)) /
		((u*l - b)*(v*m - d) - (v*l - e)*(u*m - a));
}

float Warping::bilinearInterpolate(float x, float y, int c) {
	int i = floorf(x), j = floorf(y);
	float a = x - i, b = y - j;
	return (1 - a)*(1 - b)*src(i, j, c) + a*(1 - b)*src(i + 1, j, c)
		+ (1 - a)*b*src(i, j + 1, c) + a*b*src(i + 1, j + 1, c);
}

void Warping::reverseMapping() {
	cimg_forXYC(dest_A4, u, v, c) { // c indicates color channels
		float x = getXTransformInv(u, v);
		float y = getYTransformInv(u, v);
		dest_A4(u, v, c) = bilinearInterpolate(x, y, c);
	}
}

