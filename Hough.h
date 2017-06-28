/*
#  File        : Hough.h
#  Description : Detect Edges of Paper Sheet with Hough Transform
#  Copyright   : HYPJUDY 2017/4/6
#  Details     : https://hypjudy.github.io/2017/03/28/cvpr-A4-paper-sheet-detection-and-cropping/
#  Code        : https://github.com/HYPJUDY/A4-paper-sheet-detection-and-cropping
*/

#pragma once
#ifndef _Hough_
#define _Hough_
#include "CImg.h"
#include<iostream>
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
	/* adjustable parameters */
	const float BLUR_SIGMA = 2;
	const float GRAD_THRESHOLD = 20;
	const int Q = 3; // the denominator parameter used to get
	                 // threshold in getHoughEdges; aims to filter
	                 // out more than 3 edges
	// since angle and rho are in different scale, use different scope
	const int SCOPE_ANGLE = 20; // scope of clusters in hough space
	const int SCOPE_RHO = 100; // scope of clusters in hough space
	const int D = 20; // intersects can be out of image in distance D
	float x1, y1, x2, y2, x3, y3, x4, y4; // source corners

	int w, h; // width and height of rgb image
	CImg<float> gradients;
	CImg<float> hough_space;
	CImg<float> rgb_img;
	CImg<float> marked_img; // with paper sheet corners and edges mark
	CImg<float> gray_img;
	std::vector<HoughEdge> hough_edges; // four edges in hough space
	std::vector<Line> lines; // four edges in parameter space
	std::vector<Point> corners; // duplicate four corners in normal space
	std::vector<Point> ordered_corners; // four corners in normal space
	// in the order of top-left, top-right, bottom-left, bottom-right

	float distance(float diff_x, float diff_y);
	void rgb2gray();
	void getGradient();
	void houghTransform();
	void getHoughEdges();
	void getLines();
	void getCorners();
	void orderCorners();
	void displayCornersAndLines();
public:
	Hough(char * filePath);
	CImg<float> getRGBImg() { return rgb_img; }
	CImg<float> getMarkedImg() { return marked_img; }
	std::vector<Point> getOrderedCorners() { return ordered_corners;}
};





#endif