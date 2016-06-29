#ifndef SCHEADER
#define SCHEADER

#include <iostream>
#include <cstdio>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <cmath>

using namespace cv;
using namespace std;

struct myLine;
bool lineCmp(const Vec2f& a, const Vec2f& b);
bool pointCmp(const Point2f& a, const Point2f& b);
Mat skewCorrection(Mat srcImg);

#endif
