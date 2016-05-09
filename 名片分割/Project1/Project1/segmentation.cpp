#include <iostream>
#include <cstdio>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <cmath>

using namespace cv;
using namespace std;

const int L = 256;

int otsu(Mat& grayImg) {
    // Calculate the frequency of each value
    // Ref: http://stackoverflow.com/questions/21287082/accessing-certain-pixels-intensity-valuegrayscale-image-in-opencv
    double frequency[L];
    memset(frequency, 0, sizeof(frequency));
    for ( int j = 0; j < grayImg.rows; ++j ) {
        for ( int i = 0; i < grayImg.cols; ++i ) {
            ++frequency[grayImg.at<uchar>(j, i)];
        }
    }

    const int total = grayImg.rows * grayImg.cols;
    for ( int i = 0; i < L; ++i ) {
        frequency[i] /= total; 
    }

    // Calculate cdf for p and i * p
    double culp[L];
    double culip[L];
    culp[0] = frequency[0];
    culip[0] = 0;
    for ( int i = 1; i < L; ++i ) {
        culp[i] = culp[i - 1] + frequency[i];
        culip[i] = culip[i - 1] + i * frequency[i];
    }

    // Find k s.t. max sigmaB
    int maxSigma = 0;
    int atPos;
    for ( int k = 1; k < L - 1; ++k ) {
        double mg = culip[L - 1];
        double sigmaB = pow(mg * culp[k] - culip[k], 2) / (culp[k] * (1 - culp[k]));

        if ( sigmaB > maxSigma ) {
            maxSigma = sigmaB;
            atPos = k;
        }
    }

    cout << "Threshold: " << atPos << endl;

    for ( int j = 0; j < grayImg.rows; ++j ) {
        for ( int i = 0; i < grayImg.cols; ++i ) {
            if ( grayImg.at<uchar>(j, i) >= atPos ) {
                grayImg.at<uchar>(j, i) = 255;
            } else {
                grayImg.at<uchar>(j, i) = 0;
            }
        }
    }

    return atPos;
}


// Ref: https://en.wikipedia.org/wiki/Otsu%27s_method
int main() {
	string argv;
	cin >> argv;


    Mat srcImg, grayImg;
    srcImg = imread(argv);
    cvtColor(srcImg, grayImg, CV_BGR2GRAY);

	// Ref: http://docs.opencv.org/2.4/modules/imgproc/doc/histograms.html?highlight=calchist#calchist
	// Ref: http://opencvexamples.blogspot.com/2013/10/histogram-calculation.html
    MatND hist;
    int hsize[] = {256};
    float range[] = {0, 256};
    const float* ranges[] = {range};
    calcHist(&grayImg, 1, 0, Mat(), hist, 1, hsize, ranges);

    int width = 512;
    int height = 400;
    int bin_w = cvRound((double) width / 256);
 
    Mat histImg(height, width, CV_32FC3, Scalar(0,0,0));
    normalize(hist, hist, 0, histImg.rows, NORM_MINMAX, -1, Mat() );
     
    for( int i = 1; i < 256; ++i ) {
        line(histImg, Point( bin_w * (i - 1), height - cvRound(hist.at<float>(i - 1)) ),
                       Point( bin_w * (i), height - cvRound(hist.at<float>(i))),
                       Scalar(255, 255, 255), 2, 8, 0);
    }

	otsu(grayImg);
    imshow("final", grayImg);

    waitKey(0);

    return 0;
}
