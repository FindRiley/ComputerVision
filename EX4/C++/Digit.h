#pragma once

#include "CImg.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <queue>
#include <vector>

using namespace cimg_library;
using namespace std;
typedef unsigned char uchar;

struct Point {
	int x, y;
	Point(int x_, int y_) {
		x = x_;
		y = y_;
	}
};

struct Rect {
	Point p1, p2;
	Rect():p1(0, 0), p2(0, 0) {}
	Rect(Point p1_, Point p2_): p1(p1_.x, p1_.y), p2(p2_.x, p2_.y) {}
};

/* digit longest str, shortest str on x and y */
struct Maps {
	int digit;
	string y_lstr, y_sstr, x_lstr, x_sstr;
	Maps(): digit(-1), y_sstr(""), y_lstr(""), x_lstr(""), x_sstr("") {}
	Maps(int d, string ly, string sy, string lx, string sx) {
		digit = d;
		y_lstr = ly;
		y_sstr = sy;
		x_lstr = lx;
		x_sstr = sx;
	}
};

class Digit {
public:
	Digit(string);	// constructor
	void ToGrayScale();
	void Segment();
	void Dilate();
	void BlockDetect();
	void DrawBlock();
	void DrawScale();
	void DigitMap();
	void DigitDetect();

	void ShowOriginal();
	void ShowGrayscaled();
	void ShowSegmented();
	void ShowDilated();
	void ShowConBlock();
	void ShowScaled();
	void PrintDigits();

	void SaveGrayScale();
	void SaveSegmented();
	void SaveDilated();
	void SaveConBlock();
	void SaveScaled();

private:
	CImg<uchar> img;	// Original image
	CImg<uchar> grayscaled;
	CImg<uchar> segmented;
	CImg<uchar> dilated;
	CImg<uchar> conBlock;
	CImg<uchar> scaled;

	int T;				// threshold
	int histogram[256];	// histogram of grayscale value
	vector<vector<Point>> conBlocks;
	vector<Rect> rects;
	vector<Rect> scales;
	Rect ruler;
	Maps std_maps[10];
	vector<Maps> dgt_maps;
};
