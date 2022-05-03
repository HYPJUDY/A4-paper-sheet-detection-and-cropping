/*
#  File        : Hough.cpp
#  Description : Detect Edges of Paper Sheet with Hough Transform
#  Copyright   : HYPJUDY 2017/4/6
#  Details     : https://hypjudy.github.io/2017/03/28/cvpr-A4-paper-sheet-detection-and-cropping/
#  Code        : https://github.com/HYPJUDY/A4-paper-sheet-detection-and-cropping
*/

#include "Hough.h"
#include<cmath>
#include<algorithm>

/* Compare function for HoughEdge sort.
The strongest edge rank first. */
bool cmp_edges_val(HoughEdge e1, HoughEdge e2) {
	return e1.val > e2.val;
}

/* Compare function for HoughEdge sort.
The edge with smallest slope rank first. */
bool cmp_edges_angle(HoughEdge e1, HoughEdge e2) {
	return e1.angle < e2.angle;
}

/* Compare function for corners sort.The corner
closest to original point rank first. */
bool cmp_corners(Point c1, Point c2) {
	return (c1.x * c1.x + c1.y * c1.y)
		< (c2.x * c2.x + c2.y * c2.y);
}

/* Compare function for Lines sort.
The line with smallest slope rank first.*/
bool cmp_lines(Line l1, Line l2) {
	return l1.m < l2.m;
}

/* Constructor */
Hough::Hough(char* filePath) {
	// init
	rgb_img.load(filePath);
	w = rgb_img.width();
	h = rgb_img.height();
	gray_img = gradients = CImg<double>(w, h, 1, 1, 0);
	hough_space = CImg<double>(360, distance(w, h), 1, 1, 0);

	rgb2gray();
	gray_img.blur(BLUR_SIGMA).display();// .save("dataset1/blur.bmp");
	getGradient();
	gradients.display();// .save("dataset1/gradient.bmp");
	houghTransform();
	hough_space.display();// .save("dataset1/hough_space.bmp");
	getHoughEdges();
	hough_space.display();// .save("dataset1/hough_space2.bmp");
	getLines();
	getCorners();
	orderCorners();
	displayCornersAndLines();
}

/* Euclidean distance / Pythagorean Theorem */
float Hough::distance(float diff_x, float diff_y) {
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
	CImg_3x3(I, float);
	cimg_for3x3(gray_img, x, y, 0, 0, I, float) {
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
				if (rho >= 0 && rho < hough_space.height()) {
					// By the above calculation, the hough space
					// is not consistent. (left 180 degree and
					// right 180 degree should swap)
					// If not consistent, the points lying in the 
					// split edge will be considered as two different
					// parts which is wrong. So I shift hough space
					// by 180 degree to make it consistent.
					// Then angle should minus 180 in some following
					// calculation. 
					++hough_space((angle + 180) % 360, rho);
				}
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
	int threshold = floor(maxVal / Q);
	std::cout << maxVal << " " << threshold << std::endl;
	cimg_forXY(hough_space, angle, rho) {
		int val = hough_space(angle, rho);
		if (val < threshold || rho == 0) {
			// filter out rho == 0 (intercept == 0)
			hough_space(angle, rho) = 0;
		}
		else {
			HoughEdge hough_edge(angle, rho, val);
			bool is_new_corner = true;
			for (int i = 0; i < hough_edges.size(); ++i) {
				//if (distance(hough_edges[i].angle - angle,
				//	hough_edges[i].rho - rho) < 20) {
				if (abs(hough_edges[i].angle - angle) < SCOPE_ANGLE
					&& abs(hough_edges[i].rho - rho) < SCOPE_RHO) {
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
	if (hough_edges.size() > 4) { // filter out some (maybe not 4) strong edges
		sort(hough_edges.begin(), hough_edges.end(), cmp_edges_val);
		// Some edges like tables edges can be stronger than paper edges.
		// We should leave then here and judge then by geometrical relationship later.
 		// Most of the time only one more edge is confusing
		while (hough_edges.size() > 5) hough_edges.pop_back();
		sort(hough_edges.begin(), hough_edges.end(), cmp_edges_angle);
		if (hough_edges.size() == 5) {
			double dangle = abs(hough_edges[0].angle - hough_edges[1].angle);
			double dangle1 = abs(hough_edges[1].angle - hough_edges[2].angle);
			double dangle2 = abs(hough_edges[2].angle - hough_edges[3].angle);
			double dangle3 = abs(hough_edges[3].angle - hough_edges[4].angle);
			double diff = 2; // three lines are parallel if their angles are in this range
			// find out four sides (two pairs) of quadrilateral
			// each pair is parallel (similar angle) and different intercept (rho)
			if (abs(dangle - dangle1) > diff) {
				if (dangle < dangle1) { // 0 and 1 are a pair
					if (abs(dangle2 - dangle3) > diff) {
						if (dangle2 < dangle3) { // 2 and 3 are a pair
							hough_edges.erase(hough_edges.begin() + 4);
						}
						else { // 3 and 4 are a pair
							hough_edges.erase(hough_edges.begin() + 2);
						}
					}
					else { // angles of 2, 3 and 4 are very similar (parallel)
						// remove the weakest point
						if (hough_edges[2].val < hough_edges[3].val &&
							hough_edges[2].val < hough_edges[4].val)
							hough_edges.erase(hough_edges.begin() + 2);
						else if (hough_edges[3].val < hough_edges[4].val)
							hough_edges.erase(hough_edges.begin() + 3);
						else
							hough_edges.erase(hough_edges.begin() + 4);
					}
				}
				else { // 1 and 2 are a pair; 3 and 4 are a pair
					hough_edges.erase(hough_edges.begin());
				}
			}
			else { // angles of 0, 1 and 2 are very similar (parallel)
				// remove the weakest point
				if (hough_edges[0].val < hough_edges[1].val &&
					hough_edges[0].val < hough_edges[2].val)
					hough_edges.erase(hough_edges.begin());
				else if (hough_edges[1].val < hough_edges[2].val)
					hough_edges.erase(hough_edges.begin() + 1);
				else
					hough_edges.erase(hough_edges.begin() + 2);
			}
		}
		if (hough_edges.size() != 4) {
			std::cout << "ERROR: Bug in function void Hough::getHoughEdges()!\
            Please check the ifelse statement to filter out four hough_edges." << std::endl;
			exit(-2);
		}
	}
	else if (hough_edges.size() < 4) {
		std::cout << "ERROR: Please set parameter Q larger in file \
			'hough_transform.h' to filter out four edges!" << std::endl;
		exit(-1);
	}
}

/* Transform the points in hough space to lines in parameter space */
void Hough::getLines() {
	for (int i = 0; i < hough_edges.size(); ++i) {
		if ((hough_edges[i].angle - 180) == 0) { // perpendicular to x axis
			lines.push_back(Line(0, 0, hough_edges[i].rho));
			continue;
		}
		double theta = 1.0 * (hough_edges[i].angle - 180) * cimg::PI / 180.0;
		double m = -cos(theta) / sin(theta);
		double b = 1.0 * hough_edges[i].rho / sin(theta);
		lines.push_back(Line(m, b));
	}
}

/* Get four corners of paper sheet by calculate
*  the intersections of four lines. */
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
				double _x = (b1 - b0) / (m0 - m1);
				x = int(_x);
				// !! Use higher precision _x but not x to calculate y
				y = (m0 * _x + b0 + m1 * _x + b1) / 2; // ensure align at one point
			}

			/* Sometimes the intersects of lines are out of image bound
			*  due to part of paper sheet image or not well aligned lines.
			*  Setting them to the border of image can solve this
			*  problem to some extent. */
			if (x >= 0 - D && x < w + D && y >= 0 - D && y < h + D) {
				if (x < 0) x = 0; else if (x >= w) x = w - 1;
				if (y < 0) y = 0; else if (y >= h) y = h - 1;
				if (lines[i].end_point_num == 0) { // first end point
					lines[i].x0 = x;
					lines[i].y0 = y;
				}
				else if (lines[i].end_point_num == 1) { // second end point
					lines[i].x1 = x;
					lines[i].y1 = y;
				}
				corners.push_back(Point(x, y));
				++lines[i].end_point_num;
			}
		}
	}
}

void Hough::orderCorners() {
	// Usually, if paper sheet is placed vertically(do not need strictly)
	// corners are ordered in top-left, top-right, bottom-left, bottom-right
	//  position by sorting (compare by the distance from original point)
	// Note: original point is in the top-left of image
	sort(corners.begin(), corners.end(), cmp_corners);
	for (int i = 0; i < corners.size(); i += 2)
		ordered_corners.push_back(Point(corners[i].x, corners[i].y));
	
	if (ordered_corners.size() < 4) {
		std::cout << "ERROR: Can not detect four ordered_corners in function \
        void Hough::orderCorners(). Please try to adjust parameters." << std::endl;
		exit(-3);
	}
	x1 = ordered_corners[0].x, y1 = ordered_corners[0].y; // top-left
	x2 = ordered_corners[1].x, y2 = ordered_corners[1].y; // top-right
	x3 = ordered_corners[2].x, y3 = ordered_corners[2].y; // bottom-left
	x4 = ordered_corners[3].x, y4 = ordered_corners[3].y; // bottom-right
	// fine tuning the corners to white paper sheet if not
	const int SHIFT = 3;
	if (rgb_img(x1, y1) < 125) {
		x1 += SHIFT;
		y1 += SHIFT;
	}
	if (rgb_img(x2, y2) < 125) {
		x2 -= SHIFT;
		y2 += SHIFT;
	}
	if (rgb_img(x3, y3) < 125) {
		x3 += SHIFT;
		y3 -= SHIFT;
	}
	if (rgb_img(x4, y4) < 125) {
		x4 -= SHIFT;
		y4 -= SHIFT;
	}
	
	// If horizontally or x1 > x2 (means top-left is in the right 
	// of top-right), top-left and bottom-left corners,
	// top-right and bottom-right corners need swapping.
	// If not, it seems like look from the back of the paper
	// I roughly judge it by image's width and height
	// but not paper sheet's for convenience.
	if (w > h || x1 > x2) { 
		int tmpx = x2, tmpy = y2;
		x2 = x1, y2 = y1;
		x1 = tmpx, y1 = tmpy;
		tmpx = x4, tmpy = y4;
		x4 = x3, y4 = y3;
		x3 = tmpx, y3 = tmpy;
	}
	ordered_corners[0].x = x1, ordered_corners[0].y = y1;
	ordered_corners[1].x = x2, ordered_corners[1].y = y2;
	ordered_corners[2].x = x3, ordered_corners[2].y = y3;
	ordered_corners[3].x = x4, ordered_corners[3].y = y4;
}

/* draw and print corners and lines in original image */
void Hough::displayCornersAndLines() {
	marked_img = getRGBImg();
	// draw
	const unsigned char color_red[] = { 255,0,0 };
	const unsigned char color_yellow[] = { 255,255,0 };
	for (int i = 0; i < lines.size(); ++i) {
		marked_img.draw_line(lines[i].x0, lines[i].y0,
		lines[i].x1, lines[i].y1, color_red);
		marked_img.draw_circle(lines[i].x0, lines[i].y0, 5, color_yellow);
		marked_img.draw_circle(lines[i].x1, lines[i].y1, 5, color_yellow);

		// print
		if (lines[i].dist_o > 0) {
			std::cout << "Line " << i << ": x = " << lines[i].dist_o << std::endl;
		}
		else {
			char op = lines[i].b > 0 ? '+' : '-';
			std::cout << "Line " << i << ": y = " << lines[i].m
				<< "x " << op << abs(lines[i].b) << std::endl;
		}
		std::cout << "Two end points of line " << i << ": (" << lines[i].x0 <<
			", " << lines[i].y0 << "), (" << lines[i].x1 <<
			", " << lines[i].y1 << ")" << std::endl;
	}
}
