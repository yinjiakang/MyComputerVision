#include <iostream>
#include <cstdio>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <cmath>

using namespace cv;
using namespace std;

// Store line info
struct myLine {
    double m; // Slope of the line
    double c; // Intercept of the line
    bool vertical; // True if the line is parallel to y-axis
};

/*
void myHoughLines(const Mat& img, vector<Vec2f>& lines, float rho, float theta, int threshold) {
    float irho = 1 / rho;
    int width = img.size().width;
    int height = img.size().height;  
}
*/

// Line comparison function based on theta
bool lineCmp(const Vec2f& a, const Vec2f& b) {
    double threshold = 0.1;
    if ( abs(a[1] - b[1]) < threshold ) {
        return a[0] < b[0];
    }
    return a[1] < b[1];
}

// Point comparison function
bool pointCmp(const Point2f& a, const Point2f& b) {
    return a.x + a.y < b.x + b.y;
}

// Ref: http://docs.opencv.org/2.4/doc/tutorials/introduction/linux_gcc_cmake/linux_gcc_cmake.html#linux-gcc-usage
int main(int argc, char* argv[]) {
    if ( argc != 2 ) {
        cout << "Please specify the path of the photo you want to test!" << endl;
        return -1;
    }

    Mat srcImg, dstImg, srcGray, srcBlur, edge;
    srcImg = imread(argv[1]);
    if ( !srcImg.data ) {
        cout << "No image data" << endl;
        return -1;
    }

    // Ref: http://docs.opencv.org/2.4/modules/imgproc/doc/geometric_transformations.html#resize
    resize(srcImg, srcImg, Size(), 0.2, 0.2);

    // Ref: http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/canny_detector/canny_detector.html
    // Transfrom to gray scale.
    cvtColor(srcImg, srcGray, CV_BGR2GRAY);

    // Blur the image to erase some noise
    blur(srcGray, srcBlur, Size(3, 3));

    // Set threshold for Canny
    const int lowThreshold = 125;
    const int highThreshold = 500;
    const int kernel_size = 3;
    Canny(srcBlur, edge, lowThreshold, highThreshold, kernel_size);

    // Ref: http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/hough_lines/hough_lines.html
    // Set parameters for hough transform
    const float rho = 1;
    const float theta = CV_PI / 100;
    const int threshold = 97;
    vector<Vec2f> lines;
    vector<myLine> mls;

    HoughLines(edge, lines, rho, theta, threshold, 0, 0);
    // myHoughLines(edge, lines, rho, theta, threshold);
    
    // Sort lines based on theta
    sort(lines.begin(), lines.end(), lineCmp);
    // Move lines with theta near 3.1 to the front
    int last = lines.size() - 1;
    while ( lines[0][1] <= 0.1 && lines[last][1] >= 3.1 ) {
        vector<Vec2f>::iterator it = lines.begin() + 1;
        lines.insert(it, lines[last]);
        it = lines.end() - 1;
        lines.erase(it);
    }

    double rotationAngle = lines[0][1] * 180 / CV_PI;
    double rotationThreshold = 0.1;
    // Rotate 90 degree clockwise for horizontal papers, e.g. 2.jpg and 4.jpg
    if ( lines[0][1] > rotationThreshold ) {
        rotationAngle -= 90;
    }

    for ( size_t i = 0; i < lines.size(); i++ ) {
        float rho = lines[i][0], theta = lines[i][1];

        // Eliminate duplicates.
        const float deltaRho = 100;
        const float deltaTheta = 0.25;

        if ( i > 0 && abs(rho - lines[i - 1][0]) < deltaRho &&
            ( abs(CV_PI - (rho + lines[i - 1][0])) < deltaRho || abs(theta - lines[i - 1][1]) < deltaTheta ) ) {
            continue;
        }
        cout << "rho: " << rho << "  theta: " << theta << endl;

        // Produce two lines on the line
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        pt1.x = cvRound(x0 + 1000 * (-b));
        pt1.y = cvRound(y0 + 1000 * (a));
        pt2.x = cvRound(x0 - 1000 * (-b));
        pt2.y = cvRound(y0 - 1000 * (a));
        // Draw red lines
        line(srcImg, pt1, pt2, Scalar(0, 0, 255), 3, CV_AA);

        struct myLine ml;
        // Deal with lines parallel to y-axis.
        if ( b == 0 ) {
            ml.vertical = 1;
            ml.c = rho;
            printf("Line equation: x = %lf\n", rho);
        } else {
            ml.vertical = 0;
            // m for slope and c for y-intercept
            double m = -a / b, c = rho / b;
            ml.m = m;
            ml.c = c;
            printf("Line equation: y = (%lfx) + (%lf)\n", m ,c);
        }
        
        mls.push_back(ml);
    }

    vector<Point2f> pts;
    // Calculate line-line intersection.
    // Ref: https://en.wikipedia.org/wiki/Line–line_intersection
    for ( int i = 0; i < 2; ++i ) {
        for ( int j = 2; j < 4; ++j ) {
            double a = mls[i].m, b = mls[j].m;
            double c = mls[i].c, d = mls[j].c;
            double x, y;
            // Slope = inf
            if ( mls[i].vertical == 1 ) {
                x = c;
                y = b * c + d;
            } else {
                x = (d - c) / (a - b);
                y = (a * d - b * c) / (a - b);
            }
            printf("Point: (%lf, %lf)\n", x, y);
            Point2f pt1;
            pt1.x = x;
            pt1.y = y;
            pts.push_back(pt1);
            // Draw blue points
            line(srcImg, pt1, pt1, Scalar(255, 0, 0), 12, CV_AA);
        }
    }

    // Calculate the center of the paper
    Point center;
    center.x = (pts[0].x + pts[1].x + pts[2].x + pts[3].x) / 4;
    center.y = (pts[0].y + pts[1].y + pts[2].y + pts[3].y) / 4;
    line(srcImg, center, center, Scalar(0, 255, 0), 12, CV_AA);

    Mat rotatedImg;
    Mat rotationMatrix;

    cout << "Rotation angle: " << rotationAngle << endl;

    // Ref: http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/warp_affine/warp_affine.html
    rotationMatrix = getRotationMatrix2D(center, rotationAngle, 1);
    warpAffine(srcImg, rotatedImg, rotationMatrix, rotatedImg.size() );
    // Map the points to the rotated image
    transform(pts, pts, rotationMatrix);

    if ( srcImg.rows < srcImg.cols ) {
        // Rotate 90 degree clockwise for horizontal papers and diminish their sizes
        const float scale = 0.7;
        rotationMatrix = getRotationMatrix2D(center, 90, scale);
        warpAffine(rotatedImg, rotatedImg, rotationMatrix, rotatedImg.size() );
        transform(pts, pts, rotationMatrix);
    }

    // Sort the points to determine the position of each point
    sort(pts.begin(), pts.end(), pointCmp);
    cout << "Four new points: " << endl;
    for ( int i = 0; i < pts.size(); ++i ) {
        cout << pts[i].x << ", " << pts[i].y << endl;
    }

    // Set standard points
    const int lx = 130;
    const int ly = 160;
    const int rx = 510;
    const int ry = 720;
    Point2f topLeft;
    topLeft.x = lx;
    topLeft.y = ly;
    Point2f topRight;
    topRight.x = rx;
    topRight.y = ly;
    Point2f bottomLeft;
    bottomLeft.x = lx;
    bottomLeft.y = ry;
    Point2f bottomRight;
    bottomRight.x = rx;
    bottomRight.y = ry;
    vector<Point2f> standardPoints;
    standardPoints.push_back(topLeft);
    standardPoints.push_back(topRight);
    standardPoints.push_back(bottomLeft);
    standardPoints.push_back(bottomRight);

    // Ref: http://docs.opencv.org/2.4/modules/imgproc/doc/geometric_transformations.html
    // Assertion failed: https://groups.google.com/forum/?fromgroups=#!topic/android-opencv/CPDMJsmYVBI
    Mat perspectiveMatrix;
    perspectiveMatrix = getPerspectiveTransform(pts, standardPoints);

    Mat perspectiveImg(800, 800, rotatedImg.type());
    warpPerspective(rotatedImg, perspectiveImg, perspectiveMatrix, perspectiveImg.size());

    imshow("Display Image", perspectiveImg);
    waitKey(0);
    return 0;
}
