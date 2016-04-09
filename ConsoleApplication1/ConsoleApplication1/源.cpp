#include <iostream>
#include <cstdio>
#include <string>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


//参考: http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/hough_lines/hough_lines.html

struct Cartesian_Line {
	double k;
	double b;
	bool isExist;
};

bool cmp(const Vec2f& a, const Vec2f& b) {
	double threshold = 0.1;
	if ( abs(a[1] - b[1]) < threshold ) {
		return a[0] < b[0];
	}
	return a[1] < b[1];
}

bool cmp2(const Point2f& a, const Point2f& b) {
	return a.x + a.y < b.x + b.y;
}

int main() {

	Mat srcImg, dstImg, srcGray, srcBlur, edgeImg;

	string name;
	cout << "input the name of the picture" << endl;
	cin >> name;
	srcImg = imread(name);

	// 降低比例
	//if (srcImg.rows < srcImg.cols)
		resize(srcImg, srcImg, Size(), 0.2, 0.2);
	//else
		//resize(srcImg, srcImg, Size(), 0.1, 0.1);

	cvtColor(srcImg, srcGray, CV_BGR2GRAY);
	// 模糊
	blur(srcGray, srcBlur, Size(3, 3));
	// 边缘
	Canny(srcBlur, edgeImg, 120, 500, 3);

	dstImg.create(srcImg.size(), srcImg.type());
	dstImg = Scalar::all(0);
	
	float rho = 1;
	float theta = CV_PI / 100;
	vector<Vec2f> lines;
	vector<Cartesian_Line> cartesian_lines;
	HoughLines(edgeImg, lines, rho, theta, 97, 0, 0);
	sort(lines.begin(), lines.end(), cmp);

	cout << "linesize: " << lines.size() << endl;

	int last = lines.size() - 1;

	// 消除0度 与180度 直线平行的情况
	/*while (lines[0][1] <= 0.1 && lines[last][1] >= 3.1) {
		vector<Vec2f>::iterator it = lines.begin() + 1;
		lines.insert(it, lines[last]);
		it = lines.end() - 1;
		lines.erase(it);
	}*/
	while (lines[0][1] <= 0.1 && lines[last][1] >= 3.1) {
		vector<Vec2f>::iterator it = lines.begin() + 1;
		for (size_t i = 1; i < last; ++i) {
			if (lines[i][1] <= 0.1 && abs(lines[0][0] - lines[i][0]) < 50) {
				++it;
			}
		}
		lines.insert(it, lines[last]);
		it = lines.end() - 1;
		lines.erase(it);
	}

	double degreePerPI = 180 / CV_PI;
	double rotationAngle = lines[0][1] * degreePerPI;

	cout << "xxxxxx:" << lines[0][0] << " " << lines[0][1] << " " << rotationAngle << endl;

	if (lines[0][1] > 0.1) rotationAngle -= 90;

	float centerx = 0, centery = 0;
	// 保存四个点
	vector<Point2f> p;
	int numofpoint = 0;

	for (size_t i = 0; i < lines.size(); i++) {
		float rho = lines[i][0], theta = lines[i][1];

		cout << "rho: " << rho << " " << "theta :" << theta << endl;
		// 消除重复
		float deltaRho = 100;
		float deltaTheta = 0.25;


		/*if (i > 0 && abs(rho - lines[i - 1][0]) < deltaRho &&
			(abs(CV_PI - (rho + lines[i - 1][0])) < deltaRho || abs(theta - lines[i - 1][1]) < deltaTheta)) {
			continue;
		}*/


		if (i > 0 && abs(abs(rho) - abs(lines[i - 1][0])) < deltaRho &&
			(abs(CV_PI - (theta + lines[i - 1][1])) < deltaTheta || abs(theta - lines[i - 1][1]) < deltaTheta)) {
			// When one theta is near PI and the other is near 0 or thetas are very close.
			vector<Vec2f>::iterator it = lines.begin() + i;
			lines.erase(it);
			--i;
			continue;
		}

		// Eliminate wrong lines.
		if (i > 0 && i < lines.size() - 1 &&
			// When the line is not parallel to the two sides
			abs(theta - lines[i - 1][1]) > deltaTheta && abs(theta - lines[i + 1][1]) > deltaTheta &&
			abs(CV_PI - (theta + lines[i - 1][1])) > deltaTheta) {
			// When one theta is near PI and the other is near 0
			vector<Vec2f>::iterator it = lines.begin() + i;
			lines.erase(it);
			--i;
			continue;
		}




		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a * rho, y0 = b * rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		line(dstImg, pt1, pt2, Scalar(0, 0, 255), 3, CV_AA);
		
		struct Cartesian_Line thisline;

		// 斜率不存在
		if (b == 0) {
			thisline.isExist = 1;
			thisline.b = rho;
			cout << "Line : x = " << rho << endl;
		}
		else {
			thisline.isExist = 0;
			double k = -a / b, bb = rho / b;
			thisline.k = k;
			thisline.b = bb;
			cout << "Line : y = " << k << "x + " << bb << endl;
		}

		cartesian_lines.push_back(thisline);
	}

	cout << cartesian_lines.size() << endl;



	// 计算交点
	for (int i = 0; i < 2; ++i) {
		for (int j = 2; j < 4; ++j) {
			double a = cartesian_lines[i].k, b = cartesian_lines[j].k;
			double c = cartesian_lines[i].b, d = cartesian_lines[j].b;
			double x, y;
			if (cartesian_lines[i].isExist == 1) {
				x = c;
				y = b * c + d;
			}
			else {
				x = (d - c) / (a - b);
				y = (a * d - b * c) / (a - b);
			}
			p.push_back(Point2f(x, y));
			centerx += x;
			centery += y;
			numofpoint++;
			line(dstImg, Point2f(x, y), Point2f(x, y), Scalar(0, 255, 0), 10, CV_AA);
			cout << "Point: ( " << x << "," << y << ")" << endl;
		}
	}

	imshow("dstImg", dstImg);
	

	Point center;
	center.x = centerx / 4;
	center.y = centery / 4;
	float scale = 1;
	if (srcImg.rows < srcImg.cols) {
		rotationAngle += 90;
		scale = 0.5;
		cout << "--------in-scale--------" << endl;
	}
	/*cout << "srcImg: " << srcImg.cols << " " << srcImg.rows << endl;
	imshow("srcImg", srcImg);*/
	// 参考: http://www.opencv.org.cn/opencvdoc/2.3.2/html/doc/tutorials/imgproc/imgtrans/warp_affine/warp_affine.html?highlight=warping
	Mat rotatedImg ,rotationMatrix;
	rotationMatrix = getRotationMatrix2D(center, rotationAngle, scale);
	warpAffine(srcImg, rotatedImg, rotationMatrix, rotatedImg.size());
	transform(p, p, rotationMatrix);


	imshow("rotatedImg", rotatedImg);


	cout << "rotatedImg: " << rotatedImg.cols << " " << rotatedImg.rows << endl;
	sort(p.begin(), p.end(), cmp2);
	cout << "Four new points: " << endl;
	for (int i = 0; i < p.size(); ++i) {
		cout << p[i].x << ", " << p[i].y << endl;
	}

	// 计算切割后左上 左下 右上 右下顶点
	float left = rotatedImg.cols, right = 0, top = rotatedImg.rows, bottom = 0;
	for (int i = 0; i < p.size(); ++i) {
		if (left > p[i].x) left = p[i].x;
		if (right < p[i].x) right = p[i].x;
		if (top > p[i].y) top = p[i].y;
		if (bottom < p[i].y) bottom = p[i].y;
	}
	cout << "top, bottom, left, right: " << top << " " << bottom << " " << left << " " << right << endl;
	cout << "left, top: " << left << " " << top << endl;
	cout << "right, top: " << right << " " << top << endl;
	cout << "left, bottom: " << left << " " << bottom << endl;
	cout << "right, bottom: " << right << " " << bottom << endl;

	vector<Point2f> fixpoints;
	fixpoints.push_back(Point2f(left, top));
	fixpoints.push_back(Point2f(right, top));
	fixpoints.push_back(Point2f(left, bottom));
	fixpoints.push_back(Point2f(right, bottom));
	

	Mat perspectiveMatrix = getPerspectiveTransform(p, fixpoints);
	cout << "rotatedImg: " << rotatedImg.cols << " " << rotatedImg.rows << endl;
	float height, width;
	if (rotatedImg.rows < rotatedImg.cols) {
		height = rotatedImg.cols;
		width = rotatedImg.rows;
	}
	else {
		height = rotatedImg.rows;
		width = rotatedImg.cols;
	}
	cout << "height and width: "<< height << " " << width << endl;
	//Mat perspectiveImg(800, 800, rotatedImg.type());
	Mat perspectiveImg(height, width, rotatedImg.type());
	warpPerspective(rotatedImg, perspectiveImg, perspectiveMatrix, perspectiveImg.size());
	imshow("perspectiveImg222", perspectiveImg);

	Mat roi_img = perspectiveImg(Range(top, bottom - 1), Range(left, right - 1));
	imshow("roi_img", roi_img);

	waitKey(0);
	return 0;
}



