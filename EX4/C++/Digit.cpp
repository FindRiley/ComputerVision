#include "Digit.h"

Digit::Digit(string imgName) {
	img.load_bmp(imgName.c_str());
	if (!img.data()) {
		cout << "Could not open or find this image." << endl;
		exit(1);
	}
	T = 128;

	std_maps[0] = Maps(0, "121\0",		"121\0",		"121\0",		"121\0");
	std_maps[1] = Maps(1, "121\0",		"1\0",			"121\0",		"21\0");
	std_maps[2] = Maps(2, "121\0",		"121\0",		"2321\0",		"232\0");
	std_maps[3] = Maps(3, "12121\0",	"12121\0",	"12312\0",	"232\0");
	std_maps[4] = Maps(4, "121\0",		"121\0",		"121\0",		"121\0");
	std_maps[5] = Maps(5, "121\0",		"121\0",		"12321\0",	"232\0");
	std_maps[6] = Maps(6, "12121\0",	"121\0",		"1321\0",		"132\0");
	std_maps[7] = Maps(7, "1\0",			"1\0",			"121\0",		"21\0");
	std_maps[8] = Maps(8, "12121\0",	"12121\0",	"123121\0", "232\0");
	std_maps[9] = Maps(9, "12121\0",	"121\0",		"1231\0",		"231\0");
}

void  Digit::ToGrayScale() {
	grayscaled = CImg<uchar>(img.width(), img.height(), 1, 1);
	cimg_forXY(grayscaled, x, y) {
		double newValue = (img(x, y, 0) * 306 + img(x, y, 1) * 601 + img(x, y, 2) * 117) >> 10;
		grayscaled(x, y) = newValue;
	}
}

void Digit::Segment() {
	memset(histogram, 0, sizeof(histogram));
	segmented = CImg<uchar>(img.width(), img.height(), 1, 1, 0);
	cimg_forXY(grayscaled, x, y) {
		++histogram[grayscaled(x, y)];
	}

	int deltaT = 50, newT = 127;
	long long sum_low, sum_high;
	int count_low, count_high;
	T = 128;
	while (deltaT > 3) {
		if (T < 0 || T > 255) {
			cout << "Invalid threshold: " << T << endl;
			exit(2);
		}

		cout << "T: " << T << " newT: ";
		sum_low = sum_high = count_low = count_high = 0;
		for (int i = 0; i < T; ++i) {			// 将灰度低于 T 的像素点的灰度值求和
			sum_low += histogram[i] * i;
			count_low += histogram[i];
		}
		for (int i = T; i < 256; ++i) {		// 将灰度高于 T 的像素点的灰度值求和
			sum_high += histogram[i] * i;
			count_high += histogram[i];
		}
		if (count_low == 0) {
			T = sum_high / count_high;
			deltaT = 50;
		}
		else if (count_high == 0) {
			T = sum_low / count_low;
			deltaT = 50;
		}
		else {
			newT = (sum_low / count_low + sum_high / count_high) / 2;
			deltaT = abs(newT - T);
			T = newT;
		}
		cout << newT << " deltaT: " << deltaT << endl;
	}

	cimg_forXY(grayscaled, x, y) {
		if (grayscaled(x, y) >= T) {
			segmented(x, y) = 255;
		}
	}
}

void Digit::Dilate() {
	dilated = CImg<uchar>(grayscaled.width(), grayscaled.height(), 1, 1);
	int min;
	cimg_forXY(grayscaled, x, y) {
		min = 255;
		for (int i = -1; i <= 0; ++i)
			for (int j = -1; j <= 0; ++j)
				if (x + i >= 0 && x + i < grayscaled.width()
						&& y + j >= 0 && y + j < grayscaled.height()
						&& grayscaled(x + i, y + j) < min) {
					min = grayscaled(x + i, y + j);
				}
		dilated(x, y) = min;
	}
}

bool cmpX(Point a, Point b) {
	return a.x < b.x;
}

bool cmpY(Point a, Point b) {
	return a.y < b.y;
}

void Digit::BlockDetect() {
	queue<Point> q_points;
	CImg<uchar> temp = dilated;
	int count = 0;				// count connected blocks
	cimg_forXY(temp, x, y) {
		if (temp(x, y) < 200) {
			temp(x, y) = 255;			// only check once
			conBlocks.push_back(vector<Point>());
			q_points.push(Point(x, y));

			while (!q_points.empty()) {
				int col = q_points.front().x;
				int row = q_points.front().y;
				q_points.pop();
				conBlocks[count].push_back(Point(col, row));

				for (int i = -1; i <= 1; ++i)			// check 8 neighbors
					for (int j = -1; j <= 1; ++j)
						if (col + i > 0 && col + i < temp.width()
								&& row + j > 0 && row + j < temp.height()
								&& temp(col + i, row + j) < 200) {
							temp(col + i, row + j) = 255;
							q_points.push(Point(col + i, row + j));
						}
			}
			if (conBlocks[count].size() < 9 || conBlocks[count].size() > 10000) {
				conBlocks.erase(conBlocks.begin() + count);
			}
			else {
				++count;
			}
		}
	}
}

void Digit::DrawBlock() {
	conBlock = img;
	int x1, x2, y1, y2;
	for (int i = 0; i < conBlocks.size(); ++i) {		// get block rectangle
		sort(conBlocks[i].begin(), conBlocks[i].end(), cmpX);
		x1 = conBlocks[i][0].x;
		x2 = conBlocks[i][conBlocks[i].size() - 1].x;

		sort(conBlocks[i].begin(), conBlocks[i].end(), cmpY);
		y1 = conBlocks[i][0].y;
		y2 = conBlocks[i][conBlocks[i].size() - 1].y;
		rects.push_back(Rect(Point(x1, y1), Point(x2, y2)));
	}
	for (int i = 0; i < rects.size(); ++i) {
		x1 = rects[i].p1.x;
		x2 = rects[i].p2.x;
		y1 = rects[i].p1.y;
		y2 = rects[i].p2.y;

		if (x2 - x1 > img.width() / 3 && y2 - y1 < 25) { // get ruler
			ruler.p1 = Point(x1, y1);
			ruler.p2 = Point(x2, y2);
			cout << endl << "ruler: " << "x1,y1: " << x1 << "," << y1 << "  x2,y2: " << x2 << "," << y2 << endl;
			continue;
		}
		if (x2 - x1 > 15 || y2 - y1 > 20 
			|| (y2 - y1) / (x2 - x1) > 3 || (x2 - x1) / (y2 - y1) > 3)
			continue;

		scales.push_back(rects[i]);					// store number block

		for (int j = x1; j <= x2; ++j) {
			conBlock(j, y1, 0) = 255;
			conBlock(j, y1, 1) = conBlock(j, y1, 2) = 0;
			conBlock(j, y2, 0) = 255;
			conBlock(j, y2, 1) = conBlock(j, y2, 2) = 0;
		}
		for (int j = y1; j <= y2; ++j) {
			conBlock(x1, j, 0) = 255;
			conBlock(x1, j, 1) = conBlock(x1, j, 2) = 0;
			conBlock(x2, j, 0) = 255;
			conBlock(x2, j, 1) = conBlock(x2, j, 2) = 0;
		}
	}
}

void Digit::DrawScale() {
	vector<Rect>::iterator it;
	int maxY = ruler.p2.y;
	cout << endl << "scaleSize: " << scales.size() << endl;
	for (it = scales.begin(); it != scales.end();) {
		if (it->p1.y - ruler.p2.y > -1 && it->p1.y - ruler.p2.y < 15) {
			++it;
			if (it->p2.y > maxY) maxY = it->p2.y;
		}
		else {
			it = scales.erase(it);
		}
	}
	cout << endl << "scaleSize: " << scales.size() << endl;

	ruler.p2.y = maxY;
	
	int x1, x2, y1, y2;
	x1 = ruler.p1.x - 5 >= 0 ? ruler.p1.x - 5 : ruler.p1.x;
	x2 = ruler.p2.x + 5 < img.width() ? ruler.p2.x + 5 : ruler.p2.x;
	y1 = ruler.p1.y - 5 >= 0 ? ruler.p1.y - 5 : ruler.p1.y - 1;
	y2 = ruler.p2.y + 5 < img.height() ? ruler.p2.y + 5 : ruler.p2.y;

	scaled = img;
	for (int i = x1; i <= x2; ++i) {
		scaled(i, y1, 0) = scaled(i, y2, 0) = 0;
		scaled(i, y1, 1) = scaled(i, y2, 1) = 0;
		scaled(i, y1, 2) = scaled(i, y2, 2) = 255;
	}
	for (int i = y1; i < y2; ++i) {
		scaled(x1, i, 0) = scaled(x2, i, 0) = 0;
		scaled(x1, i, 1) = scaled(x2, i, 1) = 0;
		scaled(x1, i, 2) = scaled(x2, i, 2) = 255;
	}
}

bool cmpX2(Rect a, Rect b) {
	return a.p1.x < b.p1.x;
}

void Digit::DigitMap() {
	int count;
	sort(scales.begin(), scales.end(), cmpX2);
	for (int i = 0; i < scales.size(); ++i) {
		dgt_maps.push_back(Maps());
		for (int j = scales[i].p1.y; j <= scales[i].p2.y; ++j) {
			count = 0;
			for (int k = scales[i].p1.x; k <= scales[i].p2.x;) {
				if (grayscaled(k, j) < 200) {
					++count;
					while (++k <= scales[i].p2.x && grayscaled(k, j) < 200);
				}
				++k;
			}
			if (count > 0 && (dgt_maps[i].y_lstr.size() == 0 || count + 48 != dgt_maps[i].y_lstr[dgt_maps[i].y_lstr.size() - 1])) {
				dgt_maps[i].y_lstr += to_string(count);
			}
		}
	}

	for (int i = 0; i < scales.size(); ++i) {
		for (int j = scales[i].p1.x; j <= scales[i].p2.x; ++j) {
			count = 0;
			for (int k = scales[i].p1.y; k <= scales[i].p2.y;) {
				if (grayscaled(j, k) < 200) {
					++count;
					while (++k < scales[i].p2.y && grayscaled(j, k) < 200);
				}
				++k;
			}
			if (count > 0 && (dgt_maps[i].x_lstr.size() == 0 || count + 48 != dgt_maps[i].x_lstr[dgt_maps[i].x_lstr.size() - 1])) {
				dgt_maps[i].x_lstr += to_string(count);
			}
		}
	}
}

void Digit::DigitDetect() {
	cout << "dgt_map size: " << dgt_maps.size() << endl;
	for (int i = 0; i < dgt_maps.size(); ++i) {
		//cout << "maps: " << dgt_maps[i].x_lstr.length() << "," << dgt_maps[i].y_lstr.length() << "  " << dgt_maps[i].x_lstr << " " << dgt_maps[i].y_lstr << endl;
		int max_match = 0, cur_match = 0, number = -1;
		for (int j = 0; j < 10; ++j) {
			cur_match = 0;
			if (string::npos != std_maps[j].x_lstr.find(dgt_maps[i].x_lstr))
				cur_match += dgt_maps[i].x_lstr.length();
			if (string::npos != dgt_maps[i].x_lstr.find(std_maps[j].x_sstr))
				cur_match += std_maps[j].x_sstr.length();
			if (string::npos != std_maps[j].y_lstr.find(dgt_maps[i].y_lstr))
				cur_match += dgt_maps[i].y_lstr.length();
			if (string::npos != dgt_maps[i].y_lstr.find(std_maps[j].y_sstr))
				cur_match += std_maps[j].y_sstr.length();

			if (cur_match > max_match) {
				max_match = cur_match;
				number = j;
			}
		}

		if (number == 0 && grayscaled(scales[i].p2.x - 1, (scales[i].p1.y + scales[i].p2.y) / 2) > 200) {
			if (grayscaled(scales[i].p1.x + 1, scales[i].p2.y - 1) > 200)
				number = 4;
			else number = 1;
		}
		else if (number == 4 && grayscaled(scales[i].p2.x - 1, (scales[i].p1.y + scales[i].p2.y) / 2) < 200)
			number = 0;
		else if (number == 2 && grayscaled(scales[i].p2.x - 1, (scales[i].p1.y + scales[i].p2.y) / 2 + 1) < 200)
			number = 5;
		else if (number == 5 && grayscaled(scales[i].p2.x - 1, (scales[i].p1.y + scales[i].p2.y) / 2 + 1) > 200)
			number = 2;
		else if (number == 3 && grayscaled(scales[i].p1.x + 1, (scales[i].p1.y + scales[i].p2.y) / 2) < 200)
			number = 8;
		else if (number == 8 && grayscaled(scales[i].p1.x + 1, (scales[i].p1.y + scales[i].p2.y) / 2) > 200)
			number = 3;

		dgt_maps[i].digit = number;
	}
}

void Digit::ShowOriginal() {
	img.display("Original");
}

void Digit::ShowGrayscaled() {
	grayscaled.display("grayscaled");
}

void Digit::ShowSegmented() {
	segmented.display("segmented");
}

void Digit::ShowDilated() {
	dilated.display("dilated");
}

void Digit::ShowConBlock() {
	conBlock.display("conBlock");
}

void Digit::ShowScaled() {
	scaled.display("scaled");
}

void Digit::PrintDigits() {
	cout << endl;
	for (int i = 0; i < dgt_maps.size(); ++i) {
		cout << dgt_maps[i].digit << "  ";
	}
	cout << endl;
}

void Digit::SaveGrayScale() {
	grayscaled.save("Result/grayscaled.bmp");
}

void Digit::SaveSegmented() {
	segmented.save("Result/segmented.bmp");
}

void Digit::SaveDilated() {
	dilated.save("Result/dilated.bmp");
}

void Digit::SaveConBlock() {
	conBlock.save("Result/conBlock.bmp");
}

void Digit::SaveScaled() {
	scaled.save("Result/scaled.bmp");
}