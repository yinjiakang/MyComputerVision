#include "segmentation.h"

void stripSegmentation(Mat grayImg, Mat* retImg, int& size) {
	Mat binarizedImg;

	adaptiveThreshold(grayImg, binarizedImg, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 35, 27);

	int frequency[500];
	memset(frequency, 0, sizeof(frequency));


	// Calculate the frequency of white pixels for each col.
	for (int j = 0; j < binarizedImg.rows; ++j) {
		for (int i = 0; i < binarizedImg.cols; ++i) {
			if (binarizedImg.at<uchar>(j, i) == 255) {
				++frequency[i];
			}
		}
	}


	// Region segmentation
	const int valueThreshold = 2;
	const int timesThreshold = 5;

	int contLowTimes[500];
	memset(contLowTimes, 0, sizeof(contLowTimes));
	contLowTimes[0] = 1;
	for (int i = 1; i < binarizedImg.cols; ++i) {
		if (frequency[i] <= valueThreshold) {
			contLowTimes[i] = contLowTimes[i - 1] + 1;
		}
		else {
			contLowTimes[i] = 0;
		}
	}

	for (int i = 0; i < binarizedImg.cols - 1; ++i) {
		if (contLowTimes[i] <= timesThreshold && contLowTimes[i + 1] == 0) {
			for (int j = i; j >= 0; --j) {
				if (contLowTimes[j] == 0) {
					break;
				}
				contLowTimes[j] = 0;
			}
		}
	}



	vector<int> pos;
	for (int i = 1; i < binarizedImg.cols; ++i) {
		if (contLowTimes[i] == 0 && contLowTimes[i - 1] != 0) {
			pos.push_back(i);
		}
		else if (contLowTimes[i] != 0 && contLowTimes[i - 1] == 0) {
			pos.push_back(i - 1);
		}
	}

	cout << "=====Dividing Point=====" << endl;
	for (int i = 0; i < pos.size(); ++i) {
		cout << pos[i] << " ";
	}
	cout << endl << endl;

	const int extendedBoundary = 0;
	size = pos.size() / 2;
	Mat croppedImg[5];

	for (int i = 0; i < size; ++i) {
		grayImg(Rect(pos[i * 2] - extendedBoundary, 0,
			pos[i * 2 + 1] - pos[i * 2] + extendedBoundary, grayImg.rows)).copyTo(croppedImg[i]);

		// Rotate 90 degrees clockwise.
		retImg[i] = rotation(croppedImg[i], 270);
	}
}

bool cmp(Rect a, Rect b) {
	return a.x < b.x;
}

void characterSegmentation(Mat* srcImg, int numberOfStrips, vector<vector<Mat> >& retImg) {
	//cout << "=====Bounding Width=====" << endl;
	const int widthLower = 5;
	const int widthHigher = 100;

	for (int i = 0; i < numberOfStrips; ++i) {
		vector<Mat> partialVector;
		Mat stripImg = srcImg[i].clone();
		//Canny(stripImg, stripImg, 100, 200, 3);
		adaptiveThreshold(stripImg, stripImg, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 35, 27);

		int size = 1;
		Mat element = getStructuringElement(MORPH_RECT,
			Size(2 * size + 1, 2 * size + 1),
			Point(size, size));
		dilate(stripImg, stripImg, element);

		vector<vector<Point> > contours;
		// Ref: http://docs.opencv.org/2.4/modules/imgproc/doc/structural_analysis_and_shape_descriptors.html?highlight=findcontours#findcontours
		// This fuction will modify the source image.
		findContours(stripImg, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		vector<Rect> boundingVector;
		for (int j = 0; j < contours.size(); ++j) {
			Rect bounding = boundingRect(contours[j]);
			// http://docs.opencv.org/2.4/modules/core/doc/drawing_functions.html
			// If the width of the region is too big or small, the region will never contain a digit.
			if (bounding.width > widthHigher || bounding.width < widthLower) {
				continue;
			}

			boundingVector.push_back(bounding);

			//drawContours(srcImg[i], contours, j, Scalar(0, 0, 0));
		}

		sort(boundingVector.begin(), boundingVector.end(), cmp);

		for (int j = 0; j < boundingVector.size(); ++j) {

			Mat tmpImg;
			srcImg[i](Rect(boundingVector[j].x, 0, boundingVector[j].width, srcImg[i].rows)).copyTo(tmpImg);
			partialVector.push_back(tmpImg);
		}

		retImg.push_back(partialVector);
	}
}


void segmentation(Mat grayImg, vector<vector<Mat> >& retImg) {
	Mat stripImg[5];
	int numberOfStrips;
	stripSegmentation(grayImg, stripImg, numberOfStrips);
	characterSegmentation(stripImg, numberOfStrips, retImg);
}



// Rotate image
Mat rotation(Mat srcImg, double angle) {
	Mat tempImg;
	float radian = (float)(angle / 180.0 * CV_PI);

	int uniSize = (int)(max(srcImg.cols, srcImg.rows) * 1.414);
	int dx = (int)(uniSize - srcImg.cols) / 2;
	int dy = (int)(uniSize - srcImg.rows) / 2;
	copyMakeBorder(srcImg, tempImg, dy, dy, dx, dx, BORDER_CONSTANT);

	Point2f center((float)(tempImg.cols / 2), (float)(tempImg.rows / 2));
	Mat affine_matrix = getRotationMatrix2D(center, angle, 1.0);

	warpAffine(tempImg, tempImg, affine_matrix, tempImg.size());

	float sinVal = fabs(sin(radian));
	float cosVal = fabs(cos(radian));
	Size targetSize((int)(srcImg.cols * cosVal + srcImg.rows * sinVal),
		(int)(srcImg.cols * sinVal + srcImg.rows * cosVal));


	int x = (tempImg.cols - targetSize.width) / 2;
	int y = (tempImg.rows - targetSize.height) / 2;
	Rect rect(x, y, targetSize.width, targetSize.height);
	tempImg = Mat(tempImg, rect);

	return tempImg;
}

void adjustStyle(vector<vector<Mat> >& srcImg) {
	for (int i = 0; i < srcImg.size(); ++i) {
		// cout << "Width and Height " << srcImg[i][0].cols << " " << srcImg[i][0].rows << endl;
		for (int j = 0; j < srcImg[i].size(); ++j) {
			// Resize to 28 * 28.
			resize(srcImg[i][j], srcImg[i][j], Size(28, 28));

			// Dilate
			int size = 1;
			Mat element = getStructuringElement(MORPH_RECT,
				Size(2 * size + 1, 2 * size + 1),
				Point(size, size));
			dilate(srcImg[i][j], srcImg[i][j], element);

		}
	}
}

vector<vector<Mat> > addBorder(vector<vector<Mat> > srcImg) {
	vector<vector<Mat> > retImg;
	for (int i = 0; i < srcImg.size(); ++i) {
		vector<Mat> partialRetImg;
		for (int j = 0; j < srcImg[i].size(); ++j) {
			int width = srcImg[i][j].cols;
			int height = srcImg[i][j].rows;
			Mat dstImg;

			// Binarized
			adaptiveThreshold(srcImg[i][j], dstImg, 255,
				ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 35, 27);

			if (width < height) {
				copyMakeBorder(dstImg, dstImg, 0, 0, (height - width) / 2, (height - width) / 2,
					BORDER_CONSTANT, Scalar(0, 0, 0));
				partialRetImg.push_back(dstImg);
			}
			else {
				partialRetImg.push_back(srcImg[i][j]);
			}
		}

		retImg.push_back(partialRetImg);
	}

	return retImg;
}


