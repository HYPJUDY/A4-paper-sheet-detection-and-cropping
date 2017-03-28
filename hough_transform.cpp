/*
#  File        : hough_transform.cpp ( C++ source file )
#
#  Description : Detect Edges of Paper Sheet with Hough Transform
#
#  Copyright   : HYPJUDY ( https://hypjudy.github.io/ )
*/

#include "hough_transform.h"
#include<iostream>
#include<cmath>

/* Constructor */
Hough::Hough(char* filePath) {
	// init
	rgb_img.load_bmp(filePath);
	w = rgb_img.width();
	h = rgb_img.height();
	gray_img = gradients = CImg<double>(w, h, 1, 1, 0);
	hough_space = CImg<double>(180, distance(w, h), 1, 1, 0);

	rgb2gray();
	gray_img.blur(BLUR_SIGMA); // noise reduction
	getGradient();
	gradients.display();
	houghTransform();
	hough_space.display();
	getHoughEdges();
	hough_space.display();
	getLines();
	getCorners();
	displayCornersAndLines();
	rgb_img.display();
}

/* Euclidean distance / Pythagorean Theorem */
double Hough::distance(double diff_x, double diff_y) {
	return sqrt(diff_x * diff_x + diff_y * diff_y);
}

/* RGB to grayscale transformation */
void Hough::rgb2gray() {
	cimg_forXY(rgb_img, x, y) {
		int r = rgb_img(x, y, 0);
		int g = rgb_img(x, y, 1);
		int b = rgb_img(x, y, 2);
		gray_img(x, y) = 0.299 * r + 0.587 * g + 0.114 * b;
	}
}

/* get intensity gradient magnitude for edge detection */
void Hough::getGradient() {
	CImg_3x3(I, double);
	cimg_for3x3(gray_img, x, y, 0, 0, I, double) {
		// one-dimension filter better than 2D(sobel etc)
		gradients(x, y) = distance(Inc - Ipc, Icp - Icn);
	}
}

/* Transform points in parameter space to hough space */
void Hough::houghTransform() {
	cimg_forXY(gradients, x, y) {
		// consider only strong edges, 
		// also helps to reduce the number of votes
		if (gradients(x, y) > GRAD_THRESHOLD) {
			cimg_forX(hough_space, angle) {
				double theta = 1.0 * angle * cimg::PI / 180.0;
				int rho = (int)(x*cos(theta) + y*sin(theta));
				if (rho >= 0 && rho < hough_space.height())
					++hough_space(angle, rho);
			}
		}
	}
}

/* Find out four edges of paper sheet in parameter space 
*  => Get four clusters with the highest values and 
*  select the brighest point from each of them.
*/
void Hough::getHoughEdges() {
	int maxVal = hough_space.max();
	int threshold = floor(maxVal / sqrt(2)) - BIAS;
	std::cout << maxVal << " " << threshold << std::endl;
	cimg_forXY(hough_space, angle, rho) {
		int val = hough_space(angle, rho);
		if (val < threshold){
			hough_space(angle, rho) = 0;
	    }
		else {
			HoughEdge hough_edge(angle, rho, val);
			bool is_new_corner = true;
			for (int i = 0; i < hough_edges.size(); ++i) {
				if (distance(hough_edges[i].angle - angle,
						hough_edges[i].rho - rho) < SCOPE) {
					is_new_corner = false;
					// compare with the other value in this cluster
					if (val > hough_edges[i].val) {
						hough_edges[i] = hough_edge; // update
						break;
					}
				}
			}
			if (is_new_corner) hough_edges.push_back(hough_edge);
		}
	}
}

/* Transform the points in hough space to lines in parameter space */
void Hough::getLines() {
	for (int i = 0; i < hough_edges.size(); ++i) {
		if (hough_edges[i].angle == 0) { // perpendicular to x axis
			lines.push_back(Line(0, 0, hough_edges[i].rho));
			continue;
		}
		double theta = 1.0 * hough_edges[i].angle * cimg::PI / 180.0;
		double m = -cos(theta) / sin(theta);
		double b = 1.0 * hough_edges[i].rho / sin(theta);
		lines.push_back(Line(m, b));
	}
}

/* Get four corners of paper sheet by calculate 
*  the intersections of four lines.
*/
void Hough::getCorners() {
	int x, y;
	double m0, m1, b0, b1;
	for (int i = 0; i < lines.size(); ++i) { // for each line i
		for (int j = 0; j < lines.size(); ++j) { // intersect with line j
			if (j == i || lines[i].end_point_num >= 2 // at most two end points
				|| lines[i].dist_o > 0 && lines[j].dist_o > 0)  // both vertical
				continue;
			m0 = lines[i].m;
			b0 = lines[i].b;
			m1 = lines[j].m;
			b1 = lines[j].b;
			if (lines[i].dist_o > 0) { // line i vertical
				x = lines[i].dist_o;
				y = m1 * x + b1;
			}
			else if (lines[j].dist_o > 0) { // line j vertical
				x = lines[j].dist_o;
				y = m0 * x + b0;
			}
			else { // not vertical
				x = (b1 - b0) / (m0 - m1);
				y = (m0 * x + b0 + m1 * x + b1) / 2; // ensure align at one point
			}
			if (x >= 0 && x < w && y >= 0 && y < h) {
				if (lines[i].end_point_num == 0) { // first end point
					lines[i].x0 = x;
					lines[i].y0 = y;
				}
				else if (lines[i].end_point_num == 1) { // second end point
					lines[i].x1 = x;
					lines[i].y1 = y;
				}
				++lines[i].end_point_num;
			}
		}
	}
}

/* draw and print corners and lines in original image */
void Hough::displayCornersAndLines() {
	for (int i = 0; i < lines.size(); ++i) {
		// draw
		const unsigned char color_red[] = { 255,0,0 };
		const unsigned char color_yellow[] = { 255,255,0 };
		rgb_img.draw_line(lines[i].x0, lines[i].y0,
			lines[i].x1, lines[i].y1, color_red);
		rgb_img.draw_circle(lines[i].x0, lines[i].y0, 5, color_yellow);
		rgb_img.draw_circle(lines[i].x1, lines[i].y1, 5, color_yellow);

		// print
		if (lines[i].dist_o > 0) {
			std::cout << "Line " << i << ": x = " << lines[i].dist_o << std::endl;
		}
		else {
			char op = lines[i].b > 0 ? '+' : '-';
			std::cout << "Line " << i << ": y = " << lines[i].m
				<< "x " << op << abs(lines[i].b) << std::endl;
		}
		std::cout << "Two end points of line "<< i << ": (" << lines[i].x0 <<
			", " << lines[i].y0 << "), (" << lines[i].x1 <<
			", " << lines[i].y1 << ")" << std::endl;
	}
}
