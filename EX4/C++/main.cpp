#include "Digit.h"

int main() {
	Digit image("Test/H.bmp");
	image.ToGrayScale();
	//image.SaveGrayScale();

	image.Segment();				// 
	//image.SaveSegmented();

	image.Dilate();
	image.ShowDilated();
	//image.SaveDilated();

	image.BlockDetect();
	image.DrawBlock();
	image.ShowConBlock();
	//image.SaveConBlock();

	image.DrawScale();
	image.ShowScaled();
	//image.SaveScaled();

	image.DigitMap();
	image.DigitDetect();
	image.PrintDigits();

	system("pause");
	return 0;
}