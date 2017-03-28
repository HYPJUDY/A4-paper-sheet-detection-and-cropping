/*
#  File        : hough_transform.h ( C++ source file )
#
#  Description : Detect Edges of Paper Sheet with Hough Transform
#
#  Copyright   : HYPJUDY ( https://hypjudy.github.io/ )
*/

#pragma once
#ifndef _Hough_
#define _Hough_
#include "CImg.h"
#include<vector>
using namespace cimg_library;
struct HoughEdge {
	int angle, rho, val;
	HoughEdge(int _angle, int _rho, int _val)
		: angle(_angle), rho(_rho), val(_val) {}
};
struct Line {
	double m, b;
	int dist_o; // distance to origin point when 
	    // line(x = dist_o) is perpendicular to x axis
	int x0, x1, y0, y1; // two end points
	int end_point_num;
	Line(double _m, double _b, int _dist_o = 0, int _x0 = 0, int _y0 = 0,
		int _x1 = 0, int _y1 = 0, int _end_point_num = 0)
		: m(_m), b(_b), dist_o(_dist_o), x0(_x0), x1(_x1), y0(_y0), y1(_y1),
	    end_point_num(_end_point_num) {}
};
struct Point {
	int x, y;
	Point(int _x, int _y) : x(_x), y(_y) {}
};
class Hough {
private:
	const double BLUR_SIGMA = 2;
	const double GRAD_THRESHOLD = 20;
	const int BIAS = 30; // subtrat it from threshold used in getHoughEdges
	const int SCOPE = 20; // scope of clusters in hough space
	int w, h; // width and height of rgb image
	CImg<double> gradients;
	CImg<double> hough_space;
	CImg<double> rgb_img;
	CImg<double> gray_img;
	std::vector<HoughEdge> hough_edges; // four edges in hough space
	std::vector<Line> lines; // four edges in parameter space
	std::vector<Point> Corners; // four corners in normal space

public:
	Hough(char * filePath);
	double distance(double diff_x, double diff_y);
	void rgb2gray();
	void getGradient();
	void houghTransform();
	void getHoughEdges();
	void getLines();
	void getCorners();
	void displayCornersAndLines();
	CImg<double> getRGBImg() { return rgb_img; }
};

#endif